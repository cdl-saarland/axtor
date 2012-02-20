dnl **************************************************************************
dnl * Initialize
dnl **************************************************************************
AC_INIT([Axtor],[1.00],[simon.m.moll@googlemail.com])

dnl Identify where LLVM source tree is
LLVM_SRC_ROOT="/afs/inf.ed.ac.uk/user/v/v1smoll/workspace/llvm-3.0.src/"
LLVM_OBJ_ROOT="/disk/scratch/simon/build"

dnl Find absolute paths to LLVM source and object trees
LLVM_ABS_SRC_ROOT="`cd $srcdir ; cd $LLVM_SRC_ROOT ; pwd`"
LLVM_ABS_OBJ_ROOT="`cd $LLVM_OBJ_ROOT ; pwd`"

dnl Tell autoconf that this is an LLVM project being configured
dnl This provides the --with-llvmsrc and --with-llvmobj options
LLVM_CONFIG_PROJECT($LLVM_ABS_SRC_ROOT,$LLVM_ABS_OBJ_ROOT)

dnl Tell autoconf that the auxiliary files are actually located in
dnl the LLVM autoconf directory, not here.
AC_CONFIG_AUX_DIR($LLVM_SRC/autoconf)

dnl Verify that the source directory is valid
AC_CONFIG_SRCDIR(["Makefile.common.in"])

dnl Configure a common Makefile
AC_CONFIG_FILES(Makefile.common)

dnl Configure project makefiles
dnl List every Makefile that exists within your source tree
AC_CONFIG_MAKEFILE(Makefile)

AC_CONFIG_MAKEFILE(lib/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/ast/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/cns/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/console/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/intrinsics/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/metainfo/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/parsers/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/pass/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/solvers/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/util/Makefile)
AC_CONFIG_MAKEFILE(lib/axtor/writer/Makefile)

AC_CONFIG_MAKEFILE(lib/axtor_ocl/Makefile)

AC_CONFIG_MAKEFILE(tools/Makefile)
AC_CONFIG_MAKEFILE(tools/extractor/Makefile)

dnl **************************************************************************
dnl * Determine which system we are building on
dnl **************************************************************************

dnl **************************************************************************
dnl * Check for programs.
dnl **************************************************************************

dnl **************************************************************************
dnl * Check for libraries.
dnl **************************************************************************

dnl **************************************************************************
dnl * Checks for header files.
dnl **************************************************************************

dnl **************************************************************************
dnl * Checks for typedefs, structures, and compiler characteristics.
dnl **************************************************************************

dnl **************************************************************************
dnl * Checks for library functions.
dnl **************************************************************************

dnl **************************************************************************
dnl * Enable various compile-time options
dnl **************************************************************************

dnl **************************************************************************
dnl * Set the location of various third-party software packages
dnl **************************************************************************

dnl **************************************************************************
dnl * Create the output files
dnl **************************************************************************

dnl This must be last
AC_OUTPUT
