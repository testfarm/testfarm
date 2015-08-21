##
## TestFarm
## Test Suite Execution Library - Test Engine Command/Reply Management
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

package TestFarm::Engine;

use IO::Handle;
use POSIX ":sys_wait_h";
use Fcntl;
use Cwd;

require Exporter;
@ISA = qw(Exporter);

@EXPORT = qw(
  ReportDir
  LocalLog
  GlobalLog
  TestCase
);


$pid = undef;

my $terminated = 0;
my $report_dir = undef;
my $local_log = "result.log";
my $global_log = "global.log";
my $case_name = "";
my $old_timeout = "";


sub sigchld {
  while ( 1 ) {
    my $ret = waitpid(-1, WNOHANG);
    return if ( $ret <= 0 );

    if ( (defined $pid) && ($ret == $pid) ) {
      $pid = undef;
      unless ( $terminated ) {
	print STDERR "TestFarm::Engine: Test Engine terminated\n";
	exit(255);
      }
    }
  }
}


sub sigpipe {
  if ( defined $pid ) {
    print STDERR "TestFarm::Engine: Broken pipe\n";
    exit(255);
  }
}


sub sigterm {
  if ( defined $pid ) {
    print STDERR "TestFarm::Engine: Terminated\n";
    $terminated = 1;
    kill('TERM', $pid);
    exit(0);
  }
}


sub start {
  my $global = shift;
  $global_log = $global if defined $global;
  print "Global log file: $global_log\n";

  # Create synchronization pipe for WAIT/DONE/CLOSE
  local *WPIPE_WR;
  pipe(WPIPE, WPIPE_WR)
    or die "TestFarm::Engine: Cannot create synchronization pipe: $!";
  fcntl(WPIPE, &F_SETFD, &FD_CLOEXEC)  # Enable close-on-exec mode on read endpoint
    or die "TestFarm::Engine: Cannot setup synchronization pipe: $!";
  fcntl(WPIPE_WR, &F_SETFD, 0)  # Disable close-on-exec mode on write endpoint
    or die "TestFarm::Engine: Cannot setup synchronization pipe: $!";
  WPIPE->autoflush(1);

  # Spawn test engine through a pipe
  my $cmd = "testfarm-engine -wait ".fileno(WPIPE_WR)." -o ".$global_log;
  $pid = CORE::open(ENGINE, "|exec $cmd")
    or die "TestFarm::Engine: Cannot spawn test engine: $!";
  ENGINE->autoflush(1);

  CORE::close(WPIPE_WR);

  $SIG{CHLD} = \&sigchld;
  $SIG{PIPE} = \&sigpipe;
  $SIG{HUP} = \&sigterm;
  $SIG{QUIT} = \&sigterm;
  $SIG{TERM} = \&sigterm;

  STDOUT->autoflush(1);

  return $pid;
}


sub stop {
  $SIG{CHLD} = 'IGNORE';
  $SIG{PIPE} = 'IGNORE';
  $SIG{HUP} = 'IGNORE';
  $SIG{QUIT} = 'IGNORE';
  $SIG{TERM} = 'IGNORE';

  # Close engine process and synchronization pipe
  if ( defined $pid ) {
    $pid = undef;
    CORE::close(ENGINE);
    CORE::close(WPIPE);
  }
}


sub started {
  return (defined $pid) && ($pid > 0);
}

sub cmd {
  print ENGINE @_,"\n";
}

sub version {
  print ENGINE "version\n";
}

sub echo {
  print ENGINE "echo @_\n";
}

