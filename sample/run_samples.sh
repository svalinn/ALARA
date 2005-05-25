#!/bin/sh

outdir=$1

alarabin='../src/alara'

if ! test -x $alarabin; then
    alarabin='alara'
    alarastring=`which $alarabin`
    if test "X$alarastring" = "X"; then
	echo "Could not find executable to run sample cases!
If this is build tree, make sure executable was built successfully.
If this is the installed sample directory, make sure executable is in path."
	exit -1
    else
	echo "Running sample cases with executable from path $alarastring"
    fi
else
    echo "Running sample cases with executable from local build: $alarabin"
fi

$alarabin -V

rm -rf $outdir
mkdir $outdir
mkdir -f dump_files
echo sample1
$alarabin -v 3 -t $outdir/sample1.tree sample1 > $outdir/sample1.out
echo sample2
$alarabin -v 3 -t $outdir/sample2.tree sample2 > $outdir/sample2.out
echo sample3
$alarabin -v 3 -t $outdir/sample3.tree sample3 > $outdir/sample3.out
echo sample4
$alarabin -v 3 -t $outdir/sample4.tree sample4 > $outdir/sample4.out
echo sample5
sed -e "s/\.\/sample5.photonSrc/$outdir\/sample5.photonSrc/" sample5 > sample5.tmp
$alarabin -v 3 -t $outdir/sample5.tree sample5.tmp > $outdir/sample5.out
rm sample5.tmp
echo sample6
$alarabin -v 3 -t $outdir/sample6.tree sample6 > $outdir/sample6.out
echo sample7
$alarabin -v 3 -t $outdir/sample7.tree sample7 > $outdir/sample7.out
echo sample8
$alarabin -v 3 -t $outdir/sample8.tree sample8 > $outdir/sample8.out
