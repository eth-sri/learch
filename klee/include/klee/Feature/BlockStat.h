#ifndef BLOCK_STAT_H_
#define BLOCK_STAT_H_

#include <string>
#include <vector>
#include <algorithm>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfoMetadata.h"

extern std::vector<std::string> overflowLabels;
extern std::vector<std::string> shiftLabels;
extern std::vector<std::string> oobLabels;
extern std::vector<std::string> pointerLabels;
extern std::vector<std::string> nullLabels;

class BlockStat {
public:
    BlockStat() : num_block(0), num_inst(0), num_source(0), num_bug(0), num_overflow(0), num_shift(0), num_oob(0), num_pointer(0), num_null(0) {}
    virtual ~BlockStat() {}

    BlockStat* copy() {
        BlockStat* other = new BlockStat();
        other->num_block = num_block;
        other->num_inst = num_inst;
        other->num_source = num_source;
        other->num_bug = num_bug;
        other->num_overflow = num_overflow;
        other->num_shift = num_shift;
        other->num_oob = num_oob;
        other->num_pointer = num_pointer;
        other->num_null = num_null;
        return other;
    }

    void analyze(llvm::BasicBlock* bb) {
        num_block += 1;
        for (llvm::BasicBlock::iterator inst = bb->begin(); inst != bb->end(); ++inst) {
            num_inst += 1;
            const llvm::DebugLoc& debugLoc = inst->getDebugLoc();
            if (debugLoc.get() != nullptr) {
                std::string directory = debugLoc->getDirectory();
                std::string filePath = debugLoc->getFilename();
                int line = debugLoc->getLine();
                std::string cov = directory + filePath + std::to_string(line);
                if (cov.find("environment") != std::string::npos) {
                    num_source += 1;
                }
            }
            if (llvm::isa<llvm::CallInst>(inst) || llvm::isa<llvm::InvokeInst>(inst)) {
                llvm::Function *func;
                if (llvm::isa<llvm::CallInst>(inst)) {
                    func = llvm::cast<llvm::CallInst>(inst)->getCalledFunction();
                } else {
                    func = llvm::cast<llvm::InvokeInst>(inst)->getCalledFunction();
                }
                if (func && func->hasName()) {
                    std::string funcName = func->getName().str();
                    if (std::find(overflowLabels.begin(), overflowLabels.end(), funcName) != overflowLabels.end()) {
                        num_bug += 1;
                        num_overflow += 1;
                    }

                    if (std::find(shiftLabels.begin(), shiftLabels.end(), funcName) != shiftLabels.end()) {
                        num_bug += 1;
                        num_shift += 1;
                    }

                    if (std::find(oobLabels.begin(), oobLabels.end(), funcName) != oobLabels.end()) {
                        num_bug += 1;
                        num_oob += 1;
                    }

                    if (std::find(pointerLabels.begin(), pointerLabels.end(), funcName) != pointerLabels.end()) {
                        num_bug += 1;
                        num_pointer += 1;
                    }

                    if (std::find(nullLabels.begin(), nullLabels.end(), funcName) != nullLabels.end()) {
                        num_bug += 1;
                        num_null += 1;
                    }
                }
            }
        }
    }

    void update(BlockStat* other) {
        num_block += other->num_block;
        num_inst += other->num_inst;
        num_source += other->num_source;
        num_bug += other->num_bug;
        num_overflow += other->num_overflow;
        num_shift += other->num_shift;
        num_oob += other->num_oob;
        num_pointer += other->num_pointer;
        num_null += other->num_null;
    }

    void divide(unsigned d) {
        num_block /= d;
        num_inst /= d;
        num_source /= d;
        num_bug /= d;
        num_overflow /= d;
        num_shift /= d;
        num_oob /= d;
        num_pointer /= d;
        num_null /= d;
    }

    void dump(llvm::BasicBlock* bb) {
        insert_feature(bb, "num_block", num_block);
        insert_feature(bb, "num_inst", num_inst);
        insert_feature(bb, "num_source", num_source);
        insert_feature(bb, "num_bug", num_bug);
        insert_feature(bb, "num_overflow", num_overflow);
        insert_feature(bb, "num_shift", num_shift);
        insert_feature(bb, "num_oob", num_oob);
        insert_feature(bb, "num_pointer", num_pointer);
        insert_feature(bb, "num_null", num_null);
    }

    void insert_feature(llvm::BasicBlock* bb, std::string feature_type, unsigned feature) {
        llvm::Instruction* inst = &*(bb->begin());
        llvm::LLVMContext& c = inst->getContext();
        llvm::MDNode* n = llvm::MDNode::get(c, llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(c), feature)));
        inst->setMetadata(feature_type, n);
    }

    unsigned num_block;
    unsigned num_inst;
    unsigned num_source;
    unsigned num_bug;
    unsigned num_overflow;
    unsigned num_shift;
    unsigned num_oob;
    unsigned num_pointer;
    unsigned num_null;
};

#endif