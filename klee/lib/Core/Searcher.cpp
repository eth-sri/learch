//===-- Searcher.cpp ------------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Searcher.h"

#include "CoreStats.h"
#include "Executor.h"
#include "PTree.h"
#include "StatsTracker.h"

#include "klee/MergeHandler.h"
#include "klee/Statistics.h"
#include "klee/Internal/Module/InstructionInfoTable.h"
#include "klee/Internal/Module/KInstruction.h"
#include "klee/Internal/Module/KModule.h"
#include "klee/Internal/ADT/DiscretePDF.h"
#include "klee/Internal/ADT/RNG.h"
#include "klee/Internal/Support/ModuleUtil.h"
#include "klee/Internal/System/Time.h"
#include "klee/Internal/Support/ErrorHandling.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"

#include <cassert>
#include <fstream>
#include <climits>
#include <iostream>
#include <experimental/filesystem>
#include <algorithm>
#include <set>

#include <Python.h>

#include "llvm/IR/CFG.h"

using namespace klee;
using namespace llvm;
namespace filesystem=std::experimental::filesystem;


namespace klee {
  extern RNG theRNG;
}

Searcher::~Searcher() {
}

///

ExecutionState &DFSSearcher::selectState() {
  return *states.back();
}

void DFSSearcher::update(ExecutionState *current,
                         const std::vector<ExecutionState *> &addedStates,
                         const std::vector<ExecutionState *> &removedStates) {
  states.insert(states.end(),
                addedStates.begin(),
                addedStates.end());
  for (std::vector<ExecutionState *>::const_iterator it = removedStates.begin(),
                                                     ie = removedStates.end();
       it != ie; ++it) {
    ExecutionState *es = *it;
    if (es == states.back()) {
      states.pop_back();
    } else {
      bool ok = false;

      for (std::vector<ExecutionState*>::iterator it = states.begin(),
             ie = states.end(); it != ie; ++it) {
        if (es==*it) {
          states.erase(it);
          ok = true;
          break;
        }
      }

      (void) ok;
      assert(ok && "invalid state removed");
    }
  }
}

///

ExecutionState &BFSSearcher::selectState() {
  return *states.front();
}

void BFSSearcher::update(ExecutionState *current,
                         const std::vector<ExecutionState *> &addedStates,
                         const std::vector<ExecutionState *> &removedStates) {
  // Assumption: If new states were added KLEE forked, therefore states evolved.
  // constraints were added to the current state, it evolved.
  if (!addedStates.empty() && current &&
      std::find(removedStates.begin(), removedStates.end(), current) ==
          removedStates.end()) {
    auto pos = std::find(states.begin(), states.end(), current);
    assert(pos != states.end());
    states.erase(pos);
    states.push_back(current);
  }

  states.insert(states.end(),
                addedStates.begin(),
                addedStates.end());
  for (std::vector<ExecutionState *>::const_iterator it = removedStates.begin(),
                                                     ie = removedStates.end();
       it != ie; ++it) {
    ExecutionState *es = *it;
    if (es == states.front()) {
      states.pop_front();
    } else {
      bool ok = false;

      for (std::deque<ExecutionState*>::iterator it = states.begin(),
             ie = states.end(); it != ie; ++it) {
        if (es==*it) {
          states.erase(it);
          ok = true;
          break;
        }
      }

      (void) ok;
      assert(ok && "invalid state removed");
    }
  }
}

///

ExecutionState &RandomSearcher::selectState() {
  return *states[theRNG.getInt32()%states.size()];
}

void
RandomSearcher::update(ExecutionState *current,
                       const std::vector<ExecutionState *> &addedStates,
                       const std::vector<ExecutionState *> &removedStates) {
  states.insert(states.end(),
                addedStates.begin(),
                addedStates.end());
  for (std::vector<ExecutionState *>::const_iterator it = removedStates.begin(),
                                                     ie = removedStates.end();
       it != ie; ++it) {
    ExecutionState *es = *it;
    __attribute__((unused))
    bool ok = false;

    for (std::vector<ExecutionState*>::iterator it = states.begin(),
           ie = states.end(); it != ie; ++it) {
      if (es==*it) {
        states.erase(it);
        ok = true;
        break;
      }
    }
    
    assert(ok && "invalid state removed");
  }
}

