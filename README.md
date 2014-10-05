SoftBoundCETS-v2.0 for LLVM-3.5.0
=================================

This is a README FILE for SoftBoundCETS pointer-based checking. For
more technical details and algorithms, visit SoftBoundCETS website at
http://www.cs.rutgers.edu/~santosh.nagarakatte/softbound/



Using SoftBoundCETS with LLVM+CLANG-3.5.0 on a x86-64 machine with Linux OS
=========================================================================


1. Download the github repository from https://github.com/santoshn/softboundcets-3.5.0.git

2. Build SoftBoundCETS for LLVM+CLANG 3.5.0

   1. Goto to directory softboundcetss-llvm-clang34 by executing the following command

            cd softboundcets-llvm-3.5.0

   2. Configure LLVM, clang and softboundcets with the following command

            ./configure --enable-assertions --disable-optimized

      If you prefer a faster compiler and do not need to debug SoftBoundCETS,
      use `--enable-optimized`.

      If you want to use SoftBoundCETS with LTO, follow the instructions at
      http://llvm.org/docs/GoldPlugin.html and add the
      `--with-binutils-include=/usr/include` parameter. Replace `/usr/include` by
      the folder that contains the `plugin-api.h` file.

   3. Build softboundcets, LLVM, clang with the following command

            make -j8

3. Set up your environment to use SoftBoundCETS

   For example in bash, it would be

         export PATH=<git_repo>/softboundcets-llvm-3.5.0/Debug+Asserts/bin:$PATH

   If you compiled an optimized build, the path is `Release+Asserts` instead of
   `Debug+Asserts`.

4. Compile the SoftBoundCETS runtime library

         cd <git_repo>
         cd softboundcets-lib
         make

   If you compiled the LLVM gold plugin, add the line below before calling
   make, in order to also build the SoftBoundCETS runtime library with LTO
   support.

         export LLVM_GOLD=<git_repo>/softboundcets-llvm-3.5.0/Debug+Asserts/lib/LLVMgold.so

5. Test whether it all worked

   1. Compile

            cd tests
            clang -fsoftboundcets test.c -o test -L<git_repo>/softboundcets-lib -lm -lrt -lsoftboundcets_rt
            clang -fsoftboundcets -flto test.c -o test-lto -L<git_repo>/softboundcets-lib/lto -lm -lrt -lsoftboundcets_rt

   2. Run the test program

            ./test

      Enter 10; the program executes successfully.

      Enter 105; a memory safety violation is triggered.


Some NOTES
==========

(1) If you want to test execution time performance of softboundcets,
use the LTO version. The non-LTO version does not inline calls made by
various check handlers. We recommended testing out the application for
memory safety errors by compiling without any optimization (-O0).

(2) This is a developmental version with active
development. LLVM-3.5.0 optimizations at higher optimization levels
(-O1 and higher) introduce vectorization instructions, which can cause
false violations. The LLVM IR will typically have insertelement,
extractelement and shufflevector with pointers. If you see false
violations, use -fno-vectorize in your flags to avoid memory safety
violations.

(3) Lot of features are currently being added. Use the google groups
to discuss ideas.
