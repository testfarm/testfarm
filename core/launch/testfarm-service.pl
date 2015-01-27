#!/usr/bin/perl -w

##
## TestFarm - Service Starter
## (C) Basil Dev 2006
##
## $Revision: 250 $
## $Date: 2006-10-03 16:27:18 +0200 (mar., 03 oct. 2006) $
##

use File::Basename;
use POSIX ":sys_wait_h";
use Cwd;

use TestFarm::Env;
use TestFarm::Service;


###########################################################
# Get & Check arguments
###########################################################

my %HANDLER = (
  "start" => "START_SERVICE",
  "stop"  => "STOP_SERVICE",
);

my $verbose = 0;
my $mode = "AUTO";
my $config = undef;
my $action = undef;

sub usage {
  my $rev = '$Revision: 250 $ ';
  $rev =~ /^\$Revision: (\S+)/;
  print STDERR "TestFarm Service Starter - version $1\n" if defined $1;
  print STDERR "Usage: testfarm-service [-v] [-m <mode>] start|stop [<config-file>]\n";
  exit(1);
}

for (my $i = 0; $i <= $#ARGV; $i++) {
  my $arg = $ARGV[$i];

  if ( $arg =~ /^-/ ) {
    if ( $arg eq "-v" ) {
      $verbose++;
    }
    elsif ( $arg eq "-m" ) {
      usage() if ( $i >= $#ARGV );
      $mode = $ARGV[++$i];
      usage() if ( $mode =~ /^-/ );
    }
    else {
      usage();
    }
  }
  else {
    if ( defined $action ) {
      if ( defined $config ) {
        usage();
      }
      else {
        $config = $arg;
      }
    }
    else {
      $action = $arg;
    }
  }
}

usage() unless ( defined $action );
my $handler = $HANDLER{$action};
usage() unless ( defined $handler );

unless ( defined $config ) {
  my @config_list = ();

  foreach my $dir ( getcwd(), @libs ) {
    local *DIR;
    my $file;
    opendir(DIR, $dir);
    while ( defined ($file = readdir(DIR)) ) {
      next if $file !~ s/\.xml$//;
      push @config_list, $file;
    }
    closedir(DIR);
  }

  if ( $#config_list < 0 ) {
    print STDERR "No System Configuration file found\n";
    usage();
  }
  elsif ( $#config_list == 0 ) {
    $config = $config_list[0];
  }
  else {
    print STDERR "Several System Configurations are available. Please specify one: @config_list\n";
    usage();
  }
}

if ( $verbose ) {
  print STDERR "System Configuration: $config\n";
}


###########################################################
# Load the Test Feature library
###########################################################

my $lib_file = undef;

foreach ( getcwd(), @libs ) {
  my $file = "$_/$config";
  if ( -f $file ) {
    $lib_file = $file;
    last;
  }

  $file = "$_/$config.pm";
  if ( -f $file ) {
    $lib_file = $file;
    last;
  }
}

unless ( $lib_file ) {
  my ($base, $dir) = fileparse($config, (".pm"));
  print STDERR "Cannot find Test Feature library: $base.pm\n";
  exit(2);
}

print STDERR "Loading Test Feature library: $lib_file\n" if $verbose;

my ($lib, $lib_dir) = fileparse($lib_file, (".pm"));
$lib_dir =~ s/\/+$//;

require $lib_file;


###########################################################
# Start the Test Services
###########################################################


if ( ${$lib."::"}{$handler} ) {
  print STDERR "Performing $config services $action\n" if $verbose;
  &{$lib."::".$handler}($mode);
  #sleep 1;
}
else {
  print STDERR "No way to perform $config services $action: function $handler not defined\n";
  exit(1);
}

exit(0);
