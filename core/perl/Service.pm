##
## TestFarm
## Service Starter
##
## This file is part of TestFarm, 
## the Test Automation Tool for Embedded Software.
## Please visit http://www.testfarm.org.
##
## TestFarm is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## TestFarm is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
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
