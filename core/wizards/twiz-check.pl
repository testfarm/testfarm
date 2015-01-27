#!/usr/bin/perl -w

##
## TestFarm
## Test Suite Development Wizards
## Script Syntax Checker
##
## $Revision: 258 $
## $Date: 2006-10-04 15:39:16 +0200 (mer., 04 oct. 2006) $
##

use POSIX;
use IPC::Open3;
use IO::Select;

use TestFarm::Env;
use TestFarm::Locate;


my $ESC = "\x1B\x5B";


############################################################
# Get options
############################################################

my @pmlist = ();
my $colors = isatty('STDERR');

sub usage {
  my $rev = '$Revision: 258 $ ';
  $rev =~ /^\$Revision: (\S+)/;
  print STDERR "TestFarm Script Syntax Checker - version $1\n" if defined $1;
  print STDERR "Usage: twiz-check [--colors] [--nocolors] <pm-file> ...\n";
  exit(1);
}

sub parse_options {
  my $opt = shift;

  if ( $opt eq "--colors" ) {
    $colors = 1;
  }
  elsif ( $opt eq "--nocolors" ) {
    $colors = 0;
  }
  else {
    return -1;
  }

  return 0;
}

if ( open(FI, ".twiz") ) {
  while ( <FI> ) {
    chomp;
    parse_options($_);
  }
  close(FI);
}

foreach ( @ARGV ) {
  if ( /^-/ ) {
    usage if parse_options($_);
  }
  else {
    push @pmlist, $_;
  }
}

usage() if ( $#pmlist < 0 );

my $ret = 0;

foreach my $file ( @pmlist ) {
  my $cmd = "perl -w -c";
  foreach ( @TestFarm::Env::libs ) { $cmd .= " -I$_" }
  $cmd .= " $file";
  #print "---- CMD = '$cmd'\n";
  my $pid  = open3(INPUT, OUTPUT, "", $cmd);

  my $trash = fileno(INPUT);  # Fool PERL interpreter to keep warning quiet

  while ( <OUTPUT> ) {
    chomp;
    my $msg = $_;

    my $prefix = TestFarm::Locate::prefix($msg);

    # Colorize messages if they are displayed on a terminal
    if ( $colors ) {
      if ( $msg =~ /syntax OK$/ ) {
        $msg = $ESC."02;32m" . $msg . $ESC."00m";
      }
      else {
        $msg = $ESC."02;31m" . $msg . $ESC."00m";
      }

      if ( $prefix ne "" ) {
        $prefix = $ESC."04;31m" . $prefix . $ESC."00m";
      }
    }

    $prefix .= " " unless ( $prefix eq "" );
    print STDERR $prefix.$msg."\n";
  }

  waitpid($pid, 0);
  if ( $? ) {
    $ret = 1;

    # Remove target file if it contains errors
    my $wizfile = $file;
    $wizfile =~ s/\.pm$/\.wiz/;
    if ( -f $wizfile ) {
      #print STDERR "[INFO] Target file $file deleted\n";
      unlink $file;
    }
  }

  close(INPUT);
  close(OUTPUT);
}

exit($ret);
