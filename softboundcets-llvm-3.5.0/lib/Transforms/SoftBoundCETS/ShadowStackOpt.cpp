//=== SoftBoundCETS/ShadowStackOpt.cpp --*- C++ -*=====///
// Shadow Stack Optimizations for SoftBoundCETS and related passes
// Copyright (c) 2014 Santosh Nagarakatte, Milo M. K. Martin. All rights reserved.
//
// Developed by: Santosh Nagarakatte, 
//               Department of Computer Science,
//               Rutgers University
//               http://www.cs.rutgers.edu/~santosh.nagarakatte/softbound/
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

//   3. Neither the names of Santosh Nagarakatte,
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

#include "llvm/Transforms/SoftBoundCETS/ShadowStackOpt.h"


char ShadowStackOpt::ID = 0;

static RegisterPass<ShadowStackOpt> P ("ShadowStackOpt", 
					"Shadow Stack Optimizations");

bool ShadowStackOpt::checkFunctionOfInterest(Function* func){

  
  if(func->isDeclaration())
    return false;

  if(func->isVarArg())
    return false;
  
  bool function_direct = true;
  bool function_ptr_argsret = false;

  for(User * NU: func->users()){
    CallInst * call_inst = dyn_cast<CallInst>(NU);
    if(!call_inst){   
      function_direct = false;
      break;
    }    
  }

  Type* Rty = func->getReturnType();

  if(isa<PointerType>(Rty))
    function_ptr_argsret = true;

  for(Function::arg_iterator i = func->arg_begin(), e = func->arg_end();
      i != e; ++i) {
    if(isa<PointerType>(i->getType())) {
      function_ptr_argsret = true;
      break;
    }
  }
  return ((function_direct) && (function_ptr_argsret));
}

//
// Method: getStructureMetadataType()
//
// Description:
//
// This function creates a 5-element structure type for the alloca
// with type of the pointer being input as the parameter
//
//

StructType* ShadowStackOpt::getStructMetadataType(Type* ptr_type) {

  StructType* struct_ret_type;
  struct_ret_type = StructType::get(ptr_type,
				    m_void_ptr_type,
				    m_void_ptr_type,
				    m_sizet_type,
				    m_void_ptr_type, NULL);
  return struct_ret_type;
}


Instruction* 
ShadowStackOpt::getShadowStackLoadHandler(Instruction *call, 
					  StringRef ss_string, 
					  int index){

  Instruction* returnval = nullptr;
  Instruction *next = call;

  while(true){
    next = getPrevInstruction(next);

    if(next == call->getParent()->getTerminator())
      break;

    if(CallInst* shadow = dyn_cast<CallInst>(next)){
      Function* func = shadow->getCalledFunction();

      if((func->getName().find(ss_string) == 0) ||
	 (func->getName().find(ss_string) == 0)){

	if(ConstantInt * constant = dyn_cast<ConstantInt>(shadow->getOperand(0))){
	  if(constant->getUniqueInteger() == index){
	    returnval = shadow;
	    break;
	  }
	}
	else{
	  assert(0 && "invalid shadow stack operation found");
	}
      }
    }
  }//while loop
  assert((returnval != nullptr) && "no shadow stack function to replace?");

  return returnval;
}


Value* 
ShadowStackOpt::getShadowStackArgHandler(Instruction *call, 
					 StringRef ss_string, 
					 int index){

  Value* returnval = nullptr;
  Instruction *next = call;
  bool check = false;

  if(index == -1){
    check = true;
  }
  
  while(true){
    next = getPrevInstruction(next);

    if(next == call->getParent()->getTerminator())
      break;

    if(CallInst* shadow = dyn_cast<CallInst>(next)){
      Function* func = shadow->getCalledFunction();

      if((func->getName().find(ss_string) == 0) ||
	 (func->getName().find(ss_string) == 0)){

	if(check == true){
	  next->eraseFromParent();
	  break;
	}

	if(ConstantInt * constant = dyn_cast<ConstantInt>(shadow->getOperand(1))){
	  if(constant->getUniqueInteger() == index){
	    returnval = shadow->getOperand(0);
	    next->eraseFromParent();
	    break;
	  }
	}
	else{
	  assert(0 && "invalid shadow stack operation found");
	}
      }
    }
  }//while loop
  assert(((check == true) || (returnval != nullptr)) && "no shadow stack function to replace?");

  return returnval;
}

