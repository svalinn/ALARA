#$Id: Makefile.am,v 1.6 2003-10-22 06:07:01 wilsonp Exp $

include $(top_srcdir)/Makefile.h

bin_PROGRAMS = alara

alara_SOURCES = alara.C alara.h
LDADD = Chains/libChains.a \
	DataLib/libDataLib.a \
	DataLib/ASCIILib/EAFLib/libEAFLib.a DataLib/ASCIILib/IEAFLib/libIEAFLib.a \
	DataLib/ASCIILib/libASCIILib.a DataLib/ALARALib/libALARALib.a \
	Input/libInput.a Output/libOutput.a Util/libUtil.a Calc/libCalc.a 

SUBDIRS = Calc Chains DataLib Input Output Util

