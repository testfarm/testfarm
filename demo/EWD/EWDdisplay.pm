package EWDdisplay;

use IO::Socket;

my $SOCK;

sub send {
  foreach ( @_ ) {
    $SOCK->send($_);
  }
}


sub init {
  my $ADDR = shift || '127.0.0.1';
  my $PORT = shift || 6789;

  $SOCK = IO::Socket::INET->new(PeerAddr => $ADDR,
				PeerPort => $PORT,
				Proto => 'udp');
  unless ( $SOCK ) {
    print STDERR "Cannot create UDP socket at $ADDR:$PORT: $!\n";
    exit(1);
  }

  print "EWDdisplay: Creating UDP socket at $ADDR:$PORT\n";
  return 0;
}

1;