CallInst* ShadowStackOpt::getShadowStackRetHandler(CallInst *call, StringRef ss_string){

  CallInst* returnval = nullptr;
  Instruction* next = call;
  while(true){
    next = getNextInstruction(next);

    if(next == call->getParent()->getTerminator())
      break;

    if(CallInst* shadow = dyn_cast<CallInst>(next)){
      Function* func = shadow->getCalledFunction();

      if((func->getName().find(ss_string) == 0) ||
	 (func->getName().find(ss_string) == 0)){
	returnval = shadow;
	break;
      }
    }
  }//while loop
  assert((returnval != nullptr) && "no shadow stack function to replace?");
  return returnval;


}


void ShadowStackOpt:: ReplaceShadowStackLoads(Value* Cbase, Value* Cbound, 
					      Value* Ckey, Value* Clock,
					      CallInst* call){

  CallInst* shadow_base = getShadowStackRetHandler(call, "__softboundcets_load_base_shadow_stack");
  CallInst* shadow_bound = getShadowStackRetHandler(call,"__softboundcets_load_bound_shadow_stack");
  CallInst* shadow_key = getShadowStackRetHandler(call, "__softboundcets_load_key_shadow_stack");
  CallInst* shadow_lock = getShadowStackRetHandler(call, "__softboundcets_load_lock_shadow_stack");

  shadow_base->replaceAllUsesWith(Cbase);
  shadow_bound->replaceAllUsesWith(Cbound);
  shadow_key->replaceAllUsesWith(Ckey);
  shadow_lock->replaceAllUsesWith(Clock);

  CallInst* shadow_deallocate = getShadowStackRetHandler(call, "__softboundcets_deallocate_shadow_stack_space");
  
  shadow_deallocate->eraseFromParent();
  shadow_base->eraseFromParent();
  shadow_bound->eraseFromParent();
  shadow_key->eraseFromParent();
  shadow_lock->eraseFromParent();
  
}


