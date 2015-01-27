##
## TestFarm
## Test Suite Execution Library
## Error Location Retriever
##
## $Revision: 250 $
## $Date: 2006-10-03 16:27:18 +0200 (mar., 03 oct. 2006) $
##

package TestFarm::Locate;

use FileHandle;
use IPC::Open2;
use POSIX;

use TestFarm::Env;


sub prefix {
  my $msg = shift;

  my $prefix = "";

  if ( /\s+at\s+(\S+)\s+line\s+(\d+)/ ) {
    my $filename = $1;
    my $line = $2;

    local *FIN;
    if ( open(FIN, $filename) ) {
      my $wizname = $filename;
      my $wizcount = 0;

      my $count = 0;

      while ( <FIN> ) {
        $count++;
        $wizcount++;

        last if ( $count >= $line );

        if ( /^\#\$LINE (\S+) (\d+)$/ ) {
          $wizname = $1;
          $wizcount = $2;
        }
      }
      close(FIN);

      if ( $wizname ) {
        $wizname =~ s{^$PWD/}{} if defined $PWD;
        $prefix = "$wizname:";
        $prefix .= "$wizcount:" if ( $wizcount > 0 );
      }
    }
  }

  return $prefix;
}


sub filter_server {
  # Acknowledge client process we are entering filtering loop
  STDOUT->autoflush(1);
  print STDOUT "#BEGIN $$\n";

  # The filtering loop
  while ( <STDIN> ) {
    my $msg = $_;
    last if $msg eq "#KILL $$\n";
    my $prefix = TestFarm::Locate::prefix($_);
    print STDERR $prefix." " unless ( $prefix eq "" );
    print STDERR $msg;
  }

  # Acknowledge client process we are leaving filtering loop
  print STDOUT "#END $$\n";
  exit(0);
}


my $filter_pid = undef;


sub filter_ack {
  my $FILTER_IN = shift;

  # Wait for filtering server to start
  my $rin = '';
  vec($rin, fileno($FILTER_IN), 1) = 1;
  my $n = select($rin, undef, undef, 2.0);
  return undef if ( $n <= 0 );

  my $ack = <$FILTER_IN>;

  return $ack;
}


sub filter_client {
  filter_client_kill();

  # Prepare communication with filtering server
  $FILTER_IN = FileHandle->new;
  $FILTER_OUT = FileHandle->new;

  # Spawn filtering server subprocess
  $filter_pid = open2($FILTER_IN, $FILTER_OUT, "perl -MTestFarm::Locate -e 'TestFarm::Locate::filter_server'");
  if ( $filter_pid < 0 ) {
    $filter_pid = undef;
    return -1;
  }

  # Wait for filtering server to start
  my $ack = filter_ack($FILTER_IN);
  unless ( (defined $ack) && ($ack eq "#BEGIN $filter_pid\n") ) {
    kill('TERM', $filter_pid);
    $filter_pid = undef;
    return -3;
  }

  # Save original STDERR
  unless ( open(FILTER_STDERR, ">&STDERR") ) {
    print STDERR "Couldn't save STDERR for filtering\n";
    kill('TERM', $filter_pid);
    $filter_pid = undef;
    return -4;
  }

  # Useless but required to prevent perl from bursting a warning message
  my $trash = fileno(FILTER_STDERR); $trash = 0;

  # If server seems to be ok, redirect STDERR to it
  dup2(fileno($FILTER_OUT), fileno(STDERR));

  return $filter_pid;
}


sub filter_client_kill {
  if ( $filter_pid ) {
    # Send KILL message to filtering server
    print STDERR "#KILL $filter_pid\n";

    # Restore original STDERR
    open(STDERR, ">&FILTER_STDERR");

    # Wait for filtering server to finish
    my $ack = filter_ack($FILTER_IN);
    # If filtering server does not respond, kill it
    unless ( (defined $ack) && ($ack eq "#END $filter_pid\n") ) {
      kill('TERM', $filter_pid);
    }

    # Acknowledge filtering server death
    waitpid($filter_pid, 0);
    $filter_pid = undef;
  }
}

END {
  filter_client_kill();
}

1;
