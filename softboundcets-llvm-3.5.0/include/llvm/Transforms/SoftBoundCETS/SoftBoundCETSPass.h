//=== SoftBoundCETS/SoftBoundCETSPass.h - Definitions for the SoftBound/CETS --*- C++ -*===// 
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


#ifndef SOFTBOUNDCETSPASS_H
#define SOFTBOUNDCETSPASS_H

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

typedef IRBuilder<true, TargetFolder> BuilderTy;

class SoftBoundCETSPass: public ModulePass {

 private:
  const DataLayout *TD;
  const TargetLibraryInfo *TLI;
  BuilderTy *Builder;
  SmallString<64> BlacklistFile;
  std::unique_ptr<SpecialCaseList> Blacklist;

  bool spatial_safety;
  bool temporal_safety;
  
  Function* m_introspect_metadata;
  Function* m_copy_metadata;
  Function* m_shadow_stack_allocate;
  Function* m_shadow_stack_deallocate;
  Function* m_shadow_stack_base_load;
  Function* m_shadow_stack_bound_load;
  Function* m_shadow_stack_key_load;
  Function* m_shadow_stack_lock_load;
  
  Function* m_shadow_stack_base_store;
  Function* m_shadow_stack_bound_store;
  Function* m_shadow_stack_key_store;
  Function* m_shadow_stack_lock_store;
  
  Function* m_spatial_load_dereference_check;
  Function* m_spatial_store_dereference_check;
  
  Function* m_temporal_stack_memory_allocation;
  Function* m_temporal_stack_memory_deallocation;

  Function* m_temporal_load_dereference_check;
  Function* m_temporal_store_dereference_check;
  Function* m_temporal_global_lock_function;
  
  Function* m_call_dereference_func;
  Function* m_memcopy_check;
  Function* m_memset_check;

  Function* m_metadata_map_func;
  Function* m_metadata_load_base_func;
  Function* m_metadata_load_bound_func;
  Function* m_metadata_load_key_func;
  Function* m_metadata_load_lock_func;
  
  /* Function Type of the function that loads the base and bound for
   * a given pointer 
   */
  Function* m_load_base_bound_func;
  Function* m_metadata_load_vector_func;
  Function* m_metadata_store_vector_func;
  

  /* Function Type of the function that stores the base and bound
   * for a given pointer
   */
  Function* m_store_base_bound_func;
  
  /* void pointer type, used many times in the Softboundcets pass */
  Type* m_void_ptr_type;
  Type* m_sizet_ptr_type;
  VectorType* m_base_bound_ty;
  VectorType* m_key_lock_ty;
  
  /* constant null pointer which is the base and bound for most
   * non-pointers 
   */
  ConstantPointerNull* m_void_null_ptr;
  ConstantPointerNull* m_sizet_null_ptr;
  Type* m_key_type;

  Constant* m_constantint_one;
  Constant* m_constantint_zero;

  Constant* m_constantint32ty_one;
  Constant* m_constantint32ty_zero;
  Constant* m_constantint64ty_one;   
  Constant* m_constantint64ty_zero;
    
  /* Infinite bound where bound cannot be inferred in VarArg
   * functions
   */
  Value* m_infinite_bound_ptr;
    
  
  /* Dominance Tree and Dominance Frontier for avoiding load
   * dereference checks 
   */


  DominatorTree* m_dominator_tree;
  
  /* Book-keeping structures for identifying original instructions in
   * the program, pointers and their corresponding base and bound
   */
  std::map<Value*, int> m_is_pointer;
  std::map<Value*, Value*> m_pointer_base;

  std::map<Value*, Value*> m_vector_pointer_base;
  std::map<Value*, Value*> m_vector_pointer_bound;


  std::map<Value*, Value*> m_pointer_bound;
  std::map<Value*, BasicBlock*> m_faulting_block;

    
  /* key associated with pointer */

  std::map<Value*, Value*> m_vector_pointer_key;
  std::map<Value*, Value*> m_vector_pointer_lock;

  std::map<Value*, Value*> m_pointer_key;
  /* address of the location to load the key from */
  std::map<Value*, Value*> m_pointer_lock;  
  std::map<Value*, int> m_present_in_original;


  std::map<GlobalVariable*, int> m_initial_globals;
  
  /* Map of all functions for which Softboundcets Transformation must
   * be invoked
   */
  StringMap<bool> m_func_softboundcets_transform;
  
  /* Map of all functions that need to be transformed as they have as
   * they either hava pointer arguments or pointer return type and are
   * defined in the module
   */
  StringMap<bool> m_func_to_transform;
  
  /* Map of all functions defined by Softboundcets */
  StringMap<bool> m_func_def_softbound;

  StringMap<bool> m_func_wrappers_available;
  
  /* Map of all functions transformed */
  StringMap<bool> m_func_transformed;
  
  StringMap<Value*> m_func_global_lock;
  
