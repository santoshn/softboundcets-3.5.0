//=== SoftBoundCETS/SpatialCheck.cpp --*- C++ -*=====///
// Spatial Check Optimizations for SoftBoundCETS and related passes
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
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal with the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
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

#include "llvm/Transforms/SoftBoundCETS/SpatialCheck.h"

bool SpatialCheckOpt::isSpatialCheckInstruction(Instruction* I){

  CallInst* call = dyn_cast<CallInst>(I);
  Module* mod = I->getParent()->getParent()->getParent();


  Function* func = call->getCalledFunction();

  if(func){
    if((func->getName().find("__softboundcets_spatial_load_dereference_check") ==0) ||
       (func->getName().find("__softboundcets_spatial_store_dereference_check")==0) ||
       (func->getName().find("__softboundcetsmpx_spatial_load_dereference_check")==0) ||
       (func->getName().find("__softboundcetsmpx_spatial_store_dereference_check")==0)){
      return true;
    }
  }

  return false;

}

char SpatialCheckOpt::ID = 0;

static RegisterPass<SpatialCheckOpt> P ("SpatialCheckOpt", 
					"Spatial Check Optimizations");


void SpatialCheckOpt::EliminateChecks(){

  for(std::vector<CallInst*>::iterator i = RemoveSpatialCheckCalls.begin(), e = RemoveSpatialCheckCalls.end(); i != e; ++i){
    
    CallInst* call = *i;

    call->eraseFromParent();

  }

}


void SpatialCheckOpt::IdentifyDominatingCalls(){

  for(std::vector<CallInst*>::iterator i = SpatialCheckCalls.begin(), e = SpatialCheckCalls.end(); i!= e; ++i){
    
    CallInst* call= *i;

    if(RemoveCallsMap.count(call))
      continue;

    /* operands start from 0, so third operand is 2 */
    Value* call_operand = call->getArgOperand(2);

    Value* pointer = NULL;
    if(isa<BitCastInst>(call_operand)){
      Instruction* bit = dyn_cast<BitCastInst>(call_operand);
      pointer = bit->getOperand(0);
    }

    if(pointer == NULL)
      continue;

    GlobalVariable* gv = dyn_cast<GlobalVariable>(pointer);
    if(gv)
      continue;

    /* identify the pointer that is being checked */

    /* iterate over the uses of the pointer */

    /* if the use happens to be a bitcast, then check the uses of the bitcast and see if it is a spatial check inst */

    /* Check if the bitcast dominates the orignal call */

    for (User *U : pointer->users()) {

      Instruction* Inst = dyn_cast<Instruction>(U);

      if(!Inst)
	continue;

      
      if(isa<BitCastInst> (Inst)){
	for (User * NU: Inst->users()){
	  CallInst * ci = dyn_cast<CallInst>(NU);
	  if(ci && isSpatialCheckInstruction(ci) && m_dominator_tree->dominates(call, ci)){

	    if(RemoveCallsMap.count(ci)) continue;
	    RemoveSpatialCheckCalls.push_back(ci);
	    RemoveCallsMap[ci] = true;
	  }
	}
      }
    } /* iterating over the pointer's  use ends */
  } /* iterating over all check ends */

}


bool SpatialCheckOpt::runOnFunction(Function& F){
  
  m_dominator_tree = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  SpatialCheckCalls.clear();
  RemoveSpatialCheckCalls.clear();
  RemoveCallsMap.clear();

  for(inst_iterator i = inst_begin(F), e = inst_end(F); i!=e; ++i){
    
    Instruction* I = &*i;
    if(isa<CallInst>(I)){
      if(isSpatialCheckInstruction(I)){
	CallInst* CI = dyn_cast<CallInst>(I);
	SpatialCheckCalls.push_back(CI);
      }
    }
  }

  IdentifyDominatingCalls();
  EliminateChecks();
  return true;

}
