// Prototype creator for SoftBoundCETSMPX Pass

#ifndef INITIALIZE_SOFTBOUNDCETSMPX_H
#define INITIALIZE_SOFTBOUNDCETSMPX_H

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm-c/Target.h"
#include "llvm-c/TargetMachine.h"


using namespace llvm;

class InitializeSoftBoundCETSMPX: public ModulePass {

 public:
  bool runOnModule(Module &);
  static char ID;

  void constructCheckHandlers(Module &);
  void constructMetadataHandlers(Module &);
  void constructShadowStackHandlers(Module &);
  void constructAuxillaryFunctionHandlers(Module &);
  InitializeSoftBoundCETSMPX(): ModulePass(ID){        
  }
  
  const char* getPassName() const { return "InitializeSoftBoundCETSMPX";}
};

#endif