  /* Boolean indicating whether bitcode generated is for 64bit or
     32bit */
  bool m_is_64_bit;
  
  /* Main functions implementing the structure of the Softboundcets
     pass
   */
  bool runOnModule(Module&);
  void initializeSoftBoundVariables(Module&);
  void identifyOriginalInst(Function*);
  bool isAllocaPresent(Function*);
  void gatherBaseBoundPass1(Function*);
  void gatherBaseBoundPass2(Function*);
  void addDereferenceChecks(Function*);
  bool checkIfFunctionOfInterest(Function*);
  bool isFuncDefSoftBound(const std::string &str);
  std::string transformFunctionName(const std::string &str);
  void runForEachFunctionIndirectCallPass(Function&);
  void indirectCallInstPass(Module&);
  bool checkStructTypeWithGEP(BasicBlock*, std::map<Value*, int> &, 
                              Value*, BasicBlock::iterator);
  
  
  /* Specific LLVM instruction handlers in the bitcode */
  void handleAlloca(AllocaInst*, Value*, Value*, 
                    Value*, BasicBlock*,  
                    BasicBlock::iterator&);  
  
  void insertMetadataLoad(LoadInst*);
  void handleLoad(LoadInst*);
  void handleVectorStore(StoreInst*);
  void handleStore(StoreInst*);
  void handleGEP(GetElementPtrInst*);

  void handleBitCast(BitCastInst*);
  void handlePHIPass1(PHINode*);
  void handlePHIPass2(PHINode*);
  void handleCall(CallInst*);
  void handleMemcpy(CallInst*);
  void handleIndirectCall(CallInst*);
  void handleExtractValue(ExtractValueInst*);
  void handleExtractElement(ExtractElementInst*);
  void handleSelect(SelectInst*, int);
  void handleIntToPtr(IntToPtrInst*);
  void identifyFuncToTrans(Module&);
  
  void transformFunctions(Module&);
  bool transformIndividualFunction(Module&);  
  bool hasPtrArgRetType(Function*);
  void iterateOverSuccessors(Function&);
  void transformExternalFunctions(Module&);
  bool transformIndividualExternalFunctions(Module&);
  void transformMain(Module&);
  void renameFunctions(Module&);
  void renameFunctionName(Function*, Module&, bool);
  bool checkAndShrinkBounds(GetElementPtrInst*, 
                            Value*);

  bool checkTypeHasPtrs(Argument*);
  bool checkPtrsInST(StructType*);
  bool isByValDerived(Value*);
  
  bool checkBitcastShrinksBounds(Instruction* );
  bool isStructOperand(Value*);
  void addLoadStoreChecks(Instruction*, 
                          std::map<Value*, int>&);
  void addTemporalChecks(Instruction*, 
                         std::map<Value*, int>&, 
                         std::map<Value*, int>&);

  bool optimizeTemporalChecks(Instruction*, 
                              std::map<Value*, int>&, 
                              std::map<Value*,int>&);

  bool bbTemporalCheckElimination(Instruction*, 
                                  std::map<Value*, int>&);
  
  bool funcTemporalCheckElimination(Instruction*, 
                                    std::map<Value*, int>&);
  
  bool optimizeGlobalAndStackVariableChecks(Instruction*);
  bool checkLoadStoreSourceIsGEP(Instruction*, Value*);
  void addMemcopyMemsetCheck(CallInst*, Function*);
  bool isMemcopyFunction(Function*);

  void getFunctionKeyLock(Function*, Value* &, Value* &, Value* &);
  void freeFunctionKeyLock(Function*, Value* &, Value* &, Value* &);
  Value* getPointerLoadStore(Instruction*);
  void propagateMetadata(Value*, Instruction*, int);
  
  void getFunctionKeyLock(Function &, Value* &, Value* &, Value* &);
  void addMemoryAllocationCall(Function*, Value* &, Value* & , 
                               Instruction*) ;

  
  enum { SBCETS_BITCAST, SBCETS_GEP};
  /* Auxillary base and propagation functions */

  void handleGlobalSequentialTypeInitializer(Module&, GlobalVariable*);
  void handleGlobalStructTypeInitializer(Module& , StructType* , 
                                         Constant* , GlobalVariable*, 
                                         std::vector<Constant*>, int) ;

  void addBaseBoundGlobals(Module&);
  Instruction* getGlobalInitInstruction(Module&);
  void identifyInitialGlobals(Module&);
  void getGlobalVariableBaseBound(Value*, Value* &, Value* &);
  void dissociateBaseBound(Value*);
  void dissociateKeyLock(Value*);
  
  /* Explicit Map manipulation functions */

  /* Single function that adds base/bound/key to the pointer map,
   * first argument - pointer operand
   * second argument - associated base
   * third argument - associated bound 
   * fourth argument - associated key 
   * fifth argument - associated lock 
   */
  void associateBaseBoundKeyLock(Value*, Value*, Value*, Value*, Value*);
  void associateXMMBaseBoundKeyLock(Value*, Value*, Value*);
  
