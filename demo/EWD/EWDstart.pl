#!/usr/bin/perl

use File::Basename;

sub pids {
    my $args = '';
    foreach (@_) {
	$args .= " -C '$_'";
    }

    my @pids = ();
    foreach (split /\s+/, `ps --no-headers -o pid $args`) {
	s/^\s+//;
	push @pids, $_ if $_;
    }

    return @pids;
}


sub start($;$$) {
    my $name = shift;
    my $cmd = shift || $name;
    my $cmd2 = shift || $cmd;

    my @pids = pids($cmd);
    if (@pids) {
	print "$name already started (pid=@pids)\n";
    }
    else {
	print "Starting $name\n";
	system($cmd2);
	select undef, undef, undef, 0.5;
    }
}


sub stop {
    my @pids = pids(@_);
    if (@pids) {
	print "Stopping @_ (@pids)\n";

	foreach (@pids) {
	    kill('TERM', $_);
	}

	sleep(1);

	foreach (@pids) {
	    kill('KILL', $_);
	}
    }
}

my $dir = dirname($0);

my $option = $ARGV[0];
my $restart = 1;

if ($option) {
    if ($option eq '-k') {
	$restart = 0;
    }
    else {
	print STDERR "Usage: $0 [-k]\n\n";
	print STDERR "(Re)starts the TestFarm EWD demo application in a Xvnc virtual display.\n\n";
	print STDERR "Available options:\n";
	print STDERR "  -k : Kill only, do not restart application\n";
	exit(($option eq '-h') ? 0:1);
    }
}

stop('EWDdisplay.pl', 'Xvnc');

exit(0) unless $restart;

my $xauthorityFile = $dir.'/.Xauthority';

unless (-f $xauthorityFile) {
    my $host = `uname -n`;
    chomp($host);

    print STDERR "Creating Xauthority file for '$host'\n";

    my $cookie = `/usr/bin/mcookie`;
    chomp($cookie);

    if (open(my $auth, "|xauth -f $xauthorityFile source -")) {
	print $auth "add $host:1 . $cookie\n";
	print $auth "add $host/unix:1 . $cookie\n";
	close($auth);
    }
}

start('Xvnc server', 'Xvnc', "Xvnc :1 -auth '$xauthorityFile' -localhost -rfbauth '$dir/passwd' -geometry 700x300 &");
start('EWD display', 'EWDdisplay.pl', "XAUTHORITY='$xauthorityFile' DISPLAY=:1 '$dir/EWDdisplay.pl' &");

exit(0);
