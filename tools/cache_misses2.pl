#!/usr/bin/perl
# $Id: cache_misses2.pl,v 1.1 2000-02-29 01:28:18 wilson Exp $

# Tool to read ALARA tree file <STDIN> and do post-mortem analysis on
# the number of time that ALARA missed (or would miss) on a reaction
# rate cache of a given size ($ARGV[0]).

$lineNum = 0;
$cacheSize = $ARGV[0];
$misses = 0;

sub max
{
    $a = shift;
    $b = shift;

    if ($a > $b)
    {
	$a;
    }
    else
    {
	$b;
    }
}

sub min
{
    $a = shift;
    $b = shift;

    if ($a > $b)
    {
	$b;
    }
    else
    {
	$a;
    }
}



while (<STDIN>) 
{
    # find isotope string
    (/([a-z][a-z]?-[0-9]{1,3}[mn]?)/) && ($iso[$lineNum] = $1);
    
    $ISO{$iso[$lineNum]}++;

    #print "$iso[$lineNum]\n";
    $cmpLine = 0;
    while ($cmpLine < min(scalar(@cacheData),$cacheSize) &&
	   $cacheData[$cmpLine] ne $iso[$lineNum]) {
	#print "\t$lineNum, $cmpLine: $iso[$lineNum] != $cacheData[$cmpLine]\n";
	$cmpLine++;
    }
    
    if ($cmpLine == min(scalar(@cacheData),$cacheSize)) {
	$misses++;
	$MISSES{$iso[$lineNum]}++;
	push(@cacheData,$iso[$lineNum]);
	if (scalar(@cacheData) > $cacheSize) {
	    shift(@cacheData);
	}
    } else {
	$HITS{$iso[$lineNum]}++;
	#print "\t$lineNum, $cmpLine: $iso[lineNum] == $iso[$cmpLine]\n";
    }
	
    $lineNum++;
}
$unique = keys(%ISO);
@occurrences = sort {$a <=> $b} values(%ISO);
$min = shift(@occurrences);
$max = pop(@occurrences);

@misses = sort {$a <=> $b} values(%MISSES);
$minMisses = shift(@misses);
$maxMisses = pop(@misses);

foreach $iso (sort(keys(%ISO)))
{
    $MISSRATIO{$iso} = $MISSES{$iso}/$ISO{$iso}*100;
    $avgMissRatio += $MISSRATIO{$iso};
}
@missRatios = sort {$a<=> $b} values(%MISSRATIO);
$minRatio = shift(@missRatios);
$maxRatio = pop(@missRatios);
$avgMissRatio /= $unique;

print "cache size: $cacheSize\n";
print "number of isotopes: $lineNum\n\n";
print "number of unique isotopes: $unique\n";
print " statistics: average: " . $lineNum/$unique . ", minimum: $min, maximum: $max\n\n";
print "number of cache misses: $misses\n";
print " statistics: average: " . $misses/$unique . ", minimum: $minMisses, maximum: $maxMisses\n\n";
print "global miss ratio: " . $misses/$lineNum*100 . "%\n";
print " statistics: average: " . $avgMissRatio . ", minimum: $minRatio, maximum: $maxRatio\n\n";

foreach $iso (sort(keys(%ISO)))
{
#    print "\t$iso : $ISO{$iso} -$MISSES{$iso} +$HITS{$iso} (" . 
#	($MISSES{$iso} + $HITS{$iso}) . ")\n";
}


    