///

WeightedRandomSearcher::WeightedRandomSearcher(WeightType _type)
  : states(new DiscretePDF<ExecutionState*, ExecutionStateIDCompare>()),
    type(_type) {
  switch(type) {
  case Depth: 
    updateWeights = false;
    break;
  case InstCount:
  case CPInstCount:
  case QueryCost:
  case MinDistToUncovered:
  case CoveringNew:
    updateWeights = true;
    break;
  default:
    assert(0 && "invalid weight type");
  }
}

WeightedRandomSearcher::~WeightedRandomSearcher() {
  delete states;
}

ExecutionState &WeightedRandomSearcher::selectState() {
  return *states->choose(theRNG.getDoubleL());
}

double WeightedRandomSearcher::getWeight(ExecutionState *es) {
  switch(type) {
  default:
  case Depth: 
    return es->weight;
  case InstCount: {
    uint64_t count = theStatisticManager->getIndexedValue(stats::instructions,
                                                          es->pc->info->id);
    double inv = 1. / std::max((uint64_t) 1, count);
    return inv * inv;
  }
  case CPInstCount: {
    StackFrame &sf = es->stack.back();
    uint64_t count = sf.callPathNode->statistics.getValue(stats::instructions);
    double inv = 1. / std::max((uint64_t) 1, count);
    return inv;
  }
  case QueryCost:
    return (es->queryCost.toSeconds() < .1) ? 1. : 1./ es->queryCost.toSeconds();
  case CoveringNew:
  case MinDistToUncovered: {
    uint64_t md2u = computeMinDistToUncovered(es->pc,
                                              es->stack.back().minDistToUncoveredOnReturn);

    double invMD2U = 1. / (md2u ? md2u : 10000);
    if (type==CoveringNew) {
      double invCovNew = 0.;
      if (es->instsSinceCovNew)
        invCovNew = 1. / std::max(1, (int) es->instsSinceCovNew - 1000);
      return (invCovNew * invCovNew + invMD2U * invMD2U);
    } else {
      return invMD2U * invMD2U;
    }
  }
  }
}

void WeightedRandomSearcher::update(
    ExecutionState *current, const std::vector<ExecutionState *> &addedStates,
    const std::vector<ExecutionState *> &removedStates) {
  if (current && updateWeights &&
      std::find(removedStates.begin(), removedStates.end(), current) ==
          removedStates.end())
    states->update(current, getWeight(current));

  for (std::vector<ExecutionState *>::const_iterator it = addedStates.begin(),
                                                     ie = addedStates.end();
       it != ie; ++it) {
    ExecutionState *es = *it;
    states->insert(es, getWeight(es));
  }

  std::set<ExecutionState *> removed;
  for (std::vector<ExecutionState *>::const_iterator it = removedStates.begin(),
                                                     ie = removedStates.end();
       it != ie; ++it) {
    if (removed.find(*it) == removed.end()) {
      states->remove(*it);
      removed.insert(*it);
    }
  }
}

bool WeightedRandomSearcher::empty() { 
  return states->empty(); 
}

///
RandomPathSearcher::RandomPathSearcher(Executor &_executor)
  : executor(_executor) {
}

RandomPathSearcher::~RandomPathSearcher() {
}

ExecutionState &RandomPathSearcher::selectState() {
  // klee_message("state size: %d", executor.states.size());

  unsigned flips=0, bits=0;
  PTreeNode *n = executor.processTree->root.get();
  while (!n->state) {
    if (!n->left) {
      n = n->right.get();
    } else if (!n->right) {
      n = n->left.get();
    } else {
      if (bits==0) {
        flips = theRNG.getInt32();
        bits = 32;
      }
      --bits;
      n = (flips&(1<<bits)) ? n->left.get() : n->right.get();
    }
  }

  return *n->state;
}

void
RandomPathSearcher::update(ExecutionState *current,
                           const std::vector<ExecutionState *> &addedStates,
                           const std::vector<ExecutionState *> &removedStates) {
}

bool RandomPathSearcher::empty() { 
  return executor.states.empty(); 
}

///

MergingSearcher::MergingSearcher(Searcher *_baseSearcher)
  : baseSearcher(_baseSearcher){}

MergingSearcher::~MergingSearcher() {
  delete baseSearcher;
}

