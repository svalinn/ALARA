#$Id: Makefile.am,v 1.10 2004-06-03 20:32:04 wilsonp Exp $

include $(top_srcdir)/Makefile.h


# fill in final data files here
dist_nonxs_DATA = ANS6_4_3.txt Data_list.txt elelib.std FetterC_hi.Ci_m3.wdr	\
          FetterC_lo.Ci_m3.wdr IAEA.clearance.Bq_kg matlib.sample	\
          NRCA.Ci_m3.wdr NRCC.Ci_m3.wdr NRC.clearance.Bq_kg EU.clearance.Bq_kg

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
	rm -f IAEA.clearance; ln -sf IAEA.clearance.Bq_kg IAEA.clearance; \
	rm -f IAEA_clearance; ln -sf IAEA.clearance.Bq_kg IAEA_clearance; \
	rm -f EU.clearance; ln -sf EU.clearance.Bq_kg EU.clearance; \
	rm -f EU_clearance; ln -sf EU.clearance.Bq_kg EU_clearance; \
	rm -f NRC.clearance; ln -sf NRC.clearance.Bq_kg NRC.clearance; \
	rm -f NRC_clearance; ln -sf NRC.clearance.Bq_kg NRC_clearance


install-exec-hook:
	mkdirhier $(xsdir)
	cd $(xsdir); \
	touch PlaceCrossSectionDataHere