void ShadowStackOpt::handleCallUseOfFunction(CallInst *CI, 
					     Function *NF) {

  CallSite CS(CI);
  Instruction* call = CI;
  bool ret_type_is_pointer = false;
  Value* call_replacement = NULL;
  Instruction* call_next = NULL;
  SmallVector<Value*, 32> call_args;

  const AttributeSet& call_pal = CS.getAttributes();
  SmallVector<AttributeSet, 8> param_attrs_vec;

  if(isa<PointerType>(CI->getType())) {
    ret_type_is_pointer = true;
  }

  if(ret_type_is_pointer){    
    StructType* struct_ret_type = getStructMetadataType(CI->getType());
    Instruction* FirstI = dyn_cast<Instruction>(CI->getParent()->getParent()->begin()->begin());
    assert(FirstI && "External Function?");
    AllocaInst* ret_ai = new AllocaInst(struct_ret_type, "struct_ret", FirstI);
    call_args.push_back(ret_ai);
    call_next = getNextInstruction(call);

    Value* idxs[2] = {ConstantInt::get(Type::getInt32Ty(call->getType()->getContext()), 0), nullptr};
    idxs[1] = ConstantInt::get(Type::getInt32Ty(call->getType()->getContext()), 0);
    Value* actual_ai_value = ret_ai;

    Value* gep_inst_ptr = GetElementPtrInst::Create(actual_ai_value, idxs,
						    call->getName(),
						    call_next);
    call_replacement = new LoadInst(gep_inst_ptr, "call_repl", call_next);
    call->replaceAllUsesWith(call_replacement);
    call_replacement->takeName(call);

    /* Loading the base of the returned pointer */
    idxs[1] = ConstantInt::get(Type::getInt32Ty(call->getContext()), 1);

    Value* gep_inst_base = GetElementPtrInst::Create(actual_ai_value, idxs,
						     call->getName() + "_base",
						     call_next);
    Value* call_base = new LoadInst(gep_inst_base, "call_repl_base", call_next);

    /* loading the bound  of the returned pointer*/

    idxs[1] = ConstantInt::get(Type::getInt32Ty(call->getContext()), 2);
    Value* gep_inst_bound = GetElementPtrInst::Create(actual_ai_value, idxs,
						      call->getName() + "_bound",
						      call_next);
    Value* call_bound = new LoadInst(gep_inst_bound, "call_repl_bound", call_next);

    idxs[1] = ConstantInt::get(Type::getInt32Ty(call->getContext()), 3);
    Value* gep_inst_key = GetElementPtrInst::Create(actual_ai_value, idxs,
						      call->getName() + "_key",
						      call_next);
    Value* call_key = new LoadInst(gep_inst_key, "call_repl_key", call_next);


    idxs[1] = ConstantInt::get(Type::getInt32Ty(call->getContext()), 4);
    Value* gep_inst_lock = GetElementPtrInst::Create(actual_ai_value, idxs,
						      call->getName() + "_lock",
						      call_next);
    Value* call_lock = new LoadInst(gep_inst_lock, "call_repl_lock", call_next);

    ReplaceShadowStackLoads(call_base, call_bound, call_key, call_lock, CI);
  }
  else {

    //return type is not a pointer, maintain the attributes.
    if(call_pal.hasAttributes(AttributeSet::ReturnIndex))
      param_attrs_vec.push_back(AttributeSet::get(call->getContext(), call_pal.getRetAttributes()));

  }

  int arg_index;
  CallSite::arg_iterator arg_i = CS.arg_begin();
  arg_index = 1;

  std::vector<Value*> extra_actual_parameters;

  for(unsigned i = 0; i < CS.arg_size(); i++, ++arg_index, ++arg_i) {
    call_args.push_back(*arg_i);
    Value* arg_value_considered = CS.getArgument(arg_index - 1);
    assert(arg_value_considered && "arg val null?");

    if(isa<PointerType>(arg_value_considered->getType())) {

      Value* cs_base = getShadowStackArgHandler(CI, "__softboundcets_store_base_shadow_stack", arg_index);
      extra_actual_parameters.push_back(cs_base);
      Value* cs_bound = getShadowStackArgHandler(CI, "__softboundcets_store_bound_shadow_stack", arg_index);
      extra_actual_parameters.push_back(cs_bound);
      Value* cs_key = getShadowStackArgHandler(CI, "__softboundcets_store_key_shadow_stack", arg_index);
      extra_actual_parameters.push_back(cs_key);

      Value* cs_lock = getShadowStackArgHandler(CI, "__softboundcets_store_lock_shadow_stack", arg_index);
      extra_actual_parameters.push_back(cs_lock);
      

    }/* if pointer argument ends */
    else{

      AttributeSet attrs = call_pal.getParamAttributes(arg_index);
      if(attrs.hasAttributes(arg_index)){
	AttrBuilder B(attrs, arg_index);
	param_attrs_vec.push_back(AttributeSet::get(call->getContext(), call_args.size(), B));
      }
    }
  } /* iterating over original argument ends */

  getShadowStackArgHandler(CI, "__softboundcets_allocate_shadow_stack_space", -1);



  // Add the extra arguments to the the CallSite 
  std::vector<Value*>::iterator actual_it = extra_actual_parameters.begin();

  while(actual_it != extra_actual_parameters.end()) {
    call_args.push_back(*actual_it);
    ++actual_it;
  }

  // Create the new call instruction
  Instruction* new_inst;
  if(InvokeInst* invoke_call = dyn_cast<InvokeInst>(call)) {
    invoke_call->dump();
    assert(0 && "Invoke not handled");
  }

  new_inst = CallInst::Create(NF, call_args, "", call);
  cast<CallInst>(new_inst)->setCallingConv(CI->getCallingConv());
  cast<CallInst>(new_inst)->setAttributes(AttributeSet::get(call->getContext(), param_attrs_vec));  
  call->eraseFromParent();    
}

