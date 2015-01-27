#!/usr/bin/perl -w

##
## A script that paste a GladeXML definition into a PERL script
##
## Author: Sylvain Giroudon
## Creation: 07-JUN-2004
##

if ( $#ARGV != 2 ) {
  print STDERR "Usage: paste.pl <keyword> <input-file> <paste-file>\n";
  exit 2;
}

my $keyword = $ARGV[0];
my $input = $ARGV[1];
my $paste = $ARGV[2];

open(INPUT, $input)
  or die "Cannot open input file $input: $!\n";

while ( <INPUT> ) {
  my $line = $_;

  if ( $line =~ /^$keyword\s+$/ ) {
    local *PASTE;
    if ( open(PASTE, $paste) ) {
      while ( <PASTE> ) {
	print $_;
      }
      close(PASTE);
    }
    else {
      print STDERR "Cannot open paste file $paste: $!\n";
    }
  }
  print $line;
}

close(INPUT);

exit 0;
