#$Id: Makefile.am,v 1.4 2003-01-08 05:21:08 wilsonp Exp $
AUTOMAKE_OPTIONS = gnu

bin_PROGRAMS = alara

alara_SOURCES = alara.C
noinst_HEADERS = alara.h
LDADD = Chains/libChains.a \
	DataLib/libDataLib.a \
	DataLib/ASCIILib/EAFLib/libEAFLib.a DataLib/ASCIILib/IEAFLib/libIEAFLib.a \
	DataLib/ASCIILib/libASCIILib.a DataLib/ALARALib/libALARALib.a \
	Input/libInput.a Output/libOutput.a Util/libUtil.a Calc/libCalc.a 

SUBDIRS = Calc Chains DataLib Input Output Util