ExecutionState& MergingSearcher::selectState() {
  assert(!baseSearcher->empty() && "base searcher is empty");

  if (!UseIncompleteMerge)
    return baseSearcher->selectState();

  // Iterate through all MergeHandlers
  for (auto cur_mergehandler: mergeGroups) {
    // Find one that has states that could be released
    if (!cur_mergehandler->hasMergedStates()) {
      continue;
    }
    // Find a state that can be prioritized
    ExecutionState *es = cur_mergehandler->getPrioritizeState();
    if (es) {
      return *es;
    } else {
      if (DebugLogIncompleteMerge){
        llvm::errs() << "Preemptively releasing states\n";
      }
      // If no state can be prioritized, they all exceeded the amount of time we
      // are willing to wait for them. Release the states that already arrived at close_merge.
      cur_mergehandler->releaseStates();
    }
  }
  // If we were not able to prioritize a merging state, just return some state
  return baseSearcher->selectState();
}

///

BranchingSearcher::BranchingSearcher(klee::Searcher *_baseSearcher, Executor &_executor)
    : baseSearcher(_baseSearcher), lastState(nullptr), lastSelectStackSize(0), executor(_executor) {}

BranchingSearcher::~BranchingSearcher() {
    delete baseSearcher;
}

ExecutionState &BranchingSearcher::selectState() {
    // ExecutionState* returnState;

    // if(!lastState) {
    //   returnState = &baseSearcher->selectState();
    //   lastState = returnState;
    // } else {
    //   returnState = lastState;
    // }

    // KInstruction *ki = returnState->pc;
    // switch (ki->inst->getOpcode()) {
    //   case Instruction::Br:
    //   case Instruction::IndirectBr:
    //   case Instruction::Switch:
    //     lastState = nullptr;
    //     break;
    //   default:
    //     break;
    // }

    // return *returnState;

    if(!lastState) {
      lastState = &baseSearcher->selectState();
      lastSelectStackSize = lastState->stack.size();
    }

    return *lastState;
}

void BranchingSearcher::update(klee::ExecutionState *current,
                                          const std::vector<ExecutionState *> &addedStates,
                                          const std::vector<ExecutionState *> &removedStates) {
    //If current lastState has been removed: Need to select State again.
    if (std::find(removedStates.begin(), removedStates.end(), lastState) !=
      removedStates.end())
      lastState = nullptr;

    //If current is non-null, new states might have been created. Need to select again.
    if(current && !(addedStates.empty())) {
      lastState = nullptr;
    }

    // if (current && current->stack.size() < lastSelectStackSize) {
    //   if (executor.getFeatureExtract()) {
    //     lastState->predicted = false;
    //     executor.getStateFeatures(lastState);
    //   }
    //   lastState = nullptr;
    // }

    baseSearcher->update(current, addedStates, removedStates);
}

void BranchingSearcher::addFeatures(ExecutionState &state) {
  baseSearcher->addFeatures(state);
}

GetFeaturesSearcher::GetFeaturesSearcher(Searcher *searcher, Executor &_executor) : baseSearcher(searcher), executor(_executor) {
}

GetFeaturesSearcher::~GetFeaturesSearcher() {
  delete baseSearcher;
}

ExecutionState &GetFeaturesSearcher::selectState() {
  for (auto it=executor.featureStates.begin(); it != executor.featureStates.end(); ++it) {
    (*it)->predicted = false;
    executor.getStateFeatures(*it);
  }
  executor.featureStates.clear();
  ExecutionState &selected = baseSearcher->selectState();
  addFeatures(selected);

  subpath_ty subpath;
  executor.getSubpath(&selected, subpath, 0);
  executor.incSubpath(subpath, 0);
  executor.getSubpath(&selected, subpath, 1);
  executor.incSubpath(subpath, 1);
  executor.getSubpath(&selected, subpath, 2);
  executor.incSubpath(subpath, 2);
  executor.getSubpath(&selected, subpath, 3);
  executor.incSubpath(subpath, 3);

  BasicBlock* block = selected.pc->inst->getParent();
  if (ExecutionState::blockVisitTimes.find(block) == ExecutionState::blockVisitTimes.end()) {
    ExecutionState::blockVisitTimes[block] = 0;
  }
  ExecutionState::blockVisitTimes[block]++;

  return selected;
}

