#$Id: Makefile.am,v 1.5 2003-08-19 21:56:55 wilsonp Exp $
noinst_LIBRARIES = libUtil.a

libUtil_a_SOURCES = Matrix.C  Statistics.C  debug.C  input.C  math.C  output.C  stub.C

noinst_HEADERS = Matrix.h  Statistics.h  classes.h  debug.h  functions.h  input_tokens.h dflt_datadir.h

INCLUDE = -I@top_srcdir@/

# Set the installation data directory
DISTCLEANFILES = dflt_datadir.h
dflt_datadir.h: Makefile
	echo '#define DFLT_DATADIR "$(xsdir)"' >! $@