void ShadowStackOpt::ProcessReturnInst(ReturnInst* ret_inst, Function* NF){

  Function::arg_iterator ret_arg = NF->arg_begin();
  Value* ret_argument = dyn_cast<Value>(ret_arg);


  Value* pointer = ret_inst->getReturnValue();
  
  Value* idxs[2] = {ConstantInt::get(Type::getInt32Ty(ret_inst->getContext()), 0), 0};
  
  idxs[1] = ConstantInt::get(Type::getInt32Ty(ret_inst->getContext()), 0);

  Value* gep_ret_ptr = GetElementPtrInst::Create(ret_argument, idxs,
						 ret_arg->getName() + ".ptr",
						 ret_inst);
  
  new StoreInst(pointer, gep_ret_ptr, ret_inst);


  Value* base, *bound, *key, *lock;

  base = getShadowStackArgHandler(ret_inst, "__softboundcets_store_base_shadow_stack", 0);  
  bound = getShadowStackArgHandler(ret_inst, "__softboundcets_store_bound_shadow_stack", 0);  
  key = getShadowStackArgHandler(ret_inst, "__softboundcets_store_key_shadow_stack", 0);
  lock = getShadowStackArgHandler(ret_inst, "__softboundcets_store_lock_shadow_stack", 0);
  
  idxs[1] = ConstantInt::get(Type::getInt32Ty(ret_inst->getContext()), 1);
  
  Value* gep_ret_base = GetElementPtrInst::Create(ret_argument, idxs,
						  ret_arg->getName() + ".base",
						  ret_inst);
  
  new StoreInst(base, gep_ret_base, ret_inst);


  idxs[1] = ConstantInt::get(Type::getInt32Ty(ret_inst->getContext()), 2);
  
  Value* gep_ret_bound = GetElementPtrInst::Create(ret_argument, idxs,
						   ret_arg->getName() + ".bound",
						   ret_inst);

  new StoreInst(bound, gep_ret_bound, ret_inst);

  idxs[1] = ConstantInt::get(Type::getInt32Ty(ret_inst->getContext()), 3);
  
  Value* gep_ret_key = GetElementPtrInst::Create(ret_argument, idxs,
						 ret_arg->getName() + ".key",
						 ret_inst);

  new StoreInst(key, gep_ret_key, ret_inst);


  idxs[1] = ConstantInt::get(Type::getInt32Ty(ret_inst->getContext()), 4);
  
  Value* gep_ret_lock = GetElementPtrInst::Create(ret_argument, idxs,
						  ret_arg->getName() + ".lock",
						  ret_inst);

  new StoreInst(lock, gep_ret_lock, ret_inst);
  
  BasicBlock* BB = ret_inst->getParent();
  ReturnInst::Create(ret_inst->getContext(), BB);
  ret_inst->eraseFromParent();


}

void ShadowStackOpt::ProcessArgsandRemoveShadowCalls(Function *F, Function* NF){


  Function::arg_iterator arg_i2 = NF->arg_begin();

  if(isa<PointerType>(F->getReturnType())) {
    arg_i2->setName("shadow_ret");
    arg_i2++;
  }

  std::vector<StringRef> extra_arguments;


  int ptr_arg_count=0;
  unsigned int org_num_arguments = 0;

  for (Function::arg_iterator arg_i = F->arg_begin(), arg_e = F->arg_end();
       arg_i != arg_e; ++arg_i, ++org_num_arguments){

    Argument* ptr_arg = dyn_cast<Argument>(arg_i);
    Value* ptr_arg_value = ptr_arg;

    if(isa<PointerType>(ptr_arg_value->getType())){
      extra_arguments.push_back(arg_i->getName());
    }

    arg_i->replaceAllUsesWith(arg_i2);
    arg_i2->takeName(arg_i);


    ++arg_i2;
  }

  std::vector<StringRef>:: iterator args_it = extra_arguments.begin();

  Instruction* terminator= NF->begin()->getTerminator();
  Instruction* start = getPrevInstruction(terminator);
  int arg_count = 0;
  while(args_it != extra_arguments.end()){
    
    ++arg_count;
    StringRef ptr_arg = *args_it;

    
    
    ++args_it;

    Instruction* ss_inst;

    arg_i2->setName(ptr_arg + "_base");
    ss_inst = getShadowStackLoadHandler(start, "__softboundcets_load_base_shadow_stack", arg_count);
    ss_inst->replaceAllUsesWith(arg_i2);
    ss_inst->eraseFromParent();
    
    ++arg_i2;

    arg_i2->setName(ptr_arg + "_bound");
    ss_inst = getShadowStackLoadHandler(start, "__softboundcets_load_bound_shadow_stack", arg_count);
    ss_inst->replaceAllUsesWith(arg_i2);
    ss_inst->eraseFromParent();

    ++arg_i2;    
   
    arg_i2->setName(ptr_arg + "_key");
    ss_inst = getShadowStackLoadHandler(start, "__softboundcets_load_key_shadow_stack", arg_count);
    ss_inst->replaceAllUsesWith(arg_i2);
    ss_inst->eraseFromParent();

    ++arg_i2;
    
    arg_i2->setName(ptr_arg + "_lock");
    ss_inst = getShadowStackLoadHandler(start, "__softboundcets_load_lock_shadow_stack", arg_count);
    ss_inst->replaceAllUsesWith(arg_i2);
    ss_inst->eraseFromParent();
    ++arg_i2;   
  }
}