sub trig_def {
  if ( $#_ < 0 ) {
    print ENGINE "trig def\n";
  }
  else {
    my $id = shift;

    if ( $#_ < 0 ) {
      print ENGINE "trig def ".($id || '')."\n";
    }
    elsif ( (defined $id) && ($#_ > 0) ) {
      my $periph = shift || "";
      my $regex = shift;
      print ENGINE "trig def $id \@$periph \"", $regex, "\"\n";
    }
    else {
      print STDERR "TestFarm::Engine: Missing argument for trig_def(<id>, <periph>, <regex>)\n";
    }
  }
}

sub trig_undef {
  print ENGINE "trig undef @_\n";
}

sub trig_clear {
  print ENGINE "trig clear @_\n";
}

sub trig_info($) {
  my $id = shift;
  return "" unless ( (defined $id) && ($id !~ /^\s*$/ ) );

  print ENGINE "trig info $id\n";
  chomp(my $line = <WPIPE>);

  return $line;
}

sub trig_count($) {
  my $id = shift;
  return -1 unless ( (defined $id) && ($id !~ /^\s*$/ ) );

  print ENGINE "trig count $id\n";
  chomp(my $line = <WPIPE>);

  return $line;
}

sub timeout {
  my $ret = $old_timeout;

  if ( $#_ < 0 ) {
    print ENGINE "timeout\n";
    return $ret;
  }

  my $time = "@_";
  $time =~ s/^\s*//;
  $time =~ s/\s*$//;

  return $ret if ( $time eq "" );

  if ( $time =~ /^\d+\s*(ms|s|min|h)?$/ ) {
    print ENGINE "timeout $time\n";
    $old_timeout = $time;
  }
  else {
    print STDERR "TestFarm::Engine: Syntax error in timeout value '$time'\n";
  }

  return $ret;
}

sub wait {
  my $timeout = "";

  if ( ($#_ > 0) && (defined $_[$#_]) && ($_[$#_] =~ /^(\d+)\s*(ms|s|min|h)?$/) ) {
    $#_--;
    $timeout = "-$1$2 ";
  }

  my $args = "";
  foreach ( @_ ) {
    next unless defined $_;
    next if /^\s*$/;
    $args .= "\"$_\" ";
  }

  print ENGINE "wait ", $timeout, $args,"\n";

  my $line = <WPIPE>;
  if ( defined $line ) {
    $line =~ s/^\s*//;
    $line =~ s/\s*$//;
  }

  return $line;
}

sub wait_with {
  my($periph, $tag, $pattern) = @_;
  print ENGINE "trig def WAIT \@$periph $tag +$pattern\n";
  print ENGINE "wait WAIT\n";
  my $line = <WPIPE>;
  print ENGINE "trig undef WAIT\n";

  return $line;
}

sub periph {
  my($id, $addr, $flags, $blabla) = @_;
  print ENGINE "periph $id \"$addr\" $flags \"$blabla\"\n";
  chomp(my $line = <WPIPE>);
  return $line;
}

sub open {
  my($id) = shift;
  print ENGINE "open $id\n";
}

sub close {
  return unless started();
  my($id) = shift;
  print ENGINE "close $id\n";
  my $line = <WPIPE>;
}


sub ReportDir(;$) {
  my $dir = shift;
  if ( defined $dir ) {
    if ( $dir =~ /^\s*$/ ) {
      $report_dir = undef;
    }
    else {
      $report_dir = $dir;
    }
  }

  # Return a directory relative to the CWD
  my $cwd = getcwd();
  my $ret = $report_dir;
  $ret =~ s/^$cwd\/*//;

  return $ret;
}


sub GlobalLog() {
  return $global_log;
}


sub LocalLog(;*) {
  my $local = shift;
  if ( defined $local ) {
    $local_log = $local;
    print ENGINE "local $local_log\n";
    print "Local log file: $local_log\n";
  }
  return $local_log;
}


sub result {
  return LocalLog(@_);
}


sub case {
  if ( $#_ == 0 ) {
    $case_name = $_[0];
    print ENGINE "case $case_name\n";
  }
  return $case_name;
}

sub TestCase() {
  return $case_name;
}


sub done {
  return unless started();
  print ENGINE "done\n";
  my $line = <WPIPE>;
}


sub do_with {
  my($target, $cmd) = @_;
  print ENGINE "do \@$target '$cmd'\n";
}

sub verdict {
  print ENGINE "verdict @_\n";
}

BEGIN {
}

END {
  stop();
}

1;
