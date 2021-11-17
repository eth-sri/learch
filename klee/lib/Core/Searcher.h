//===-- Searcher.h ----------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_SEARCHER_H
#define KLEE_SEARCHER_H

#include "klee/ExecutionState.h"
#include "klee/Internal/System/Time.h"
#include "klee/Internal/Module/KInstruction.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <queue>
#include <set>
#include <vector>
#include <Python.h>
#include <object.h>
#include <klee/Internal/Support/Timer.h>

namespace llvm {
  class BasicBlock;
  class Function;
  class Instruction;
  class raw_ostream;
}

namespace klee {
  template<class T, class Comparator> class DiscretePDF;
  class ExecutionState;
  class Executor;

  class Searcher {
  public:
    virtual ~Searcher();

    virtual ExecutionState &selectState() = 0;

    virtual void update(ExecutionState *current,
                        const std::vector<ExecutionState *> &addedStates,
                        const std::vector<ExecutionState *> &removedStates) = 0;

    virtual bool empty() = 0;

    // prints name of searcher as a klee_message()
    // TODO: could probably make prettier or more flexible
    virtual void printName(llvm::raw_ostream &os) {
      os << "<unnamed searcher>\n";
    }

    // pgbovine - to be called when a searcher gets activated and
    // deactivated, say, by a higher-level searcher; most searchers
    // don't need this functionality, so don't have to override.
    virtual void activate() {}
    virtual void deactivate() {}

    virtual void addFeatures(ExecutionState &) {}

    // utility functions

    void addState(ExecutionState *es, ExecutionState *current = 0) {
      std::vector<ExecutionState *> tmp;
      tmp.push_back(es);
      update(current, tmp, std::vector<ExecutionState *>());
    }

    void removeState(ExecutionState *es, ExecutionState *current = 0) {
      std::vector<ExecutionState *> tmp;
      tmp.push_back(es);
      update(current, std::vector<ExecutionState *>(), tmp);
    }

    enum CoreSearchType {
      DFS,
      BFS,
      RandomState,
      RandomPath,
      NURS_CovNew,
      NURS_MD2U,
      NURS_Depth,
      NURS_ICnt,
      NURS_CPICnt,
      NURS_QC,
      SGS_1,
      SGS_2,
      SGS_4,
      SGS_8,
      ML,
    };
  };

  class DFSSearcher : public Searcher {
    std::vector<ExecutionState*> states;