void GetFeaturesSearcher::addFeatures(ExecutionState &es) {
  long index = featureIndex++;
  // std::cout << featureIndex << " " << es.feature[11] << " " << es.feature[12] << std::endl;
  es.features.emplace_back(index, es.feature);
  // std::cout << "select " << index << " " << es.pc->inst->getParent()->getParent()->getName().str() << std::endl;
}

void GetFeaturesSearcher::update(klee::ExecutionState *current,
        const std::vector<ExecutionState *> &addedStates,
        const std::vector<ExecutionState *> &removedStates) {
    baseSearcher->update(current, addedStates, removedStates);
}

MLSearcher::MLSearcher(Executor &_executor, std::string model_type, std::string model_path, bool _sampling) : executor(_executor), sampling(_sampling) {
  Py_Initialize();
  PyGILState_STATE gstate = PyGILState_Ensure();
  PyObject *pName = PyUnicode_FromString("model");
  PyObject *pModule = PyImport_Import(pName);
  if (model_type == "linear") {
    type = Linear;
  } else if (model_type == "feedforward") {
    type = Feedforward;
  } else if (model_type == "rnn") {
    type = RNN;
  }
  PyObject* pArgs = PyTuple_New(2);
  PyTuple_SetItem(pArgs, 0, PyBytes_FromString(model_type.c_str()));
  PyTuple_SetItem(pArgs, 1, PyBytes_FromString(model_path.c_str()));
  PyObject* pInitFunc = PyObject_GetAttrString(pModule, "init_model");
  PyObject_CallObject(pInitFunc, pArgs);

  Py_DECREF(pModule);
  Py_DECREF(pName);
  Py_DECREF(pArgs);
  Py_DECREF(pInitFunc);
  PyGILState_Release(gstate);
}

MLSearcher::~MLSearcher() {
  Py_Finalize();
}

ExecutionState &MLSearcher::selectState() {
  PyGILState_STATE gstate = PyGILState_Ensure();
  int batch_size = 0;
  PyObject *features = PyList_New(0), *hiddens = PyList_New(0);
  for (auto state : states) {
    if (!state->predicted) {
      ++batch_size;
      PyObject* feature = PyList_New(0);
      for (uint i=2; i<state->feature.size(); i++) {
        PyList_Append(feature, PyFloat_FromDouble(state->feature[i]));
      }
      PyList_Append(features, feature);
      if (type == RNN) {
        PyObject* hidden = PyList_New(0);
        for (uint i=0; i<state->hidden_state.size(); i++) {
          PyList_Append(hidden, PyFloat_FromDouble(state->hidden_state[i]));
        }
        PyList_Append(hiddens, hidden);
      }
    }
  }

  if (batch_size > 0) {
    PyObject* pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, features);
    PyTuple_SetItem(pArgs, 1, hiddens);
    PyObject *pName = PyUnicode_FromString("model");
    PyObject *pModule = PyImport_Import(pName);
    PyObject *pCallFunc = PyObject_GetAttrString(pModule, "predict");
    PyObject* res = PyObject_CallObject(pCallFunc, pArgs);
    PyObject* rewards = PyTuple_GetItem(res, 0);
    PyObject* new_hiddens = PyTuple_GetItem(res, 1);

    int i=0;
    for (auto state : states) {
      if(!state->predicted) {
        state->predicted = true;
        state->predicted_reward = PyFloat_AsDouble(PyList_GetItem(rewards, i));
        if (type == RNN) {
          for (uint j=0; j<state->hidden_state.size(); j++) {
            state->hidden_state[j] = PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(new_hiddens, i), j));
          }
        }
        i++;
      }
    }
  }

  ExecutionState *selection = NULL;
  if (sampling) {
    PyObject* pArgs = PyTuple_New(1);
    PyObject* predicted = PyList_New(0);
    for (auto state : states) {
      PyList_Append(predicted, PyFloat_FromDouble(state->predicted_reward));
    }
    PyTuple_SetItem(pArgs, 0, predicted);
    PyObject *pName = PyUnicode_FromString("model");
    PyObject *pModule = PyImport_Import(pName);
    PyObject *pCallFunc = PyObject_GetAttrString(pModule, "sample");
    PyObject *res = PyObject_CallObject(pCallFunc, pArgs);
    selection = states[PyLong_AsLong(res)];
  } else {
    double current_max = -100000000.0;
    bool current_set = false;
    for (auto state : states) {
      // std::cout << state->predicted_reward << " ";
      if(!current_set || current_max < state->predicted_reward) {
        selection = state;
        current_max = state->predicted_reward;
        current_set = true;
      }
    }
    // std::cout << std::endl << current_max << std::endl << std::endl;
  }

  selection->predicted_reward = 0.0;
  selection->predicted = false;

  PyGILState_Release(gstate);

  return *selection;
}

