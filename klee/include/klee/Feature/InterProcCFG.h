#ifndef INTER_PROC_CFG_H_
#define INTER_PROC_CFG_H_

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/GraphTraits.h"

#include "klee/Feature/GenericGraph.h"
#include "klee/Feature/BlockStat.h"

class InterProcCFGEdge;
class InterProcCFGNode;
class InterProcCFG;

class InterProcCFGEdge: public GenericEdge<InterProcCFGNode> {
public:
    enum CEDGEK {
        CFEdge, CallRetEdge, PseudoEdge
    };

    InterProcCFGEdge(InterProcCFGNode* s, InterProcCFGNode* d, CEDGEK kind) : GenericEdge<InterProcCFGNode>(s, d, kind) {}
    virtual ~InterProcCFGEdge() {}
};

class InterProcCFGNode : public GenericNode<InterProcCFGNode, InterProcCFGEdge> {
private:
    llvm::BasicBlock* bb;
    InterProcCFG* cfg;
    BlockStat* stat;
public:
    InterProcCFGNode(NodeID i, llvm::BasicBlock* _bb, InterProcCFG* _cfg) : GenericNode<InterProcCFGNode, InterProcCFGEdge>(i, 0), bb(_bb), cfg(_cfg), stat(NULL) {}
    virtual ~InterProcCFGNode() { delete stat; }

    inline llvm::BasicBlock* getBasicBlock() const { return bb; }
    InterProcCFG* getCFG() { return cfg; }
    BlockStat* getStat() { return stat; }
    void setStat(BlockStat* _stat) { stat = _stat; }
};

class InterProcCFG : public GenericGraph<InterProcCFGNode, InterProcCFGEdge> {
public:
    typedef llvm::DenseMap<const llvm::BasicBlock*, InterProcCFGNode*> BBCFGMap;
    typedef llvm::DenseMap<const InterProcCFGNode*, NodeID> NodeIDMap;

private:
    llvm::Module* mod;
    NodeID totalNodeNum;
    BBCFGMap bbToCFGNodeMap;
    NodeIDMap nodeToIDMap;
    InterProcCFGNode* entry;

    bool termInst(llvm::Instruction &dest);
    void addCFGEdge(const llvm::BasicBlock* src, const llvm::BasicBlock* dst, InterProcCFGEdge::CEDGEK kind);
    void addCFGEdge(InterProcCFGNode* src, InterProcCFGNode* dst, InterProcCFGEdge::CEDGEK kind);
public:
    InterProcCFG(llvm::Module* _mod);
    virtual ~InterProcCFG();

    inline InterProcCFGNode* getNode(const llvm::BasicBlock* bb) const {
        BBCFGMap::const_iterator it = bbToCFGNodeMap.find(bb);
        assert(it != bbToCFGNodeMap.end() && "node for basic block not found!");
        return it->second;
    }

    inline NodeID getNodeID(const InterProcCFGNode* node) const {
        NodeIDMap::const_iterator it = nodeToIDMap.find(node);
        assert(it != nodeToIDMap.end() && "node not found!");
        return it->second;
    }

    inline InterProcCFGNode* getEntryNode() {
        return entry;
    }

    unsigned getTotalNodeNum() {
        return totalNodeNum;
    }

    bool hasEdge(InterProcCFGNode* src, InterProcCFGNode* dst, InterProcCFGEdge::CEDGEK kind) const;
    void computeScore();
    void computeScoreLinear();
    void computeScoreSquare();
    void dump(const std::string& filename);
};

namespace llvm {
    template<> struct GraphTraits<InterProcCFGNode*> : public GraphTraits<GenericNode<InterProcCFGNode, InterProcCFGEdge>*> {
        typedef InterProcCFGNode* NodeRef;
    };

    template<> struct GraphTraits<Inverse<InterProcCFGNode*>> : public GraphTraits<Inverse<GenericNode<InterProcCFGNode, InterProcCFGEdge>*>> {
        typedef InterProcCFGNode* NodeRef;
    };

    template<> struct GraphTraits<InterProcCFG*> : public GraphTraits<GenericGraph<InterProcCFGNode, InterProcCFGEdge>*> {
        typedef InterProcCFGNode* NodeRef;
        static NodeRef getEntryNode(InterProcCFG* g) { return g->getEntryNode(); }
        static unsigned size(InterProcCFG* g) { return g->getTotalNodeNum(); }
    };
}

#endif
