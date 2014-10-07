//=== SoftBoundCETS/InitializeSoftBoundCETSMPX.cpp - Helper Pass for SoftBound/CETS --*- C++ -*===// 
// Copyright (c) 2014 Santosh Nagarakatte. All rights reserved.

// Developed by: Santosh Nagarakatte

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

//   3. Neither the names of Santosh Nagarakatte nor
//      the names of its contributors may be used to endorse or promote
//      products derived from this Software without specific prior
//      written permission.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// WITH THE SOFTWARE.
//===---------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/SoftBoundCETS/InitializeSoftBoundCETSMPX.h"

char InitializeSoftBoundCETSMPX :: ID = 0;

static RegisterPass<InitializeSoftBoundCETSMPX> P 
("InitializeSoftBoundCETSMPX","Prototype Creator Pass for Softboundcetsmpx");

void InitializeSoftBoundCETSMPX:: constructShadowStackHandlers(Module & module){

  Type* VoidTy = Type::getVoidTy(module.getContext());
  Type* 
    VoidPtrTy = PointerType::getUnqual(Type::getInt8Ty(module.getContext()));
  Type* SizeTy = Type::getInt64Ty(module.getContext());
  
  Type* Int32Ty = Type::getInt32Ty(module.getContext());
  module.getOrInsertFunction("__softboundcetsmpx_allocate_shadow_stack_space", 
                             VoidTy, Int32Ty, NULL);
  module.getOrInsertFunction("__softboundcetsmpx_deallocate_shadow_stack_space", 
                             VoidTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_load_base_shadow_stack", 
			     VoidPtrTy, Int32Ty, NULL);
  module.getOrInsertFunction("__softboundcetsmpx_load_bound_shadow_stack", 
                             VoidPtrTy, Int32Ty, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_load_key_shadow_stack", 
			     SizeTy, Int32Ty, NULL);
  module.getOrInsertFunction("__softboundcetsmpx_load_lock_shadow_stack", 
			     VoidPtrTy, Int32Ty, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_store_base_shadow_stack", 
			     VoidTy, VoidPtrTy, Int32Ty, NULL);
  module.getOrInsertFunction("__softboundcetsmpx_store_bound_shadow_stack", 
			     VoidTy, VoidPtrTy, Int32Ty, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_store_key_shadow_stack", 
			     VoidTy, SizeTy, Int32Ty, NULL);
  module.getOrInsertFunction("__softboundcetsmpx_store_lock_shadow_stack", 
			     VoidTy, VoidPtrTy, Int32Ty, NULL);


}

void InitializeSoftBoundCETSMPX:: constructMetadataHandlers(Module & module){

  Type* VoidTy = Type::getVoidTy(module.getContext());
  Type* 
    VoidPtrTy = PointerType::getUnqual(Type::getInt8Ty(module.getContext()));
  Type* SizeTy = Type::getInt64Ty(module.getContext());
  
  Type* Int32Ty = Type::getInt32Ty(module.getContext());

  module.getOrInsertFunction("__softboundcetsmpx_introspect_metadata", 
                             VoidTy, VoidPtrTy, VoidPtrTy, Int32Ty, NULL);
  module.getOrInsertFunction("__softboundcetsmpx_copy_metadata", 
                             VoidTy, VoidPtrTy, VoidPtrTy, SizeTy, NULL);

  Type* PtrVoidPtrTy = PointerType::getUnqual(VoidPtrTy);
  Type* PtrSizeTy = PointerType::getUnqual(SizeTy);
  
  module.getOrInsertFunction("__softboundcetsmpx_metadata_map",
			     VoidPtrTy, VoidPtrTy, NULL);
    
  module.getOrInsertFunction("__softboundcetsmpx_metadata_load_base",
			     VoidPtrTy, VoidPtrTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_metadata_load_bound",
			     VoidPtrTy, VoidPtrTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_metadata_load_key",
			     SizeTy, VoidPtrTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_metadata_load_lock",
			     VoidPtrTy, VoidPtrTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_metadata_load_vector", 
			     VoidTy, VoidPtrTy, PtrVoidPtrTy, PtrVoidPtrTy, 
			     PtrSizeTy, PtrVoidPtrTy, Int32Ty, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_metadata_store_vector", 
			     VoidTy, VoidPtrTy, VoidPtrTy, 
			     VoidPtrTy, SizeTy, VoidPtrTy, VoidPtrTy, Int32Ty, NULL);
    
  module.getOrInsertFunction("__softboundcetsmpx_metadata_load", 
			     VoidTy, VoidPtrTy, PtrVoidPtrTy, PtrVoidPtrTy, 
			     PtrSizeTy, PtrVoidPtrTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_metadata_store", 
			     VoidTy, VoidPtrTy, VoidPtrTy, 
			     VoidPtrTy, SizeTy, VoidPtrTy, VoidPtrTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_memcopy_check",
			     VoidTy, VoidPtrTy, VoidPtrTy, SizeTy, 
			     VoidPtrTy, VoidPtrTy, VoidPtrTy, VoidPtrTy,
			     SizeTy, VoidPtrTy, SizeTy, VoidPtrTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_memset_check",
			     VoidTy, VoidPtrTy, SizeTy, 
			     VoidPtrTy, VoidPtrTy, SizeTy, VoidPtrTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_get_global_lock", 
                             VoidPtrTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_stack_memory_allocation", 
                             VoidTy, PtrVoidPtrTy, 
                             PtrSizeTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_stack_memory_deallocation", 
                             VoidTy, SizeTy, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_spatial_call_dereference_check",
                             VoidTy, VoidPtrTy, VoidPtrTy, VoidPtrTy, NULL);


  Type* size_ty = Type::getInt64Ty(module.getContext());

  PointerType* sizet_ptr_ty = PointerType::getUnqual(Type::getInt64Ty(module.getContext()));

  module.getOrInsertFunction("__softboundcetsmpx_print_metadata", VoidTy, VoidPtrTy, VoidPtrTy, VoidPtrTy, size_ty, sizet_ptr_ty, NULL);

  
  module.getOrInsertFunction("__softboundcetsmpx_dummy", VoidTy, NULL);

  Type* bool_ty = Type::getInt1Ty(module.getContext());

  module.getOrInsertFunction("__softboundcetsmpx_intermediate", VoidTy, bool_ty, bool_ty, bool_ty, size_ty, NULL);

}