  public:
    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return states.empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "DFSSearcher\n";
    }
  };

  class SubpathGuidedSearcher : public Searcher {
    std::vector<ExecutionState*> states;
  private:
    Executor &executor;
    uint index;

  public:
      SubpathGuidedSearcher(Executor &_executor, uint index);

  public:
    ExecutionState &selectState();
    void update(ExecutionState *current,
                        const std::vector<ExecutionState *> &addedStates,
                        const std::vector<ExecutionState *> &removedStates);
    bool empty() { return states.empty();}
    void printName(llvm:: raw_ostream &os) {
      os << "New Searcher\n";
    }


  };

  class BFSSearcher : public Searcher {
    std::deque<ExecutionState*> states;

  public:
    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return states.empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "BFSSearcher\n";
    }
  };

  class RandomSearcher : public Searcher {
    std::vector<ExecutionState*> states;

  public:
    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return states.empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "RandomSearcher\n";
    }
  };

  class WeightedRandomSearcher : public Searcher {
  public:
    enum WeightType {
      Depth,
      QueryCost,
      InstCount,
      CPInstCount,
      MinDistToUncovered,
      CoveringNew
    };

  private:
    DiscretePDF<ExecutionState*, ExecutionStateIDCompare> *states;
    WeightType type;
    bool updateWeights;
    
    double getWeight(ExecutionState*);

  public:
    WeightedRandomSearcher(WeightType type);
    ~WeightedRandomSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty();
    void printName(llvm::raw_ostream &os) {
      os << "WeightedRandomSearcher::";
      switch(type) {
      case Depth              : os << "Depth\n"; return;
      case QueryCost          : os << "QueryCost\n"; return;
      case InstCount          : os << "InstCount\n"; return;
      case CPInstCount        : os << "CPInstCount\n"; return;
      case MinDistToUncovered : os << "MinDistToUncovered\n"; return;
      case CoveringNew        : os << "CoveringNew\n"; return;
      default                 : os << "<unknown type>\n"; return;
      }
    }
  };

  class RandomPathSearcher : public Searcher {
    Executor &executor;

  public:
    RandomPathSearcher(Executor &_executor);
    ~RandomPathSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty();
    void printName(llvm::raw_ostream &os) {
      os << "RandomPathSearcher\n";
    }
  };


  extern llvm::cl::opt<bool> UseIncompleteMerge;
  class MergeHandler;
  class MergingSearcher : public Searcher {
    friend class MergeHandler;

    private:

    Searcher *baseSearcher;

    /// States that have been paused by the 'pauseState' function
    std::vector<ExecutionState*> pausedStates;

    public:
    MergingSearcher(Searcher *baseSearcher);
    ~MergingSearcher();

    /// ExecutionStates currently paused from scheduling because they are
    /// waiting to be merged in a klee_close_merge instruction
    std::set<ExecutionState *> inCloseMerge;

    /// Keeps track of all currently ongoing merges.
    /// An ongoing merge is a set of states (stored in a MergeHandler object)
    /// which branched from a single state which ran into a klee_open_merge(),
    /// and not all states in the set have reached the corresponding
    /// klee_close_merge() yet.
    std::vector<MergeHandler *> mergeGroups;

    /// Remove state from the searcher chain, while keeping it in the executor.
    /// This is used here to 'freeze' a state while it is waiting for other
    /// states in its merge group to reach the same instruction.
    void pauseState(ExecutionState &state){
      assert(std::find(pausedStates.begin(), pausedStates.end(), &state) == pausedStates.end());
      pausedStates.push_back(&state);
      baseSearcher->removeState(&state);
    }

    /// Continue a paused state
    void continueState(ExecutionState &state){
      auto it = std::find(pausedStates.begin(), pausedStates.end(), &state);
      assert( it != pausedStates.end());
      pausedStates.erase(it);
      baseSearcher->addState(&state);
    }

    ExecutionState &selectState();

    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates) {
      // We have to check if the current execution state was just deleted, as to
      // not confuse the nurs searchers
      if (std::find(pausedStates.begin(), pausedStates.end(), current) ==
          pausedStates.end()) {
        baseSearcher->update(current, addedStates, removedStates);
      }
    }

    bool empty() { return baseSearcher->empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "MergingSearcher\n";
    }
  };

  class BatchingSearcher : public Searcher {
    Searcher *baseSearcher;
    time::Span timeBudget;
    unsigned instructionBudget;

    ExecutionState *lastState;
    time::Point lastStartTime;
    unsigned lastStartInstructions;

  public:
    BatchingSearcher(Searcher *baseSearcher, 
                     time::Span _timeBudget,
                     unsigned _instructionBudget);
    ~BatchingSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return baseSearcher->empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "<BatchingSearcher> timeBudget: " << timeBudget
         << ", instructionBudget: " << instructionBudget
         << ", baseSearcher:\n";
      baseSearcher->printName(os);
      os << "</BatchingSearcher>\n";
    }
  };

  class BranchingSearcher : public Searcher {
    public:
      BranchingSearcher(Searcher *baseSearcher, Executor &_executor);
      ~BranchingSearcher();

      ExecutionState &selectState();
      void update(ExecutionState *current,
                  const std::vector<ExecutionState *> &addedStates,
                  const std::vector<ExecutionState *> &removedStates);
      bool empty() { return baseSearcher->empty(); }
      void printName(llvm::raw_ostream &os) {
          os << "<BranchingSearcher>"
             << ", baseSearcher:\n";
          baseSearcher->printName(os);
          os << "</BranchingSearcher>\n";
      }
      void addFeatures(ExecutionState &state) override;

      Searcher *baseSearcher;

    private:
      ExecutionState *lastState;
      unsigned lastSelectStackSize;
      Executor &executor;
  };

    class GetFeaturesSearcher : public Searcher {
    public:
        GetFeaturesSearcher(Searcher *searcher, Executor &_executor);
        ~GetFeaturesSearcher();
        void addFeatures(ExecutionState &) override;

        ExecutionState &selectState();
        void update(ExecutionState *current,
                    const std::vector<ExecutionState *> &addedStates,
                    const std::vector<ExecutionState *> &removedStates);
        bool empty() { return baseSearcher->empty(); }
        void printName(llvm::raw_ostream &os) {
            os << "<GetFeaturesSearcher>"
               << ", baseSearcher:\n";
            baseSearcher->printName(os);
            os << "</GetFeaturesSearcher>\n";
        }

    private:
        Searcher *baseSearcher;
        Executor &executor;
        long featureIndex = 0;
    };

    class MLSearcher : public Searcher {
    public:
      enum ModelType {
        Linear,
        Feedforward,
        RNN
      };

    private:
      Executor& executor;
      std::vector<ExecutionState*> states;
      ModelType type;
      bool sampling;

    public:
      MLSearcher(Executor &_executor, std::string model_type, std::string model_path, bool _sampling);
      virtual ~MLSearcher();
      virtual ExecutionState &selectState();
      void update(ExecutionState *current,
                  const std::vector<ExecutionState *> &addedStates,
                  const std::vector<ExecutionState *> &removedStates);
      virtual bool empty() { return states.empty(); }
      virtual void printName(llvm::raw_ostream &os) {
        os << "<MLSearcher>"
           << "</MLSearcher>\n";
      }
    };

  class IterativeDeepeningTimeSearcher : public Searcher {
    Searcher *baseSearcher;
    time::Point startTime;
    time::Span time;
    std::set<ExecutionState*> pausedStates;

  public:
    IterativeDeepeningTimeSearcher(Searcher *baseSearcher);
    ~IterativeDeepeningTimeSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return baseSearcher->empty() && pausedStates.empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "IterativeDeepeningTimeSearcher\n";
    }
  };

  class InterleavedSearcher : public Searcher {
    typedef std::vector<Searcher*> searchers_ty;

    searchers_ty searchers;
    unsigned index;

  public:
    explicit InterleavedSearcher(const searchers_ty &_searchers);
    ~InterleavedSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return searchers[0]->empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "<InterleavedSearcher> containing "
         << searchers.size() << " searchers:\n";
      for (searchers_ty::iterator it = searchers.begin(), ie = searchers.end();
           it != ie; ++it)
        (*it)->printName(os);
      os << "</InterleavedSearcher>\n";
    }
  };

}

#endif /* KLEE_SEARCHER_H */
