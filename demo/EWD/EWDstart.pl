#!/usr/bin/perl

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

stop('EWDdisplay.pl', 'Xvnc');

start('Xvnc server', 'Xvnc', 'Xvnc :1 -localhost -rfbauth passwd -geometry 700x300 &');
start('EWD display', 'EWDdisplay.pl', 'DISPLAY=:1 ./EWDdisplay.pl &');
