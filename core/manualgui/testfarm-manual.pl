#!/usr/bin/perl -w

##
## TestFarm
## Manual User Interface
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

use File::Basename;
use IO::Handle;
use FileHandle;
use POSIX ":sys_wait_h";
use Errno 'EINTR';
use Cwd;

use TestFarm::Env;
use TestFarm::Service;
use TestFarm::Engine;


my $banner = 'testfarm-manual';


###########################################################
# Wait for cryptoperl feeder termination (if any)
###########################################################

if ( defined $__feeder__ ) {
  #print STDERR "-- $banner: FEEDER $__feeder__\n";
  waitpid($__feeder__, 0);
}


###########################################################
# Get & Check arguments
###########################################################

my $config_filein = undef;
my $glade_filein = undef;
my $verbose = 0;
my $ctl = undef;
my $logview = 0;

sub usage {
  my $rev = '$Revision: 542 $ ';
  $rev =~ /^\$Revision: (\S+)/;
  print STDERR "TestFarm Manual User Interface - version $1\n" if defined $1;
  print STDERR "Usage: $banner [-v] [-c<fd>] [-logview] [-i <glade-file>] [<config-file>]\n";
  exit(1);
}

for (my $i = 0; $i <= $#ARGV; $i++) {
  my $arg = $ARGV[$i];

  if ( $arg =~ /^-/ ) {
    if ( $arg eq "-v" ) {
      $verbose++;
    }
    elsif ( $arg =~ /^-c(\d+)$/ ) {
      $ctl = $1;
    }
    elsif ( $arg eq "-logview" ) {
      $logview++;
    }
    elsif ( $arg eq "-i" ) {
      usage() if ( $i >= $#ARGV );
      $glade_filein = $ARGV[++$i];
      usage() if ( $glade_filein =~ /^-/ );
    }
    else {
      usage();
    }
  }
  else {
    usage() if ( defined $config_filein );
    $config_filein = $arg;
  }
}


unless ( defined $config_filein ) {
  my @config_list = ();

  foreach my $dir ( $PWD, @libs ) {
    local *DIR;
    my $file;
    opendir(DIR, $dir);
    while ( defined ($file = readdir(DIR)) ) {
      push @config_list, $dir.'/'.$file if ( $file =~ /\.xml$/ );
    }
    closedir(DIR);
  }

  if ( $#config_list < 0 ) {
    print STDERR "No System Configuration file found\n";
    usage();
  }
  elsif ( $#config_list == 0 ) {
    $config_filein = $config_list[0];
  }
  else {
    print STDERR "Several System Configurations are available. Please specify one: @config_list\n";
    usage();
  }
}

unless ( -f $config_filein ) {
  print STDERR "Cannot find System Configuration file $config_filein\n";
  exit(2);
}


unless ( defined $glade_filein ) {
  $glade_filein = $config_filein;
  $glade_filein =~ s/\.xml$/.glade/;
}


my ($config_basename, $config_dirname) = fileparse($config_filein, (".xml"));

if ( $verbose ) {
  print STDERR "System Configuration Name: $config_basename\n";
  print STDERR "System Configuration File: $config_filein\n";
  print STDERR "Glade Interface File     : $glade_filein\n";
}


STDOUT->autoflush(1);


###########################################################
# Load the Test Feature library
###########################################################

my $lib_file = $config_filein;
$lib_file =~ s/\.xml$/.pm/;

unless ( -f $lib_file ) {
  foreach ( $PWD, @libs ) {
    my $file = $_.'/'.$config_basename.'.pm';
    if ( -f $file ) {
      $lib_file = $file;
      last;
    }
  }
}

unless ( -f $lib_file ) {
  print STDERR "Cannot find Test Feature library $config_basename.pm\n";
  exit(2);
}

print STDERR "Loading Test Feature library $lib_file\n" if $verbose;

my ($lib, $lib_dir) = fileparse($lib_file, (".pm"));
$lib_dir =~ s/\/+$//;

require $lib_file;
#$lib->import();


###########################################################
# Open the interface control pipes
###########################################################

if ( defined $ctl ) {
  # Create parent control pipe
  unless ( open(CTL_RD, "<&=$ctl") ) {
    print STDERR "$banner: Cannot open control pipe fd: $!\n";
    exit(3);
  }

  # Create interface control pipe
  unless ( pipe(CTL, CTL_WR) ) {
    print STDERR "$banner: Cannot create control pipe: $!\n";
    exit(3);
  }

  # Disable close-on-exec mode on read endpoint
  unless ( fcntl(CTL, &F_SETFD, 0) ) {
    print STDERR "$banner: Cannot setup control pipe: $!\n";
    exit(3);
  }

  # Enable close-on-exec mode on write endpoint
  unless ( fcntl(CTL_WR, &F_SETFD, &FD_CLOEXEC) ) {
    print STDERR "$banner: Cannot setup control pipe: $!\n";
    exit(3);
  }

  CTL_WR->autoflush(1);
}


###########################################################
# Start the Graphical User Interface
###########################################################

sub sigchld {
  while ( waitpid(-1, WNOHANG) > 0 ) {}
}

sub sigterm {
  exit(0);
}

$SIG{CHLD} = \&sigchld;
$SIG{TERM} = \&sigterm;
$SIG{PIPE} = \&sigterm;

$INTERFACE_pid = undef;

$INTERFACE_cmd = 'testfarm-manual-interface ';
if ( defined $ctl ) {
  $INTERFACE_cmd .= '-c'.fileno(CTL).' ';
}
$INTERFACE_cmd .= $glade_filein;

$INTERFACE_pid = open(INTERFACE, "exec $INTERFACE_cmd|");

unless ( $INTERFACE_pid ) {
  print STDERR "Cannot launch Manual Interface '$glade_filein': $!\n";
  exit(3);
}

# Close useless control pipe endpoint
close(CTL);

if ( $verbose ) {
  print STDERR "Manual Interface started: pid=$INTERFACE_pid\n";
}

INTERFACE->autoflush(1);


###########################################################
# Start the Test Services
###########################################################

if ( ${$lib."::"}{"START_SERVICE"} ) {
  &{$lib."::START_SERVICE"}("MANUAL");
  sleep 1;
}


###########################################################
# Start the Test Engine
###########################################################

my $logname = $logview ? "testfarm-manual.log" : "/dev/null";

TestFarm::Engine::start($logname);
TestFarm::Engine::echo("*** Starting Manual Interface");
TestFarm::Engine::version();

my $logview_pid = undef;
if ( $logview ) {
  my $cmd = "testfarm-logview -t 'Manual Interface' $logname";

  $logview_pid = fork();
  if ( defined $logview_pid ) {
    if ( $logview_pid == 0 ) {
      unless ( exec($cmd) ) {
        print STDERR "Failed to exec Log Viewer \"$cmd\" (pid=$$): $!\n";
        exit(3);
      }
    }
  }
  else {
    print STDERR "Failed to fork Log Viewer process: $!\n";
  }
}

&{$lib."::START"}("MANUAL") && exit(3);


###########################################################
# Main processing loop
###########################################################

my $rin = '';
if ( defined $ctl ) {
  vec($rin, $ctl, 1) = 1;
}
vec($rin, fileno(INTERFACE), 1) = 1;

while ( 1 ) {
  my $rout;
  my $eout;
  my ($n) = select($rout=$rin, undef, undef, undef);

  if ( $n < 0 ) {
    if ( int($!) != EINTR ) {
      printf STDERR "select: $!\n";
      last;
    }
  }
  elsif ( $n > 0 ) {
    if ( defined $ctl ) {
      if ( vec($rout, $ctl, 1) ) {
	my $line = <CTL_RD>;
	last unless defined $line;
	print CTL_WR $line;
      }
    }

    if ( vec($rout, fileno(INTERFACE), 1) ) {
      my $line = <INTERFACE>;
      last unless defined $line;
      chomp($line);
      my $prefix = ($line =~ s/^\$//) ? '$':'';
      my $func = $prefix.$lib."::".$line;
      print "$func\n";
      eval($func);
    }
  }
}


if ( $verbose ) {
  print STDERR "Manual Interface terminated\n";
}

exit(0);


END {
  $SIG{TERM} = 'IGNORE';
  $SIG{PIPE} = 'IGNORE';

  if ( defined $ctl ) {
    close(CTL_WR);
    close(CTL_RD);
    $ctl = undef;
  }

  if ( defined $INTERFACE_pid ) {
    kill('TERM', $INTERFACE_pid);
    $INTERFACE_pid = undef;
    close(INTERFACE);
  }

  if ( defined $logview_pid ) {
    kill('TERM', $logview_pid);
    $logview_pid = undef;
  }

  if ( defined $lib ) {
    &{$lib."::STOP"}();
    if ( ${$lib."::"}{"STOP_SERVICE"} ) {
      &{$lib."::STOP_SERVICE"}("MANUAL");
    }
  }
}
