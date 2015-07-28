#!/usr/bin/perl

sub start($;$$) {
  my $name = shift;
  my $cmd = shift || $name;
  my $cmd2 = shift || $cmd;

  my @pids = split /\s+/, `ps --no-headers -o pid -C $cmd`;
  if ( @pids ) {
    print "$name already started (pid=@pids)\n";
  }
  else {
    print "Starting $name\n";
    system($cmd2);
    select undef, undef, undef, 0.5;
  }
}

system('killall EWDdisplay.pl');
system('killall Xvnc');

sleep 1;

start('Xvnc server', 'Xvnc', 'Xvnc :1 -localhost -rfbauth passwd -geometry 700x300 &');
start('EWD display', 'EWDdisplay.pl', 'DISPLAY=:1 ./EWDdisplay.pl &');
