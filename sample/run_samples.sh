#!/bin/sh

export outdir=$1
rm -rf $outdir
mkdir $outdir
echo sample1
alara -v 3 -t $outdir/sample1.tree sample1 > $outdir/sample1.out
echo sample2
alara -v 3 -t $outdir/sample2.tree sample2 > $outdir/sample2.out
echo sample3
alara -v 3 -t $outdir/sample3.tree sample3 > $outdir/sample3.out
echo sample4
alara -v 3 -t $outdir/sample4.tree sample4 > $outdir/sample4.out
echo sample5
sed -e "s/\.\/sample5.photonSrc/$outdir\/sample5.photonSrc/" sample5 > sample5.tmp
alara -v 3 -t $outdir/sample5.tree sample5.tmp > $outdir/sample5.out
rm sample5.tmp
echo sample6
alara -v 3 -t $outdir/sample6.tree sample6 > $outdir/sample6.out
echo sample7
alara -v 3 -t $outdir/sample7.tree sample7 > $outdir/sample7.out
echo sample8
alara -v 3 -t $outdir/sample8.tree sample8 > $outdir/sample8.out
