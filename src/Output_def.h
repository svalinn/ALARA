/* $Id: Output_def.h,v 1.17 2008-07-31 18:08:50 phruksar Exp $ */
#ifndef OUTPUT_DEF_H
#define OUTPUT_DEF_H

#define OUTFMT_HEAD     0
#define OUTFMT_UNITS    1
#define OUTFMT_COMP     2  
#define OUTFMT_NUM      4
#define OUTFMT_ACT      8
#define OUTFMT_HEAT    16
#define OUTFMT_ALPHA   32
#define OUTFMT_BETA    64
#define OUTFMT_GAMMA  128
#define OUTFMT_SRC    256
#define OUTFMT_CDOSE  512
#define OUTFMT_ADJ   1024
#define OUTFMT_EXP   2048
#define OUTFMT_EXP_CYL_VOL 4096
#define OUTFMT_WDR   8192
#define OUTFMT_INT_ENG 16384

#define OUTNORM_KG        -2
#define OUTNORM_G         -1
#define OUTNORM_NULL       0
#define OUTNORM_CM3        1
#define OUTNORM_M3         2
#define OUTNORM_VOL_INT  100

#define BQ_CI     2.7027e-11

#define CM3_M3           1e-6
#define G_KG             1e-3

#endif
