#!/usr/bin/perl
# $Id: average_stride.pl,v 1.1 2000-02-29 01:28:18 wilson Exp $

# Tool to read ALARA tree file <STDIN> and do post-mortem analysis on
# the number of time that ALARA missed (or would miss) on a reaction
# rate cache of a given size ($ARGV[0]).

use IO::Handle;

STDERR->autoflush(1);

$verbose = ($ARGV[0] eq "-v"?1:0);

$lineNum = 0;
$misses = 0;

$min_stride = 100000;
$max_stride = 0;
$min_ustride = 100000;
$max_ustride = 0;

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

    $stride = 1;
    undef %INTER;
    while ($stride < $lineNum &&
	   $iso[$lineNum-$stride] ne $iso[$lineNum]) {
	$INTER{$iso[$lineNum-$stride]}++;
	$stride++;
    }

    $ustride = keys(%INTER)+1;
    
    if ($verbose) {
	print "$iso[$lineNum] $iso[$lineNum-$stride] " . ($lineNum-1);
    }
    if ($stride < $lineNum)
    {
	$stride;
	$ustride;

	$avg_stride += $stride;
	$max_stride = max($max_stride,$stride);
	$min_stride = min($min_stride,$stride);
	$lg2 = int(log($stride)/log(2))+1;
	$dist[$lg2]++;

	$avg_ustride += $ustride;
	$max_ustride = max($max_ustride,$ustride);
	$min_ustride = min($min_ustride,$ustride);
	$ulg2 = int(log($ustride)/log(2))+1;
	$udist[$ulg2]++;

	$repeatNum++;

	if ($verbose) {
	    print  " *stride: $stride $lg2 *stats: " . $avg_stride/$repeatNum 
		. " $min_stride $max_stride *ustride: $ustride $ulg2 *stats: " . $avg_ustride/$repeatNum 
		    . " $min_ustride $max_ustride\n";
	}
    } else {
	if ($verbose) {
	    print " **NEW**\n";
	}
    }

    $lineNum++;
    if ($lineNum%5000 == 0) {
	print STDERR " \# " . $lineNum/1000;
	if ($verbose) {
	    print STDERR "\n";
	}
    }
}

$unique = keys(%ISO);
@occurrences = sort {$a <=> $b} values(%ISO);
$min = shift(@occurrences);
$max = pop(@occurrences);

print STDERR "number of isotopes: $lineNum\n\n";
print STDERR "number of unique isotopes: $unique\n";
print STDERR "number of cache hopes: $repeatNum\n";
print STDERR " occurrence stats: average: " . $lineNum/$unique . 
    ", minimum: $min, maximum: $max\n\n";
print STDERR "stride stats: average: " . $avg_stride/$repeatNum . 
    ", minimum: $min_stride, maximum: $max_stride\n\n";
print STDERR "unique stride stats: average: " . $avg_ustride/$repeatNum . 
    ", minimum: $min_ustride, maximum: $max_ustride\n\n";

$cumm = 0;
$ucumm = 0;
$x = 1;
for ($xp=0;$xp<max(scalar(@dist),scalar(@udist));$xp++)
{
    $cumm += $dist[$xp];
    $ucumm += $udist[$xp];
    print STDERR "$x $dist[$xp] $cumm " . $cumm/$repeatNum . 
	" $udist[$xp] $ucumm " .  $ucumm/$repeatNum . "\n";
    $x *= 2;
}