void ShadowStackOpt::transformIndividualFunction(Function* F){

  Module* M = F->getParent();
  PointerType* ptr_ty = dyn_cast<PointerType>(m_void_ptr_type);
  Type* new_ret_type = NULL;
  Type* ptr_struct_type = NULL;
  Type* old_ret_type = F->getReturnType();
 
  if (!isa<PointerType>(F->getReturnType())){
    new_ret_type = F->getReturnType();
  }
  else{

    /* Create a new return type for functions returnign pointers */
    new_ret_type = Type::getVoidTy(M->getContext());    
    StructType* struct_ret_type = getStructMetadataType(old_ret_type);
    ptr_struct_type = PointerType::get(struct_ret_type, ptr_ty->getAddressSpace());

  }

  const FunctionType* fty = F->getFunctionType();
  std::vector<Type*> params;

  SmallVector<AttributeSet, 8> param_attrs_vec;
  const AttributeSet& pal = F->getAttributes();

  //
  // Get the attributes of the return value
  //

  if(isa<PointerType>(F->getReturnType())){
    params.push_back(ptr_struct_type);
  }
  else {
    if(pal.hasAttributes(AttributeSet::ReturnIndex))
      param_attrs_vec.push_back(AttributeSet::get(F->getContext(), pal.getRetAttributes()));
  }

  // Get the attributes of the arguments 
  int arg_index = 1;
  int count = 0;
  for(Function::arg_iterator i = F->arg_begin(), 
        e = F->arg_end();
      i != e; ++i, arg_index++) {

    if(isa<PointerType>(i->getType())){
      count++;
    }
    params.push_back(i->getType());

    AttributeSet attrs = pal.getParamAttributes(arg_index);

    if(attrs.hasAttributes(arg_index)){
      AttrBuilder B(attrs, arg_index);
      param_attrs_vec.push_back(AttributeSet::get(F->getContext(), params.size(), B));
    }
  }

  for(int i = 0; i < count; i++){

    /* push the metadata for each pointer argument at the end of the function */
    params.push_back(m_void_ptr_type);
    params.push_back(m_void_ptr_type);
    params.push_back(m_sizet_type);
    params.push_back(m_void_ptr_type);
  }

  FunctionType* nfty = FunctionType::get(new_ret_type, params, false);
  Function* new_func = Function::Create(nfty, F->getLinkage(), F->getName()+ ".mod");
  

  // set the new function attributes
  new_func->copyAttributesFrom(F);
  new_func->setAttributes(AttributeSet::get(F->getContext(), param_attrs_vec));

  /* Add the new function into Module's function list */
  F->getParent()->getFunctionList().insert(F, new_func);

  for (User *U: F->users()){
    CallInst* CI = dyn_cast<CallInst>(U);

    if(!CI){
      assert (0 && "Non-call uses of Function and being transformed?");
    }
    handleCallUseOfFunction(CI, new_func);   
  }

  if(isa<PointerType>(F->getReturnType())){    
    SmallVector<ReturnInst*, 8> RetVec;
    for(auto &BB: *F){
      for(auto &Inst: BB){
	if(isa<ReturnInst>(Inst)){
	  ReturnInst* RI = dyn_cast<ReturnInst>(&Inst);
	  RetVec.push_back(RI);
	}
      }
    }
    while(!RetVec.empty()){
      ProcessReturnInst(RetVec.pop_back_val(), new_func); 
    }    
  }

  new_func->getBasicBlockList().splice(new_func->begin(), F->getBasicBlockList());
  ProcessArgsandRemoveShadowCalls(F, new_func);
  F->eraseFromParent();

#if 0
  func->setLinkage(GlobalValue::ExternalLinkage);
#endif


}


void ShadowStackOpt::transformFunctions(){

  for(std::vector<Function*>::iterator i = FunctionsToTransform.begin(), 
	e = FunctionsToTransform.end(); i!= e ; ++i){
    
    Function* func = *i;
    transformIndividualFunction(func);
  }

}



bool ShadowStackOpt::runOnModule(Module &M){

  m_void_ptr_type = PointerType::getUnqual(Type::getInt8Ty(M.getContext()));
  m_sizet_type = Type::getInt64Ty(M.getContext());

  for (Module::iterator ff_begin = M.begin(), ff_end = M.end();
       ff_begin != ff_end; ++ff_begin){

    Function* func_ptr = dyn_cast<Function>(ff_begin);
    if(func_ptr && checkFunctionOfInterest(func_ptr)){
      FunctionsToTransform.push_back(func_ptr);
    }
  }
  transformFunctions();
  return true;
}
