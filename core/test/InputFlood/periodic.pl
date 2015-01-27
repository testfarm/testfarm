#! /usr/bin/perl -w

# ===========================================================================
# Interfacelet that sends a message every second
# does nothing really useful.
# Three commands are recognized:
#   version      Display version number
#   echo <text>  Print a text
#   exit         Exit the command interpreter
#   tag [<tag>]  Set/show message tag to send and clear the message counter
#   rate [<ms>]  Set/show message sending rate in milliseconds (0 for stop)
# Other commands produce an error message.
# ===========================================================================

use FileHandle;
use Time::HiRes qw(gettimeofday);
use Glib;


# Construct the version id
my $version = '$Revision: 1087 $ ';  # SVN puts the right value here
$version =~ s/[\s\$:]//g;
$version =~ s/Revision//;
$version = 0 if ( $version eq "" );

# Init GLib execution loop
my $mainloop = Glib::MainLoop->new();

# Get the big-bang date
$t0 = gettimeofday();

# Hook input events handler
STDIN->autoflush(1);
STDIN->blocking(0);
my $stdin_tag = Glib::IO->add_watch(fileno(STDIN), 'in', \&stdin_hdl);

my $timeout_tag = undef;
my $timeout_delay = 0;

my $msg_tag = 'EVENT';
my $msg_count = 0;

# Disable output buffering
STDOUT->autoflush();

print tstamp()." INIT Periodic message sender\n";

$mainloop->run();


sub tstamp {
  my $t1 = gettimeofday();
  return int(($t1 - $t0) * 1000000);
}


sub timeout_hdl {
  $msg_count++;
  print tstamp()." $msg_tag $msg_count\n";
  return 1;
}


sub stdin_hdl {
  while ( defined ($line = STDIN->gets()) ) {
    $line =~ s/^\s+//;
    $line =~ s/\s+$//;
    next if $line =~ /^$/;

    # Get the command and its arguments
    my ($cmd, @args) = split /\s+/, $line;

    # Get the time stamp in microseconds
    my $tstamp = tstamp();

    # Process the command
    if ( $cmd eq "version" ) {
      printf "%d VERSION $version\n", $tstamp;
    }
    elsif ( $cmd eq "echo" ) {
      printf "%d ECHO @args\n", $tstamp;
    }
    elsif ( $cmd eq "exit" ) {
      exit(0);
    }
    elsif ( $cmd eq "tag" ) {
      if ( $#args >= 0 ) {
	$msg_tag = $args[0];
	$msg_count = 0;
      }

      printf "%d TAG $msg_tag\n", $tstamp;
    }
    elsif ( $cmd eq "rate" ) {
      if ( defined $timeout_tag ) {
	Glib::Source->remove($timeout_tag);
	$timeout_delay = 0;
	$timeout_tag = undef;
      }

      if ( $#args >= 0 ) {
	$timeout_delay = $args[0];
	if ( $timeout_delay > 0 ) {
	  $timeout_tag = Glib::Timeout->add($timeout_delay, \&timeout_hdl);
	}
      }

      printf "%d RATE $timeout_delay\n", $tstamp;
    }
    else {
      printf "%d ERROR Illegal command '$cmd'\n", $tstamp;
    }
  }

  return 1;
}
