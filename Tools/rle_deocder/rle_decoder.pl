#!/usr/bin/perl -w 
#**********************************************************************************
# This short Perl script is meant to decode RLE(Run Length Encoding) files. 
# This may prove useful if you are looking to test your code on unique GameOfLife 
# patterns found on the web, e.g.: 
# 	https://copy.sh/life/examples/ 
#
# Original code from: 
# 	https://github.com/jaywritescode/GameOfLife-perl/blob/master/gameoflife.pl
#**********************************************************************************
use Cwd 'abs_path';
{
    my $filename = $ARGV[0]
      or die "Usage: perl rle_decoder.pl <filename.rle>\n\n";
    my @grid = load($filename);
    print_grid_to_file($filename, @grid);
}
#/*--------------------------------------------------------------------------------
#                                    Auxiliary Subroutines
#--------------------------------------------------------------------------------*/
sub print_grid_to_file {
    my ($filename,@grid) = @_;
    my $filename_noex = $filename; 
	$filename_noex =~ s{\.[^.]+$}{}; 

    my ($basename) = abs_path($0) =~ m|^(.*[/\\])[^/\\]+?$|;
    my $fullname = $basename . "$filename_noex.txt";
    open(my $fh, '>', "$fullname") or die "Could not create new file:'$fullname'\n:: $!";
    foreach my $row (@grid) {
        foreach my $cell ( @{$row} ) {
            print $fh $cell ? "1 " : "0 ";
        }
        print $fh "\n";
    }
    print "Successfully created a binary matrix from file '$filename'\nResult may be seen at '$fullname'";
}


sub load {
	my ($filename) = @_; 
    use open IN => ":crlf";
    open( my $FH, '<', $filename ) or die "Failed opening file $filename with error: $!";
    while (<$FH>) {
        last unless m/^#.*/;
    }

    my $rex1 = qr<B([0-8]*)/S([0-8]*)>i;
    my $rex2 = qr<([0-8]*)/([0-8]*)>;
    my $linerex = qr<^x ?= ?([1-9]\d*), ?y ?= ?([1-9]\d*)(, ?rule ?= ($rex1|$rex2))?$>;
    m/$linerex/;
    my ( $width, $height ) = ( $1, $2 );
    # parse the rule string into arrays
    #print "Rulestring: " . ( $4 || "B3/S23" ) . "\n";

    # my ($born, $survives); 
    # # if ( defined $3 ) {
    # #     my @born     = (0) x 9;
    # #     my @survives = (0) x 9;
    # #     for ( $born = $5 || $8 ; $born ; $born = substr( $born, 1 ) ) {
    # #         $born[ substr( $born, 0, 1 ) ] = 1;
    # #     }
    # #     for (
    # #         $survives = $6 || $7 ;
    # #         $survives ;
    # #         $survives = substr( $survives, 1 )
    # #       )
    # #     {
    # #         $survives[ substr( $survives, 0, 1 ) ] = 1;
    # #     }
    # # }
    # # else {
    # #     my @born     = ( 0, 0, 0, 1, 0, 0, 0, 0, 0 );
    # #     my @survives = ( 0, 0, 1, 1, 0, 0, 0, 0, 0 );
    # # }

    # take the rest of the file and turn it into one long string
    my $string =""; 
    while (<$FH>) {
        chomp;
        $string .= $_;
        last if m/!$/;
    }

    # split that long string into individual row strings
    my @rows = split( qr/\$/, $string );
    my $r = 0;
    my @grid; 
    foreach (@rows) {    # for each row...
        $_ .= '$';    # tack on a dollar sign to make the regexp below simpler
        my @row_grid = ();
        while (m/([1-9]\d*)?([bo\$])/)
        {    # break down the row string into <count><cell_live_status> units
            unless ( $2 eq '$' ) {
                my $p = ( $2 eq 'o' ) ? 1 : 0;
                unless ($1) {
                    push @row_grid, $p;
                }
                else {
                    push @row_grid, ($p) x $1;
                }
            }
            else {
                push @row_grid, (0) x ( $width - scalar(@row_grid) );
                push @grid, \@row_grid;
                my $count = 1;
                while ( $1 && $count++ < $1 ) {
                    push @grid, [ (0) x $width ];
                }
            }
            $_ = substr $_, $+[0]
              ;   # trim the current <run_length><tag> combination from the left
                  # end of the row string
        }
    }
    return @grid;
}
