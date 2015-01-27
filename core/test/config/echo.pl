#! /usr/bin/perl -w

# ===========================================================================
# The Sample ECHO Command Interpreter
# A dummy Test Interface command interpreter that
# does nothing really useful.
# Three commands are recognized:
#   version displays the command interpreter version,
#           according to the CVS repository it is stored in
#   echo    echos the arguments it is given
#   exit    exits the command interpreter
# Other commands produce an error message.
# ===========================================================================

use FileHandle;
use Time::HiRes qw(gettimeofday);

# Construct the version id
my $version = '$Revision: 1236 $ ';  # SVN puts the right value here
$version =~ s/[\s\$:]//g;
$version =~ s/Revision//;
$version = 0 if ( $version eq "" );

# Get the big-bang date
$t0 = gettimeofday();

# Disable output buffering
STDOUT->autoflush();

while ( <STDIN> ) {
  my ($cmd, @args) = split /\s+/;

  # Compute and dump the time stamp in microseconds
  my $t1 = gettimeofday();
  my $tstamp = ($t1 - $t0) * 1000000;

  # Process the command
  if ( $cmd eq "version" ) {
    printf "%d VERSION $version\n", $tstamp;
  }
  elsif ( $cmd eq "echo" ) {
    printf "%d ECHO @args\n", $tstamp;
  }
  elsif ( $cmd eq "exit" ) {
    last;
  }
  else {
    printf "%d ERROR Illegal command '$cmd'\n", $tstamp;
  }
}
