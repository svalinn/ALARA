#$Id: Makefile.am,v 1.7 2006-01-17 22:55:33 phruksar Exp $

include $(top_srcdir)/Makefile.h

bin_PROGRAMS = alara

alara_SOURCES = alara.C alara.h
LDADD = Chains/libChains.a \
	DataLib/libDataLib.a \
	DataLib/ASCIILib/EAFLib/libEAFLib.a DataLib/ASCIILib/IEAFLib/libIEAFLib.a \
	DataLib/ASCIILib/libASCIILib.a DataLib/ALARALib/libALARALib.a DataLib/FEINDLib/libFEINDLib.a DataLib/FEINDLib/FEIND/FEIND.a \
	Input/libInput.a Output/libOutput.a Util/libUtil.a Calc/libCalc.a 

SUBDIRS =  Calc Chains DataLib Input Output Util 