  /* XMM mode functions for base/bound and key/lock extraction */        
  void associateBaseBound(Value*, Value*, Value* );

  void associateKeyLock(Value*, Value*, Value*);
    
  /* Returns the base associated with the pointer value */
  Value* getAssociatedBase(Value*);
  
  /* Returns the bound associated with the pointer value */
  Value* getAssociatedBound(Value*);

  Value* getAssociatedKey(Value*);  
  Value* getAssociatedFuncLock(Value*);

  Value* getAssociatedLock(Value*, Value*);
      
  bool checkBaseBoundMetadataPresent(Value*);
  
  bool checkKeyLockMetadataPresent(Value*);

  /* Function to add a call to m_store_base_bound_func */
  void addStoreBaseBoundFunc(Value*, Value*, Value*,Value*, 
                             Value*, Value*, Value*, Instruction*);
  
  void addStoreXMMBaseBoundFunc(Value*, Value*, Value*, Instruction*);
  
  void setFunctionPtrBaseBound(Value*, Instruction*);
  
  void replaceAllInMap(std::map<Value*, Value*> &, 
                         Value*, Value*);
  
  void castAddToPhiNode(PHINode* , Value*, BasicBlock*, 
                        std::map<Value*, Value*>&, Value*);
  
  void getConstantExprBaseBound(Constant*,  
                                Value* &, Value* &);
  
  Value* castAndReplaceAllUses(Value*, Value*, Instruction*);
  
  bool checkIfNonCallUseOfFunction(Function*);
  
  
  /* Other helper functions */
  
  Value* introduceGEPWithLoad(Value*, int, Instruction*);
  Value* storeShadowStackBaseForFunctionArgs(Instruction*, int);
  Value* storeShadowStackBoundForFunctionArgs(Instruction*, int);
  Value* storeShadowStackKeyForFunctionArgs(Instruction*, int);
  Value* storeShadowStackLockForFunctionArgs(Instruction*, int);
  
  Value* retrieveShadowStackBaseForFunctionArgs(Instruction*, int );
  Value* retrieveShadowStackBoundForFunctionArgs(Instruction*, int);
  Value* retrieveShadowStackKeyForFunctionArgs(Instruction*, int);
  Value* retrieveShadowStackLockForFunctionArgs(Instruction*, int);
    
  Value* introduceGlobalLockFunction(Instruction*);
  void introspectMetadata(Function*, Value*, Instruction*, int);
  void introduceShadowStackLoads(Value*, Instruction*, int);
  void introduceShadowStackAllocation(CallInst*);
  void iterateCallSiteIntroduceShadowStackStores(CallInst*);
  void introduceShadowStackStores(Value*, Instruction*, int);
  void introduceShadowStackDeallocation(CallInst*, Instruction*);
  int getNumPointerArgsAndReturn(CallInst*);

  void checkIfRetTypePtr(Function*, bool &);
  Instruction* getReturnInst(Function*, int);
  
  // 
  // Method: getNextInstruction
  // 
  // Description:
  // This method returns the next instruction after the input instruction.
  //
  
  Instruction* getNextInstruction(Instruction* I){
    
    if (isa<TerminatorInst>(I)) {
      return I;
    } else {
      BasicBlock::iterator i = I;
      return ++i;
    }    
  }
  
  const Type* getStructType(const Type*);
  Value*  getSizeOfType(Type*);
  
  Value* castToVoidPtr(Value*, Instruction*);
  bool checkGEPOfInterestSB(GetElementPtrInst*);
  void handleReturnInst(ReturnInst*);    
  
 public:
  static char ID;


  /* INITIALIZE_PASS(SoftBoundCETSPass, "softboundcetspass", */
  /*               "SoftBound CETS for memory safety", false, false) */
    
    
 SoftBoundCETSPass(StringRef BlacklistFile = "")
   : ModulePass(ID),
    BlacklistFile(BlacklistFile){
    spatial_safety= true;
    temporal_safety=true;

    //    initializeSoftBoundCETSPass(*PassRegistry::getPassRegistry());

  }
  const char* getPassName() const { return " SoftBoundCETSPass";}


  void getAnalysisUsage(AnalysisUsage& au) const override {

#if 0
    au.addRequired<DominatorTreeWrapperPass>();
#endif 

    au.addRequired<LoopInfo>();
    //    au.addRequired<DataLayout>();
    au.addRequired<TargetLibraryInfo>();
  }


};

#if 0
INITIALIZE_PASS_BEGIN(SoftBoundCETSPass, "SoftBoundCETSPass", "SoftBoundCETS Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass);
INITIALIZE_PASS_END(SoftBoundCETSPass, "SoftBoundCETSPass", "SoftBoundCETS Pass", false, false)
#endif

#endif
