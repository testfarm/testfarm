##
## TestFarm
## Test Suite Development Wizards - Various Utility Functions
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

package TestFarm::Wiz;

use Cwd;

require Exporter;
@ISA = qw(Exporter);

@EXPORT = qw(
  ERROR
  ERROR0
  ERROR2
  WARN
  WARN2
  INFO
  INFO2
  REPORT_ERR_WARN
  $packname
  $filename
  $dirname
);

@EXPORT_OK = qw(
  location_clear
  location_push
  location_pop
  location_line
  $verbose
  $colors
  $warncount
  $errcount
  @wizlib
  wizlib_init
);


my $ESC = "\x1B\x5B";


############################################################
# Import WIZLIB path list
############################################################

@wizlib = ();          # List of wiz lib pathes

sub wizlib_init {
  @wizlib = ();

  # Import TESTFARM_WIZLIB
  my $env = $ENV{TESTFARM_WIZLIB};
  if ( defined $env ) {
    my @dirs = split /:/, $env;

    foreach my $dir ( @dirs ) {
      if ( -d $dir ) {
        push @wizlib, $dir;
      }
      else {
        WARN2("Environment variable TESTFARM_WIZLIB contains an unreachable directory: $dir");
      }
    }
  }

  # If WIZ lib path is empty, take $PWD/wiz by default (unless not reachable)
  if ( $#wizlib < 0 ) {
    my $dir = getcwd().'/wiz';
    if ( -d $dir ) {
      @wizlib = ( $dir );
    }
  }
}


############################################################
# Target settings
############################################################

$packname = "none";       # Target package name
$filename = "/dev/null";  # Target file name
$dirname = ".";           # Target directory name


############################################################
# Message coloring
############################################################

$colors = 0;


############################################################
# Messages location
############################################################

my $curfile = "";
my $curline = 0;

@location_file = ();
@location_line = ();

sub location_clear {
  @location_file = ();
  $curfile = "";

  @location_line = ();
  $curline = 0;
}

sub location_push {
  my ($file, $line) = @_;

  push @location_file, $curfile;
  $curfile = $file;

  push @location_line, $curline;
  $curline = $line;
}

sub location_pop {
  $curfile = pop @location_file;
  $curline = pop @location_line;

  return ($curfile, $curline);
}


sub location_line {
  $curline = shift;
}


############################################################
# Error messages
############################################################

$errcount = 0;

sub ERROR {
  ERROR0("@_");
  $errcount++;
}

sub ERROR0 {
  if ($curline > 0) {
    print STDERR $ESC."04;31m" if $colors;
    print STDERR "$curfile:$curline:";
    print STDERR $ESC."00m" if $colors;
    print STDERR " ";
  }
  print STDERR $ESC."02;31m" if $colors;
  print STDERR "[ERROR] @_\n";
  print STDERR $ESC."00m" if $colors;
}

sub ERROR2 {
  print STDERR $ESC."02;31m" if $colors;
  print STDERR "[ERROR] @_\n";
  print STDERR $ESC."00m" if $colors;
  $errcount++;
}


############################################################
# Warning messages
############################################################

$warncount = 0;

sub WARN {
  my $text = "@_";
  chomp $text;

  if ($curline > 0) {
    print STDERR $ESC."04;34m" if $colors;
    print STDERR "$curfile:$curline:";
    print STDERR $ESC."00m" if $colors;
    print STDERR " ";
  }
  print STDERR $ESC."02;34m" if $colors;
  print STDERR "[WARNING] $text\n";
  print STDERR $ESC."00m" if $colors;
  $warncount++;
}

sub WARN2 {
  print STDERR "[WARNING] @_\n";
  $warncount++;
}


############################################################
# Informational messages
############################################################

$verbose = 0;

sub INFO {
  my $level = shift;

  if ( $verbose >= $level ) {
    print STDERR "$curfile:$curline: " if ($curline > 0);
    print STDERR "[INFO] @_\n";
  }
}


sub INFO2 {
  my $level = shift;

  print STDERR "[INFO] @_\n" if ( $verbose >= $level );
}


############################################################
# Error/Warning report
############################################################

sub REPORT_ERR_WARN {
  my $filein = shift;

  if ( ($warncount > 0) || ($errcount > 0) ) {
    print STDERR "*** ";
  }

  if ( $warncount > 0 ) {
    print STDERR "$warncount warning".(($warncount > 1) ? "s":"");
    if ( $errcount > 0 ) {
      print STDERR ", ";
    }
  }

  if ( $errcount > 0 ) {
    print STDERR "$errcount error".(($errcount > 1) ? "s":"");
  }

  if ( ($warncount > 0) || ($errcount > 0) ) {
    print STDERR " encountered";
    if ( defined $filein ) { print STDERR " while processing $filein"; }
    print STDERR "\n";
  }

  return $errcount;
}

1;
