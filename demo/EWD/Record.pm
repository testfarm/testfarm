package Record;

use IO::Handle;
use Time::HiRes qw(gettimeofday);


my $NAME = 'RECORD';
my $EVENT = undef;
my $CALIBRATE = undef;

my $t0 = 0;
my $t1 = 0;
my $debug = 0;

# Init position recording
my $value = undef;
my $prev = undef;
my $min = undef;
my $max = undef;

my $record = new IO::Handle;
my $record_fname = undef;


sub Init {
  $NAME = shift || 'RECORD';
  $EVENT = shift;
  $CALIBRATE = shift;

  # Get the big-bang date
  $t0 = gettimeofday();
}


sub Process() {
  # Enter processing loop
  while ( defined (my $msg = STDIN->getline()) ) {
    process_message($msg);
  }
}


sub show_value {
  my $value = shift;
  return ( defined $value ) ? sprintf("%.1f", $value) : '-';
}


sub process_command {
  my $msg = shift;

  $msg =~ s/^\s+//;
  $msg =~ s/\s+$//;

  if ( $debug ) {
    print "COMMAND '$msg'\n";
  }

  return 0 if ( $msg eq '' );
  my ($cmd, @argv) = split /\s+/, $msg;

  if ( $cmd eq 'clear' ) {
    $min = $value;
    $max = $value;
  }
  elsif ( $cmd eq 'show' ) {
    print 'VALUE='.show_value($value).' MIN='.show_value($min).' MAX='.show_value($max)."\n";
  }
  elsif ( $cmd eq 'record' ) {
    if ( $record->opened() ) {
      $record->close();
      print "FILE $record_fname closed\n";
      $record_fname = undef;
    }

    $record_fname = $argv[0] || $name;
    if ( defined $record_fname ) {
      unless ( open($record, ">$record_fname") ) {
	print STDERR "$NAME: Cannot create record file '$record_fname'\n";
	$record_fname = undef;
      }
    }
  }
  elsif ( $cmd eq 'calibrate' ) {
    if ( defined $CALIBRATE ) {
      &{$CALIBRATE}(@argv);
    }
  }
  elsif ( $cmd eq 'debug' ) {
    $debug = $argv[0];
    printf 'DEBUG='.($debug ? 'on':'off')."\n";
  }
  else {
    print STDERR "$NAME: Unknown command '$cmd'\n";
  }
}


sub process_event {
  my $msg = shift;

  return unless defined $EVENT;

  $msg =~ s/^(\S+)\s+(\S+)\s*//;
  my $id = $1;
  my $action = $2;

  my $t = gettimeofday() - $t0;

  $value = &{$EVENT}($id, $action, $msg, $t);

  if ( $debug ) {
    print "EVENT ".($action eq 'APPEAR' ? '+':'-')."$id : ".show_value($value)."\n";
  }

  if ( defined $value ) {
    if ( (! defined $min) || ($value < $min) ) {
      $min = $value;
    }
    if ( (! defined $max) || ($value > $max) ) {
      $max = $value;
    }
  }

  if ( $record->opened ) {
    if ( $value != $prev ) {
      $record->printf("%.3f %d\n", $t, (defined $value) ? $value : -1);
    }
  }

  $prev = $value;
}


sub process_message {
  my $msg = shift;

  $msg =~ s/^\s+//;
  $msg =~ s/\s+$//;
  return if $msg =~ /^$/;

  if ( $msg =~ s/^\+\+\s+// ) {
    process_command($msg);
  }
  else {
    process_event($msg);
  }
}


1;