void MLSearcher::update(klee::ExecutionState *current,
                         const std::vector<ExecutionState *> &addedStates,
                         const std::vector<ExecutionState *> &removedStates) {
  states.insert(states.end(),
                addedStates.begin(),
                addedStates.end());
  std::set<ExecutionState *> removed;
  for (std::vector<ExecutionState *>::const_iterator it = removedStates.begin(),
                                                     ie = removedStates.end();
       it != ie; ++it) {
    ExecutionState *es = *it;
    if (removed.find(es) != removed.end()) continue;
    __attribute__((unused))
    bool ok = false;

    for (std::vector<ExecutionState*>::iterator it = states.begin(),
           ie = states.end(); it != ie; ++it) {
      if (es==*it) {
        states.erase(it);
        ok = true;
        break;
      }
    }
    
    assert(ok && "invalid state removed");
    removed.insert(es);
  }                        
}

BatchingSearcher::BatchingSearcher(Searcher *_baseSearcher,
                                   time::Span _timeBudget,
                                   unsigned _instructionBudget) 
  : baseSearcher(_baseSearcher),
    timeBudget(_timeBudget),
    instructionBudget(_instructionBudget),
    lastState(0) {
  
}

BatchingSearcher::~BatchingSearcher() {
  delete baseSearcher;
}

ExecutionState &BatchingSearcher::selectState() {
  if (!lastState ||
      (((timeBudget.toSeconds() > 0) &&
        (time::getWallTime() - lastStartTime) > timeBudget)) ||
      ((instructionBudget > 0) &&
       (stats::instructions - lastStartInstructions) > instructionBudget)) {
    if (lastState) {
      time::Span delta = time::getWallTime() - lastStartTime;
      auto t = timeBudget;
      t *= 1.1;
      if (delta > t) {
        klee_message("increased time budget from %f to %f\n", timeBudget.toSeconds(), delta.toSeconds());
        timeBudget = delta;
      }
    }
    lastState = &baseSearcher->selectState();
    lastStartTime = time::getWallTime();
    lastStartInstructions = stats::instructions;
    return *lastState;
  } else {
    return *lastState;
  }
}

void
BatchingSearcher::update(ExecutionState *current,
                         const std::vector<ExecutionState *> &addedStates,
                         const std::vector<ExecutionState *> &removedStates) {
  if (std::find(removedStates.begin(), removedStates.end(), lastState) !=
      removedStates.end())
    lastState = 0;
  baseSearcher->update(current, addedStates, removedStates);
}

/***/

IterativeDeepeningTimeSearcher::IterativeDeepeningTimeSearcher(Searcher *_baseSearcher)
  : baseSearcher(_baseSearcher),
    time(time::seconds(1)) {
}

IterativeDeepeningTimeSearcher::~IterativeDeepeningTimeSearcher() {
  delete baseSearcher;
}

ExecutionState &IterativeDeepeningTimeSearcher::selectState() {
  ExecutionState &res = baseSearcher->selectState();
  startTime = time::getWallTime();
  return res;
}

