##
## TestFarm
## Service Starter
## (C) Basil Dev 2006
##
## $Revision: 151 $
## $Date: 2006-07-19 16:39:55 +0200 (mer., 19 juil. 2006) $
##

package TestFarm::Service;

sub pidof {
  my $cmdline = shift || return undef;
  my ($cmd) = split /\s+/, $cmdline;

  my @pids = split /\s+/, `ps --no-headers -o pid -C $cmd`;
  foreach ( @pids ) {
    next if /^$/;
    chomp(my $cmdproc = `ps --no-headers -o cmd -p $_`);
    return $_ if ( $cmdproc eq $cmdline );
  }

  return undef;
}

sub start {
  my $cmdline = shift || return 0;
  my $pid = pidof($cmdline);

  if ( defined $pid ) {
    print STDERR "TestFarm::Service: Process \"$cmdline\" already started (pid=$pid)\n";
  }
  else {
    $pid = fork();
    if ( defined $pid ) {
      if ( $pid == 0 ) {
        unless ( exec($cmdline) ) {
          print STDERR "TestFarm::Service: Failed to exec command \"$cmdline\" (pid=$$): $!\n";
          exit(255);
        }
      }
    }
    else {
      print STDERR "TestFarm::Service: Failed to fork new process: $!\n";
    }
  }

  return $pid;
}

sub stop {
  my $cmdline = shift || return 0;
  my $signal = shift || "TERM";
  my $pid = pidof($cmdline);

  if ( defined $pid ) {
    kill($signal, $pid);
  }

  return $pid;
}

1;
