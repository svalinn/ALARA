#$Id: sortEachShutdownTime.pl,v 1.1 2002-02-25 16:48:04 wilsonp Exp $
#!/usr/remote/bin/perl


$header = <STDIN>;
$sep = <STDIN>;
chop($sep);

@lines = <STDIN>;

$total = $lines[$#lines];
@isotopes = @lines[0..$#lines-2];

$columns = split(/\s+/,$total)-1;


$header = expand_header($columns,$header);
$total = expand_total($columns,$total);

for ($idx=1;$idx<=$columns;$idx++) {
    @sorted = sort by_column @isotopes;
    add_column(extract_column($idx,@sorted));
}


print "$header\n$sep$sep\n$total\n$sep$sep\n";
print join("\n",@output) . "\n";

sub expand_total {

    my ($cols,$tot) = @_;

    my ($text,@totals) = split(/\s+/,$tot);
    my $new_tot = "$text\t$totals[0]";

    for ($col=1;$col<$cols;$col++) {
	$new_tot .= "\t$text\t$totals[$col]";
    }

    return $new_tot;
}

sub expand_header {

    my ($cols,$head) = @_;

    my ($col, $new_head);
    $new_head = "isotope\tshutdown";

    for ($col=0;$col<$cols-1;$col++) {
	$header =~ /([0-9]+ [smhdwyc])/g;
	$new_head .= sprintf("\tisotope\t%10s",$1);
    }

    return $new_head;
}

sub add_column {

    my @new_column = @_;
    my $linNum;

    #print "$#output, $#new_column\n";

    if ($#output < 0) {
	@output = @new_column;
    } else {
	for ($linNum=0;$linNum<=$#new_column;$linNum++) {
	    $output[$linNum] .= "\t" . $new_column[$linNum];
	}
    }
}
	

sub extract_column {
    
    my $idx = shift(@_);
    my @data = @_;

    my ($linNum,$isotope,@column);

    for ($linNum=0;$linNum<=$#data;$linNum++) {
	($isotope,@columns) = split(/\s+/,$data[$linNum]);
	$column[$linNum] = "$isotope\t" . $columns[$idx-1];
	#print "$linNum/$#data:\t$column[$linNum]\n";
    }

    #print join("\n",@column) . "\n";

    return @column;
}

sub by_column {

    $keyA = (split(/\s+/,$a))[$idx];
    $keyB = (split(/\s+/,$b))[$idx];

    $keyB <=> $keyA;
   
}


sub max {

    my ($a,$b) = @_;

    if ($a > $b) {
	return $a;
    } else {
	return $b;
    }

}