void IterativeDeepeningTimeSearcher::update(
    ExecutionState *current, const std::vector<ExecutionState *> &addedStates,
    const std::vector<ExecutionState *> &removedStates) {

  const auto elapsed = time::getWallTime() - startTime;

  if (!removedStates.empty()) {
    std::vector<ExecutionState *> alt = removedStates;
    for (std::vector<ExecutionState *>::const_iterator
             it = removedStates.begin(),
             ie = removedStates.end();
         it != ie; ++it) {
      ExecutionState *es = *it;
      std::set<ExecutionState*>::const_iterator it2 = pausedStates.find(es);
      if (it2 != pausedStates.end()) {
        pausedStates.erase(it2);
        alt.erase(std::remove(alt.begin(), alt.end(), es), alt.end());
      }
    }    
    baseSearcher->update(current, addedStates, alt);
  } else {
    baseSearcher->update(current, addedStates, removedStates);
  }

  if (current &&
      std::find(removedStates.begin(), removedStates.end(), current) ==
          removedStates.end() &&
      elapsed > time) {
    pausedStates.insert(current);
    baseSearcher->removeState(current);
  }

  if (baseSearcher->empty()) {
    time *= 2U;
    klee_message("increased time budget to %f\n", time.toSeconds());
    std::vector<ExecutionState *> ps(pausedStates.begin(), pausedStates.end());
    baseSearcher->update(0, ps, std::vector<ExecutionState *>());
    pausedStates.clear();
  }
}

/***/

InterleavedSearcher::InterleavedSearcher(const std::vector<Searcher*> &_searchers)
  : searchers(_searchers),
    index(1) {
}

InterleavedSearcher::~InterleavedSearcher() {
  for (std::vector<Searcher*>::const_iterator it = searchers.begin(),
         ie = searchers.end(); it != ie; ++it)
    delete *it;
}

ExecutionState &InterleavedSearcher::selectState() {
  Searcher *s = searchers[--index];
  if (index==0) index = searchers.size();
  return s->selectState();
}

void InterleavedSearcher::update(
    ExecutionState *current, const std::vector<ExecutionState *> &addedStates,
    const std::vector<ExecutionState *> &removedStates) {
  for (std::vector<Searcher*>::const_iterator it = searchers.begin(),
         ie = searchers.end(); it != ie; ++it)
    (*it)->update(current, addedStates, removedStates);
}

SubpathGuidedSearcher::SubpathGuidedSearcher(Executor &_executor, uint index)
        : executor(_executor), index(index) {
}

ExecutionState &SubpathGuidedSearcher::selectState() {
    unsigned long minCount = ULONG_MAX;
    std::vector<ExecutionState *> selectSet;
    // std::cout << "states:" << std::endl;
    for(auto & state : states) {
      subpath_ty subpath;
      executor.getSubpath(state, subpath, index);
      unsigned long curr = executor.getSubpathCount(subpath, index);
      // std::cout << state;
      // std::cout << " (" << state->prevPC->info->id << ", " << state->prevPC->inst->getOpcode() << ") ";
      // std::cout << " (" << state->pc->info->id << ", " << state->pc->inst->getOpcode() << ") ";
      // printSubpath(currentSubpath);
      // std::cout << curr << std::endl;
      // printSubpath(state->takenBranches);
      // std::cout << std::endl;
      if(curr < minCount) {
        selectSet.clear();
        minCount = curr;
      }

      if(curr == minCount) {
        selectSet.push_back(state);
      }
    }

    unsigned int random = theRNG.getInt32() % selectSet.size();
    ExecutionState *selection = selectSet[random];

    if (!executor.getFeatureExtract()) {
      subpath_ty subpath;
      executor.getSubpath(selection, subpath, index);
      // std::cout << "selected: ";
      // printSubpath(currentSubpath);
      // std::cout << std::endl;
      // std::cout << std::endl;
      executor.incSubpath(subpath, index);
    }

    return *selection;
}

void SubpathGuidedSearcher::update(ExecutionState *current,
                                   const std::vector<ExecutionState *> &addedStates,
                                   const std::vector<ExecutionState *> &removedStates) {
  states.insert(states.end(),
                addedStates.begin(),
                addedStates.end());
  std::set<ExecutionState *> removed;
  for (std::vector<ExecutionState *>::const_iterator it = removedStates.begin(),
                                                     ie = removedStates.end();
       it != ie; ++it) {
    ExecutionState *es = *it;
    if (removed.find(es) != removed.end()) continue;
    __attribute__((unused))
    bool ok = false;

    for (std::vector<ExecutionState*>::iterator it = states.begin(),
           ie = states.end(); it != ie; ++it) {
      if (es==*it) {
        states.erase(it);
        ok = true;
        break;
      }
    }
    
    assert(ok && "invalid state removed");
    removed.insert(es);
  }                        
}