void InitializeSoftBoundCETSMPX:: constructCheckHandlers(Module & module){

  Type* void_ty = Type::getVoidTy(module.getContext());

  Type* void_ptr_ty = PointerType::getUnqual(Type::getInt8Ty(module.getContext()));
  Type* size_ty = Type::getInt64Ty(module.getContext());

  module.getOrInsertFunction("__softboundcetsmpx_spatial_load_dereference_check",
                             void_ty, void_ptr_ty, void_ptr_ty, 
                             void_ptr_ty, size_ty, NULL);

  module.getOrInsertFunction("__softboundcetsmpx_spatial_store_dereference_check", 
                             void_ty, void_ptr_ty, void_ptr_ty, 
                             void_ptr_ty, size_ty, NULL);

  
  module.getOrInsertFunction("__softboundcetsmpx_temporal_load_dereference_check", 
			     void_ty, void_ptr_ty, size_ty, 
			     void_ptr_ty, void_ptr_ty, NULL);
  
  module.getOrInsertFunction("__softboundcetsmpx_temporal_store_dereference_check", 
			     void_ty, void_ptr_ty, size_ty, 
			     void_ptr_ty, void_ptr_ty, NULL);
  

  Function* global_init = (Function *) module.getOrInsertFunction("__softboundcetsmpx_global_init", 
                                                                  void_ty, NULL);

  global_init->setDoesNotThrow();
  global_init->setLinkage(GlobalValue::InternalLinkage);

  BasicBlock* BB = BasicBlock::Create(module.getContext(), 
                                      "entry", global_init);
  
  Function* softboundcetsmpx_init = (Function*) module.getOrInsertFunction("__softboundcetsmpx_init", void_ty, Type::getInt32Ty(module.getContext()), NULL);

  
  SmallVector<Value*, 8> args;
  Constant * const_one = ConstantInt::get(Type::getInt32Ty(module.getContext()), 1);
  
  args.push_back(const_one);
  Instruction* ret = ReturnInst::Create(module.getContext(), BB);
  
  CallInst::Create(softboundcetsmpx_init, args, "", ret);



  Type * Int32Type = IntegerType::getInt32Ty(module.getContext());
  std::vector<Constant *> CtorInits;
  CtorInits.push_back(ConstantInt::get(Int32Type, 0));
  CtorInits.push_back(global_init);
  StructType * ST = ConstantStruct::getTypeForElements(CtorInits, false);
  Constant * RuntimeCtorInit = ConstantStruct::get(ST, CtorInits);

  //
  // Get the current set of static global constructors and add the new ctor
  // to the list.
  //
  std::vector<Constant *> CurrentCtors;
  GlobalVariable * GVCtor = module.getNamedGlobal ("llvm.global_ctors");
  if (GVCtor) {
    if (Constant * C = GVCtor->getInitializer()) {
      for (unsigned index = 0; index < C->getNumOperands(); ++index) {
        CurrentCtors.push_back (dyn_cast<Constant>(C->getOperand (index)));
      }
    }
  }
  CurrentCtors.push_back(RuntimeCtorInit);

  //
  // Create a new initializer.
  //
  ArrayType * AT = ArrayType::get (RuntimeCtorInit-> getType(),
                                   CurrentCtors.size());
  Constant * NewInit = ConstantArray::get (AT, CurrentCtors);

  //
  // Create the new llvm.global_ctors global variable and remove the old one
  // if it existed.
  //
  Value * newGVCtor = new GlobalVariable (module,
                                          NewInit->getType(),
                                          false,
                                          GlobalValue::AppendingLinkage,
                                          NewInit,
                                          "llvm.global_ctors");
  if (GVCtor) {
    newGVCtor->takeName (GVCtor);
    GVCtor->eraseFromParent ();
  }



}


bool InitializeSoftBoundCETSMPX:: runOnModule (Module& module){
  
  constructCheckHandlers(module);
  constructShadowStackHandlers(module);
  constructMetadataHandlers(module); 
  //  constructAuxillaryFunctionHandlers(module);
  return true;
}
