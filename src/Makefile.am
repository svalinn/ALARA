$Id: Makefile.am,v 1.1 2003-01-03 02:02:14 wilsonp Exp $
AUTOMAKE_OPTIONS = gnu

bin_PROGRAMS = alara

alara_SOURCES = alara.C
noinst_HEADERS = alara.h
LDADD = Calc/libCalc.a Chains/libChains.a \
	DataLib/libDataLib.a \
	DataLib/ASCIILib/EAFLib/libEAFLib.a DataLib/ASCIILib/IEAFLib/libIEAFLib.a \
	DataLib/ASCIILib/libASCIILib.a DataLib/ALARALib/libALARALib.a \
	Input/libInput.a Output/libOutput.a Util/libUtil.a

SUBDIRS = Calc Chains DataLib Input Output Util
