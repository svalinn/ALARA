#$Id: Makefile.am,v 1.12 2005-04-06 03:05:22 wilsonp Exp $

include $(top_srcdir)/Makefile.h


# fill in final data files here
dist_nonxs_DATA = ANS6_4_3.txt Data_list.txt elelib.std matlib.sample			\
          FetterC_hi.Ci_m3.wdr	FetterC_lo.Ci_m3.wdr NRCA.Ci_m3.wdr NRCC.Ci_m3.wdr	\
          IAEA.clearance.old.Bq_kg NRC.clearance.old.Bq_kg EU.clearance.old.Bq_kg	\
          EU.clearance.2000.Bq_kg IAEA.clearance.2004.Bq_kg                             \
          NRC.clearance.steel.2003.Bq_kg NRC.clearance.concrete.2003.Bq_kg


# create symbolic links to easily remebered filenames
install-data-hook:
	cd $(nonxsdir); \
	rm -f elelib_std; ln -sf elelib.std elelib_std; \
	rm -f ANS6.4.3; ln -sf ANS6_4_3.txt ANS6.4.3; \
	rm -f ANS6_4_3; ln -sf ANS6_4_3.txt ANS6_4_3; \
	rm -f FetterHi; ln -sf FetterC_hi.Ci_m3.wdr FetterHi; \
	rm -f FetterLo; ln -sf FetterC_lo.Ci_m3.wdr FetterLo; \
	rm -f NRCA; ln -sf NRCA.Ci_m3.wdr NRCA; \
	rm -f NRCC; ln -sf NRCC.Ci_m3.wdr NRCC; \
	rm -f IAEA.clearance.Bq_kg; ln -sf IAEA.clearance.2004.Bq_kg IAEA.clearance.Bq_kg; \
	rm -f IAEA.clearance; ln -sf IAEA.clearance.Bq_kg IAEA.clearance; \
	rm -f IAEA_clearance; ln -sf IAEA.clearance.Bq_kg IAEA_clearance; \
	rm -f EU.clearance.Bq_kg; ln -sf EU.clearance.2000.Bq_kg EU.clearance.Bq_kg; \
	rm -f EU.clearance; ln -sf EU.clearance.Bq_kg EU.clearance; \
	rm -f EU_clearance; ln -sf EU.clearance.Bq_kg EU_clearance; \
	rm -f NRC.clearance.steel.Bq_kg; ln -sf NRC.clearance.steel.2003.Bq_kg NRC.clearance.steel.Bq_kg; \
	rm -f NRC.clearance.steel; ln -sf NRC.clearance.steel.Bq_kg NRC.clearance.steel; \
	rm -f NRC_clearance_steel; ln -sf NRC.clearance.steel.Bq_kg NRC_clearance_steel
	rm -f NRC.clearance.concrete.Bq_kg; ln -sf NRC.clearance.concrete.2003.Bq_kg NRC.clearance.concrete.Bq_kg; \
	rm -f NRC.clearance.concrete; ln -sf NRC.clearance.concrete.Bq_kg NRC.clearance.concrete; \
	rm -f NRC_clearance_concrete; ln -sf NRC.clearance.concrete.Bq_kg NRC_clearance_concrete


install-exec-hook:
	mkdirhier $(xsdir)
	cd $(xsdir); \
	touch PlaceCrossSectionDataHere
