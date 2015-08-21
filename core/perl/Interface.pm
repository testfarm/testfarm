##
## TestFarm
## Test Interface Library Base Class
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

package TestFarm::Interface;

use TestFarm::Engine;
use TestFarm::Trig;

my $count = 0;


#
# new( \%TEMPLATE, [<Name>,] [(PROC|TCP|SERIAL)://]<Address> )
#
sub new {
  my $proto = shift;
  my $class = ref($proto) || $proto;
  my $self = {};

  # Check number of arguments
  if ( ($#_ < 1) || ($#_ > 3) ) {
    $! = 255;
    die "TestFarm::Interface->new: Too ".(($#_ < 4) ? "few":"many")." arguments\n";
  }

  # Check the test engine is started
  if ( ! TestFarm::Engine::started() ) {
    $! = 254;
    die "TestFarm::Interface->new: Test Engine not started\n";
  }

  # Data initialization
  my $template = shift;

  my $caller;
  my $lvl = 0;
  do {
    $caller = caller($lvl++);
  } while ( $caller =~ /^TestFarm::Interface/ );

  my $type0 = $caller;
  $type0 =~ s/^TestFarm:://;

  $self->{TYPE} = $$template{TYPE} || uc($type0);
  $self->{DESCRIPTION} = $$template{DESCRIPTION} || $self->{TYPE};

  my $cmd = $$template{COMMAND} || lc($caller);
  $cmd =~ s/::/-/g;

  my $flags = $$template{FLAGS};
  $flags = "" unless defined $flags;

  # Get arguments [<Name>,] [(PROC|TCP|SERIAL)://]<Address>
  $self->{ADDRESS} = $_[$#_];
  if ( $self->{ADDRESS} =~ s/^(\S+):\/\/// ) {
    $self->{METHOD} = $1;
  }

  if ( $#_ > 0 ) {
    $self->{ID} = $_[0];
  }

  # Give a numbered name if none was given as an argument
  $count++;
  $self->{ID} ||= $self->{TYPE}.$count;
  $self->{NAME} = $self->{ID}; # For backward compatibility

  # Use the PROC method by default
  if ( defined $self->{METHOD}  ) {
    my $str1 = $self->{METHOD};
    if ( ($str1 ne "TCP") && ($str1 ne "SERIAL") && ($str1 ne "PROC") ) {
      $! = 252;
      die "TestFarm::Interface->new: Illegal interface access method '$str1'\n";
    }
  }
  else {
    $self->{METHOD} = "PROC";
  }

  if ( (! defined $cmd) && ($self->{METHOD} eq "PROC") ) {
    $! = 251;
    die "TestFarm::Interface->new: The PROC access method is not available for $self->{TYPE}\n";
  }

  # Declare peripheral interface to test engine
  my $address = $self->{ADDRESS};
  if ( $self->{METHOD} eq "PROC" ) {
    $address = "$cmd $address";
    $address =~ s/\s+$//;
  }
  TestFarm::Engine::periph($self->{ID}, $self->{METHOD}.'://'.$address, $flags, $self->{DESCRIPTION});

  # Init synchronization anti-aliasing counter
  $self->{SyncCount} = 1;
  $self->{Open} = 0;

  return bless($self, $class);
}


sub DESTROY {
  my $self = shift;
  $self->close() if $self->{Open};
}


sub id() {
  my $self = shift;
  return $self->{ID};
}


sub type() {
  my $self = shift;
  return $self->{TYPE};
}


sub trig_id(;$) {
  my $self = shift;

  my $pref = shift || "T";

  my $name = $pref.$self->{ID}.$self->{SyncCount};
  $self->{SyncCount} ++;

  return $name;
}


sub open(;$$) {
  my $self = shift;

  my $trig_regex = shift;
  my $trig_timeout = shift || '10s';
  my $trig = undef;

  if ( defined $trig_regex ) {
    $trig = TrigDef($self, undef, $trig_regex);
  }

  TestFarm::Engine::open($self->{ID});
  $self->{Open} = 1;

  if ( defined $trig ) {
    unless ( TrigWait($trig, $trig_timeout) ) {
      print STDERR "Interface ".$self->{ID}." initialization failed\n";
      $self->close();
    }
    TrigUndef($trig);
  }

  return $self->{Open};
}


sub close() {
  my $self = shift;
  TestFarm::Engine::close($self->{ID});
  $self->{Open} = 0;
}


sub command($;$$) {
  my $self = shift;
  my $cmd = shift;
  return undef unless defined $cmd;
  my $regex = shift;
  my $timeout = shift;

  my $trig = undef;
  my $info = undef;

  if ( defined $regex ) {
    my ($first) = split ' ', $cmd;
    $trig = TrigDef($self, "T".uc($first), $regex);
  }

  TestFarm::Engine::do_with($self->{ID}, $cmd);

  if ( defined $regex ) {
    if ( defined $timeout ) {
      $timeout =~ s/\s+//g;
    }

    $info = TrigWaitInfo($trig, $timeout);

    TrigUndef($trig);
  }

  return $info;
}


sub sync() {
  my $self = shift;
  my $message = "Synchronization ".$self->{SyncCount};
  my $timeout = shift || '20s';
  $self->{SyncCount} ++;
  return $self->command("echo $message", "ECHO +$message", $timeout);
}


#
# Undocumented and deprecated methods
#

sub name() {
  my $self = shift;
  return $self->{ID};
}


sub version() {
  my $self = shift;

  my $info = $self->command("version", "VERSION +", "2 s");
  if ( defined $info ) {
    if ( $info =~ /^\d+\s+VERSION\s+\S+\s+(\S+)/ ) {
      $info = $1;
    }
  }

  unless ( defined $info ) {
    print("WARNING: Unable to retrieve version of ", $self->{ID}, "\n");
  }

  return $info;
}


sub echo {
  my $self = shift;
  TestFarm::Engine::do_with($self->{ID}, "echo @_");
}

1;
