#include "klee/Feature/InterProcCFG.h"
#include "klee/Feature/GraphUtil.h"
#include "llvm/Support/DOTGraphTraits.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include <vector>
#include <unordered_map>
#include <iostream>

std::vector<std::string> termFunc{ "exit", "abort", "_exit", "_Exit" };

std::vector<std::string> overflowLabels {
    "__ubsan_handle_add_overflow",
    "__ubsan_handle_add_overflow_abort",
    "__ubsan_handle_sub_overflow",
    "__ubsan_handle_sub_overflow_abort",
    "__ubsan_handle_mul_overflow",
    "__ubsan_handle_mul_overflow_abort",
    "__ubsan_handle_divrem_overflow",
    "__ubsan_handle_divrem_overflow_abort",
    "__ubsan_handle_negate_overflow",
    "__ubsan_handle_negate_overflow_abort"
};

std::vector<std::string> shiftLabels {
    "__ubsan_handle_shift_out_of_bounds",
    "__ubsan_handle_shift_out_of_bounds_abort"
};

std::vector<std::string> oobLabels {
    "__ubsan_handle_out_of_bounds",
    "__ubsan_handle_out_of_bounds_abort"
};

std::vector<std::string> pointerLabels {
    "__ubsan_handle_pointer_overflow",
    "__ubsan_handle_pointer_overflow_abort"
};

std::vector<std::string> nullLabels {
    "__ubsan_handle_type_mismatch",
    "__ubsan_handle_type_mismatch_abort",
    "__ubsan_handle_type_mismatch_v1",
    "__ubsan_handle_type_mismatch_v1_abort"
};

