#$Id: Makefile.am,v 1.3 2003-01-06 20:58:59 wilsonp Exp $
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
