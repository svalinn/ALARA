#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(ALARA,2.7.1,wilsonp@engr.wisc.edu)
AC_LANG(C++)
AC_CONFIG_SRCDIR([src/alara.C])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_F77([g77 f77])
# Fix this when F90 support is figured out
# AC_PROG_F90
AC_PROG_RANLIB
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PATH_PROG(PERLPATH,perl)

AC_F77_LIBRARY_LDFLAGS   ### Check autoconf manual


# Checks for libraries.
# FIXME: Replace `main' with a function in `-lg2c':
# FIXED: PPHW
AC_CHECK_LIB([g2c], [d_sin],[],[],[-lm])
### IS THIS REDUNDANT?
# FIXME: Replace `main' with a function in `-lm':
# FIXED: PPHW
AC_CHECK_LIB([m], [sin])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([pow strchr strcspn strstr])

AC_CONFIG_FILES([Makefile
                 src/Calc/Makefile
                 src/Chains/Makefile
                 src/DataLib/ALARALib/Makefile
                 src/DataLib/ASCIILib/EAFLib/Makefile
                 src/DataLib/ASCIILib/IEAFLib/Makefile
                 src/DataLib/ASCIILib/Makefile
                 src/DataLib/Makefile
                 src/Input/Makefile
                 src/Makefile
                 src/Output/Makefile
                 src/Util/Makefile
                 tools/Makefile
		 tools/extract_pathways
		 tools/summary
		 doc/Makefile
		 data/Makefile
		 sample/Makefile
		 developer-info/Makefile
		 developer-info/doxyalara
		 ])
AC_OUTPUT
