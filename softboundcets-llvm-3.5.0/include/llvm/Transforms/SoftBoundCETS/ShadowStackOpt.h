//=== SoftBoundCETS/SpatialCheck.h - Definitions for the SoftBoundCETS Spatial Check Optimizations--*- C++ -*===// 
// Copyright (c) 2014 Santosh Nagarakatte, Milo M. K. Martin. All rights reserved.
//
// Developed by: Santosh Nagarakatte, 
//               Department of Computer Science,
//               Rutgers University
//               http://www.cs.rutgers.edu/~santosh.nagarakatte/softbound/
//               
//               in collaboration with
//               Milo Martin, Jianzhou Zhao, Steve Zdancewic
//               University of Pennsylvania
//
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal with the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

//   1. Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimers.

//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimers in the
//      documentation and/or other materials provided with the distribution.

//   3. Neither the names of Santosh Nagarakatte, Milo M. K. Martin,
//      Jianzhou Zhao, Steve Zdancewic, University of Pennsylvania,
//      Rutgers University, nor the names of its contributors may be
//      used to endorse or promote products derived from this Software
//      without specific prior written permission.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// WITH THE SOFTWARE.

//===---------------------------------------------------------------------===//

#ifndef SHADOW_STACK_OPT_H
#define SHADOW_STACK_OPT_H

#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"

#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/STLExtras.h"
#include <algorithm>
#include <cstdarg>


#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"

#include "llvm-c/Target.h"
#include "llvm-c/TargetMachine.h"

#include "llvm/IR/Dominators.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallString.h"
//#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetFolder.h"
#include "llvm/Support/SpecialCaseList.h"
#include "llvm/Support/Compiler.h"
//#include "llvm/Support/Debug.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include <cstdlib>
#include <memory>
#include<queue>

using namespace llvm;

class ShadowStackOpt: public ModulePass {

 private:
  bool runOnModule(Module &);
  void transformFunctions();
  void transformIndividualFunction(Function*);
  
  std::vector<Function*> FunctionsToTransform;
  bool checkFunctionOfInterest(Function*);
  StructType* getStructMetadataType(Type*);
  void handleCallUseOfFunction(CallInst*, Function *);
  void ProcessReturnInst(ReturnInst*, Function*);
  void ProcessArgsandRemoveShadowCalls(Function*, Function*);
  void ReplaceShadowStackLoads(Value*, Value*, Value*, Value*, CallInst*);

  Type* m_void_ptr_type;
  Type* m_sizet_type;

  Instruction* getPrevInstruction(Instruction * I){
    
    BasicBlock::iterator i = I;
    return --i;

  }

  Instruction* getNextInstruction(Instruction* I){
    
    if (isa<TerminatorInst>(I)) {
      return I;
    } else {
      BasicBlock::iterator i = I;
      return ++i;
    }    
  }

  CallInst* getShadowStackRetHandler(CallInst*, StringRef);
  
  Instruction* getShadowStackLoadHandler(Instruction*, StringRef, int);

  Value* getShadowStackArgHandler(Instruction*, StringRef, int);

#if 0
  CallInst* getShadowStackRetBase(CallInst*);
  CallInst* getShadowStackRetBound(CallInst*);
  CallInst* getShadowStackRetKey(CallInst*);
  CallInst* getShadowStackRetLock(CallInst*);
  CallInst* getShadowStackDeallocate(CallInst*);
#endif

 public:
  static char ID;
  
 ShadowStackOpt(): ModulePass(ID){
  }

  const char* getPassName() const {return "ShadowStackOpt";}

  void getAnalysisUsage(AnalysisUsage& au) const override {
    //    au.addRequired<DominatorTreeWrapperPass>();
  }

};


#endif
