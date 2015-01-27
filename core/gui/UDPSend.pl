#!/usr/bin/perl -w

use IO::Socket;

my $MSG = $ARGV[0] || '';

my $SOCK = IO::Socket::INET->new(PeerAddr => '127.0.0.1',
				 PeerPort => 2345,
				 Proto => 'udp');

$SOCK->send($MSG);

$SOCK->recv($RET, 1024);
my ($port, $addr) = sockaddr_in($SOCK->peername);
my $host = inet_ntoa($addr);
print "-- $host:$port -> $RET\n";
