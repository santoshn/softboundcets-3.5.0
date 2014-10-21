//=== tools/SoftBoundCETS/main.cpp - Driver for SoftBound/CETS --*- C++ -*===// 
// Copyright (c) 2011 Santosh Nagarakatte, Milo M. K. Martin. All rights reserved.

// Developed by: Santosh Nagarakatte, Milo M.K. Martin,
//               Jianzhou Zhao, Steve Zdancewic
//               Department of Computer and Information Sciences,
//               University of Pennsylvania
//               http://www.cis.upenn.edu/acg/softbound/

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
//      Jianzhou Zhao, Steve Zdancewic, University of Pennsylvania, nor
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

#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/RegionPass.h"
#include "llvm/Bitcode/BitcodeWriterPass.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassNameParser.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/InitializePasses.h"
#include "llvm/LinkAllIR.h"
#include "llvm/LinkAllPasses.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/PassManager.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/SystemUtils.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>
#include <memory>

using namespace llvm;

#include "llvm/Transforms/SoftBoundCETS/InitializeSoftBoundCETS.h"
#include "llvm/Transforms/SoftBoundCETS/SoftBoundCETSPass.h"

#include "llvm/Transforms/SoftBoundCETS/InitializeSoftBoundMPX.h"
#include "llvm/Transforms/SoftBoundCETS/SoftBoundMPXPass.h"

#include "llvm/Transforms/SoftBoundCETS/InitializeSoftBoundCETSMPX.h"
#include "llvm/Transforms/SoftBoundCETS/SoftBoundCETSMPXPass.h"

#include "llvm/Transforms/SoftBoundCETS/FixByValAttributes.h"
//#include "SoftBound/InstCountPass.h"

#include "llvm/Transforms/SoftBoundCETS/SpatialCheck.h"
#include "llvm/Transforms/SoftBoundCETS/ShadowStackOpt.h"

#include <memory>
#include <algorithm>

#include <fstream>
#include <iostream>
#include <memory>

using namespace llvm;

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input bitcode>"), cl::init("-"));

static cl::opt<bool>
shadowstackopt ("shadowstackopt",
	      cl::init(false),
	      cl::desc("Perfom Shadow Stack Optimization"));


static cl::opt<bool>
softboundmpx ("softboundmpx",
	      cl::init(false),
	      cl::desc("Perfom SoftBound Instrumentation in MPX mode"));

static cl::opt<bool>
softboundcetsmpx ("softboundcetsmpx",
		  cl::init(false),
		  cl::desc("Perfom SoftBoundCETS Instrumentation in MPX mode"));


static cl::opt<bool>
XMMMode ("xmm_mode",
         cl::init(false),
         cl::desc("Perfom XMM mode metadata propagation"));

static cl::opt<bool>
YMMMode ("ymm_mode",
         cl::init(false),
         cl::desc("Perfom YMM mode metadata propagation"));

static cl::opt<bool>
strip_intrinsic_mode ("strip_intrinsic_mode",
		cl::init(false),
		cl::desc("Strip SBCETS intrinsics mode"));

static cl::opt<bool>
fix_byval_attributes ("fix_byval_attributes",
                      cl::init(false),
                      cl::desc("Fix byval attributes with pointers"));

static cl::opt<bool>
llvm_stat_counter ("llvm_stat_counter",
		   cl::init(false),
		   cl::desc("Perform LLVM Stat Counter Instrumentation"));



int
main (int argc, char ** argv)
{
  llvm_shutdown_obj Y;
  LLVMContext &Context = getGlobalContext();

  std::string OutputFilename;

  cl::ParseCommandLineOptions(argc, argv, "SoftBound Pass for Spatial Safety\n");
  sys::PrintStackTraceOnErrorSignal();
  
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  
  initializeCore(Registry);
  initializeScalarOpts(Registry);
  initializeIPO(Registry);
  initializeAnalysis(Registry);
  initializeIPA(Registry);
  initializeTransformUtils(Registry);
  initializeInstCombine(Registry);
  initializeInstrumentation(Registry);
  initializeTarget(Registry);


  PassManager Passes;

  SMDiagnostic Err;    
  std::unique_ptr<Module> M1;

  M1.reset(ParseIRFile(InputFilename, Err, Context));
  if(M1.get() == 0){
    Err.print(argv[0], errs());
    return 1;
  }

  std::unique_ptr<tool_output_file> Out;
  std::string ErrorInfo;
  
  OutputFilename = InputFilename;

  if(fix_byval_attributes){
    OutputFilename += ".byvalfix.bc";
  }
  else if(llvm_stat_counter){
    OutputFilename += ".stat.bc";
  }
  else{
    OutputFilename += ".sbpass.bc";
  }

  Out.reset(new tool_output_file(OutputFilename.c_str(), ErrorInfo, sys::fs::F_None));

  if(!ErrorInfo.empty()){
    errs()<< ErrorInfo<<'\n';
    return 1;
  }
  bool normal_mode = true;

  if(XMMMode || YMMMode|| strip_intrinsic_mode 
     || fix_byval_attributes || llvm_stat_counter || softboundmpx || softboundcetsmpx || shadowstackopt){
    normal_mode = false;
  }



  if (normal_mode){
    
    const DataLayout * DL = M1.get()->getDataLayout();
    if (DL)
      Passes.add(new DataLayoutPass(M1.get()));


    TargetLibraryInfo *TLI = new TargetLibraryInfo(Triple(M1.get()->getTargetTriple()));
    Passes.add(TLI);
    
    Passes.add(new DominatorTreeWrapperPass());
    Passes.add(new InitializeSoftBoundCETS());
    Passes.add(new SoftBoundCETSPass());
    Passes.add(new SpatialCheckOpt());
    //    Passes.add(new ShadowStackOpt());
  }

  if(shadowstackopt){

    Passes.add(new ShadowStackOpt());
  }

  if (softboundmpx){
    const DataLayout * DL = M1.get()->getDataLayout();
    if (DL)
      Passes.add(new DataLayoutPass(M1.get()));

    
    Passes.add(new InitializeSoftBoundMPX());
    Passes.add(new SoftBoundMPXPass());
    
  }

  if (softboundcetsmpx){
    const DataLayout * DL = M1.get()->getDataLayout();
    if (DL)
      Passes.add(new DataLayoutPass(M1.get()));

    Passes.add(new InitializeSoftBoundCETSMPX());
    Passes.add(new SoftBoundCETSMPXPass());
  }
  
#if 0
  if(XMMMode){
    Passes.add(new InitializeSoftBoundXMM());
    Passes.add(new SoftBoundCETSXMMPass());

  }
  if(YMMMode){
    Passes.add(new InitializeSoftBoundYMM());
    Passes.add(new SoftBoundCETSYMMPass());
  }

  if(strip_intrinsic_mode){
    Passes.add(new StripSBCETSIntrinsics());
  }

#endif

  if(fix_byval_attributes){
    Passes.add(new FixByValAttributesPass());
  }

#if 0
  if(llvm_stat_counter){
    Passes.add(new InstCountPass());
  }
#endif
  
  Passes.add(createBitcodeWriterPass(Out->os()));
  Passes.run(*M1.get());

  Out->keep();

  return 0;
}

