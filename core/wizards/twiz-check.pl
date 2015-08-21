#!/usr/bin/perl -w

##
## TestFarm
## Test Suite Development Wizards - Script Syntax Checker
##
## Author: Sylvain Giroudon
## Creation: 21-JUN-2006
##
## This file is part of TestFarm,
## the Test Automation Tool for Embedded Software.
## Please visit http://www.testfarm.org.
##
## TestFarm is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## TestFarm is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
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