bool InterProcCFG::termInst(llvm::Instruction &dest) {
    if (!dest.getParent()) return false;

    for (llvm::BasicBlock::iterator inst = dest.getParent()->begin(); inst != dest.getParent()->end(); inst++) {
        if ((&(*inst)) == (&dest)) break;
        if (llvm::isa<llvm::CallInst>(inst) || llvm::isa<llvm::InvokeInst>(inst)) {
            llvm::Function *callee;
            if (llvm::isa<llvm::CallInst>(inst)) {
                callee = llvm::cast<llvm::CallInst>(inst)->getCalledFunction();
            } else {
                callee = llvm::cast<llvm::InvokeInst>(inst)->getCalledFunction();
            }
            if (callee && callee->hasName()) {
                if (std::find(termFunc.begin(), termFunc.end(), callee->getName().str()) != termFunc.end()) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool InterProcCFG::hasEdge(InterProcCFGNode* src, InterProcCFGNode* dst, InterProcCFGEdge::CEDGEK kind) const {
    InterProcCFGEdge edge(src, dst, kind);
    InterProcCFGEdge* outEdge = src->hasOutgoingEdge(&edge);
    InterProcCFGEdge* inEdge = src->hasIncomingEdge(&edge);
    if (outEdge && inEdge) {
        assert(outEdge == inEdge && "edges do not match!");
        return true;
    } else {
        return false;
    }
}

void InterProcCFG::addCFGEdge(InterProcCFGNode* src, InterProcCFGNode* dst, InterProcCFGEdge::CEDGEK kind) {
    if (!hasEdge(src, dst, kind)) {
        InterProcCFGEdge* edge = new InterProcCFGEdge(src, dst, kind);
        edge->getSrcNode()->addOutgoingEdge(edge);
        edge->getDstNode()->addIncomingEdge(edge);
    }
}

void InterProcCFG::addCFGEdge(const llvm::BasicBlock* src, const llvm::BasicBlock* dst, InterProcCFGEdge::CEDGEK kind) {
    InterProcCFGNode* srcNode = getNode(src);
    InterProcCFGNode* dstNode = getNode(dst);

    if (!hasEdge(srcNode, dstNode, kind)) {
        InterProcCFGEdge* edge = new InterProcCFGEdge(srcNode, dstNode, kind);
        edge->getSrcNode()->addOutgoingEdge(edge);
        edge->getDstNode()->addIncomingEdge(edge);
    }
}

InterProcCFG::InterProcCFG(llvm::Module* _mod) : mod(_mod), totalNodeNum(0) {
    // add nodes
    entry = new InterProcCFGNode(totalNodeNum, NULL, this);
    addGNode(totalNodeNum, entry);
    ++totalNodeNum;

    for (llvm::Module::iterator f = mod->begin(), e = mod->end(); f != e; ++f) {
        for (llvm::Function::iterator b = f->begin(), eb = f->end(); b != eb; ++b) {
            NodeID id = totalNodeNum;
            InterProcCFGNode* node = new InterProcCFGNode(id, &*b, this);
            bbToCFGNodeMap[&*b] = node;
            nodeToIDMap[node] = id;
            addGNode(id, node);
            ++totalNodeNum;
        }
        if (f->isDeclaration() || f->isIntrinsic()) continue;
        addCFGEdge(entry, getNode(&(f->getEntryBlock())), InterProcCFGEdge::PseudoEdge);
    }

    // add edges
    for (llvm::Module::iterator f = mod->begin(), e = mod->end(); f != e; ++f) {
        for (llvm::Function::iterator b = f->begin(), eb = f->end(); b != eb; ++b) {
            for (llvm::BasicBlock::iterator iit = b->begin(); iit != b->end(); iit++) {
                if (llvm::isa<llvm::CallInst>(iit) || llvm::isa<llvm::InvokeInst>(iit)) {
                    llvm::Function* callee;
                    if (llvm::isa<llvm::CallInst>(iit)) {
                        callee = llvm::cast<llvm::CallInst>(iit)->getCalledFunction();
                    } else {
                        callee = llvm::cast<llvm::InvokeInst>(iit)->getCalledFunction();
                    }
                    if (f->getName() == "__uClibc_main" && !callee) {
                        addCFGEdge(&*b, &(mod->getFunction("__user_main")->getEntryBlock()), InterProcCFGEdge::CallRetEdge);
                    }
                    if (!callee || callee->isDeclaration() || callee->isIntrinsic()) continue;
                    addCFGEdge(&*b, &(callee->getEntryBlock()), InterProcCFGEdge::CallRetEdge);
                    // std::cout << b->getParent()->getName().str() << " " << callee->getName().str() << std::endl;
                }
            }
            if (termInst(*(b->getTerminator()))) continue;
            for (llvm::succ_iterator sit = succ_begin(&*b), send = succ_end(&*b); sit != send; ++sit) {
                addCFGEdge(&*b, *sit, InterProcCFGEdge::CFEdge);
            }
        }
    }
}

InterProcCFG::~InterProcCFG() {
}

void InterProcCFG::computeScore() {
    computeScoreSquare();
}

void InterProcCFG::computeScoreLinear() {
    std::vector<BlockStat*> scc_stats;
    std::unordered_map<InterProcCFGNode*, unsigned> node_to_scc;
    unsigned scc_index = 0;

    for (llvm::scc_iterator<InterProcCFG*> scci = llvm::scc_begin(this), scce = llvm::scc_end(this); scci != scce; ++scci) {
        BlockStat* scc_stat = new BlockStat();
        BlockStat* desc_stat = new BlockStat();
        unsigned num_descs = 0;

        for (auto cfgnode : *scci) {
            node_to_scc[cfgnode] = scc_index;
            if (cfgnode->getBasicBlock()) {
                scc_stat->analyze(cfgnode->getBasicBlock());
            } else {
                break;
            }
            for (auto edge : cfgnode->getOutEdges()) {
                InterProcCFGNode* dstNode = edge->getDstNode();
                if (std::find(scci->begin(), scci->end(), dstNode) == scci->end()) {
                    unsigned dst_scc_index = node_to_scc[dstNode];
                    desc_stat->update(scc_stats[dst_scc_index]);
                    ++num_descs;
                }
            }
        }

        if (num_descs > 0) desc_stat->divide(num_descs);
        scc_stat->update(desc_stat);
        delete desc_stat;

        scc_stats.push_back(scc_stat);
        ++scc_index;
    }

    scc_index = 0;
    for (llvm::scc_iterator<InterProcCFG*> scci = llvm::scc_begin(this), scce = llvm::scc_end(this); scci != scce; ++scci) {
        BlockStat* scc_stat = scc_stats[scc_index];

        for (auto cfgnode : *scci) {
            cfgnode->setStat(scc_stat->copy());
        }

        delete scc_stat;
        ++scc_index;
    }
}

void InterProcCFG::computeScoreSquare() {
    std::vector<std::set<unsigned> > scc_descs;
    std::vector<BlockStat*> scc_stats;
    std::unordered_map<InterProcCFGNode*, unsigned> node_to_scc;
    unsigned scc_index = 0;

    for (llvm::scc_iterator<InterProcCFG*> scci = llvm::scc_begin(this), scce = llvm::scc_end(this); scci != scce; ++scci) {
        std::set<unsigned> scc_desc;
        BlockStat* scc_stat = new BlockStat();

        for (auto cfgnode : *scci) {
            node_to_scc[cfgnode] = scc_index;
            if(cfgnode->getBasicBlock()) {
                scc_stat->analyze(cfgnode->getBasicBlock());
            } else {
                break;
            }
            for (auto edge : cfgnode->getOutEdges()) {
                InterProcCFGNode* dstNode = edge->getDstNode();
                if (std::find(scci->begin(), scci->end(), dstNode) == scci->end()) {
                    unsigned dst_scc_index = node_to_scc[dstNode];
                    scc_desc.insert(dst_scc_index);
                    scc_desc.insert(scc_descs[dst_scc_index].begin(), scc_descs[dst_scc_index].end());
                }
            }
        }
        
        scc_descs.push_back(scc_desc);
        scc_stats.push_back(scc_stat);
        scc_index++;
    }

    scc_index = 0;
    unsigned num_of_nodes = 0;
    for (llvm::scc_iterator<InterProcCFG*> scci = llvm::scc_begin(this), scce = llvm::scc_end(this); scci != scce; ++scci) {
        std::set<unsigned> scc_desc = scc_descs[scc_index];
        BlockStat* scc_stat = scc_stats[scc_index]->copy();

        for (std::set<unsigned>::iterator it=scc_desc.begin(); it!=scc_desc.end(); it++) {
            scc_stat->update(scc_stats[*it]);
        }

        for (auto cfgnode : *scci) {
            num_of_nodes++;
            cfgnode->setStat(scc_stat->copy());
        }

        delete scc_stat;
        scc_index++;
    }

    for (unsigned i=0; i<scc_stats.size(); i++) {
        delete scc_stats[i];
    }
}

void InterProcCFG::dump(const std::string& filename) {
    llvm::GraphPrinter::WriteGraphToFile(llvm::outs(), filename, this);
}

namespace llvm {
    template<>
    struct DOTGraphTraits<InterProcCFG*> : public DefaultDOTGraphTraits {
        typedef InterProcCFGNode NodeType;
		typedef NodeType::iterator ChildIteratorType;

		DOTGraphTraits(bool isSimple = false) : DefaultDOTGraphTraits(isSimple) {}
		static std::string getGraphName(InterProcCFG* graph) {
			return "Inter-procedural Control Flow Graph";
		}

		static std::string getNodeLabel(InterProcCFGNode* node, InterProcCFG* graph) {
            if (!node->getBasicBlock()) {
                return "Pseudo Node";
            } else {
                unsigned num_block = node->getStat() ? node->getStat()->num_source : 0;
                return node->getBasicBlock()->getParent()->getName().str() + "_" + std::to_string((unsigned long)node->getBasicBlock()) + " " + std::to_string(num_block);
            }
		}

		static std::string getNodeAttributes(InterProcCFGNode* node, InterProcCFG* graph) {
			return "shape=circle";
		}

		template<class EdgeIter>
		static std::string getEdgeAttributes(InterProcCFGNode* node, EdgeIter EI, InterProcCFG* graph) {
			InterProcCFGEdge* edge = *(EI.getCurrent());
			std::string color;
			if (edge->getEdgeKind() == InterProcCFGEdge::CFEdge) {
					color = "color=blue";
			} else if (edge->getEdgeKind() == InterProcCFGEdge::CallRetEdge) {
					color = "color=red";
			}
			return color;
		}
    };
}