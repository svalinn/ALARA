#$Id: Makefile.am,v 1.5 2003-08-19 21:56:54 wilsonp Exp $
bin_PROGRAMS = alara

alara_SOURCES = alara.C alara.h
LDADD = Chains/libChains.a \
	DataLib/libDataLib.a \
	DataLib/ASCIILib/EAFLib/libEAFLib.a DataLib/ASCIILib/IEAFLib/libIEAFLib.a \
	DataLib/ASCIILib/libASCIILib.a DataLib/ALARALib/libALARALib.a \
	Input/libInput.a Output/libOutput.a Util/libUtil.a Calc/libCalc.a 

SUBDIRS = Calc Chains DataLib Input Output Util
