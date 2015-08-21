##
## TestFarm
## Serial Port PERL Library
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

package TestFarm::Serial;

use POSIX qw(:termios_h);
require 'asm/ioctls.ph';


#
# $obj = new(Device, [Speed], [Debug]);
#
sub new {
  my $proto = shift;
  my $class = ref($proto) || $proto;
  my $self = {};

  # Retrieve conctructor parameters
  $self->{device} = shift || '/dev/ttyS0';
  $self->{speed} = shift || 19200;
  $self->{debug} = shift;

  # Open serial device
  sysopen($self->{FD}, $self->{device}, 2) || die($self->{device}.": sysopen: $!\n");
  $self->{fileno} = fileno($self->{FD});

  # Configure serial port
  my $term = POSIX::Termios->new();
  $term->getattr($self->{fileno}) || die($self->{device}.": getattr: $!\n");

  # Note: equivalent shell command:
  # $ stty clocal -parenb -cstopb cs8 19200 raw -echo < $dev

  my $speed_id;
  if    ( $self->{speed} == 300 )   { $speed_id = &POSIX::B300  }
  elsif ( $self->{speed} == 600 )   { $speed_id = &POSIX::B600  }
  elsif ( $self->{speed} == 1200 )  { $speed_id = &POSIX::B1200  }
  elsif ( $self->{speed} == 2400 )  { $speed_id = &POSIX::B2400  }
  elsif ( $self->{speed} == 4800 )  { $speed_id = &POSIX::B4800  }
  elsif ( $self->{speed} == 19200 ) { $speed_id = &POSIX::B19200 }
  elsif ( $self->{speed} == 38400 ) { $speed_id = &POSIX::B38400 }
  else                              { $speed_id = &POSIX::B9600  }

  $term->setispeed($speed_id);
  $term->setospeed($speed_id);

  my $cflag = $term->getcflag();
  $cflag &= ~(&POSIX::CSIZE | &POSIX::CSTOPB | &POSIX::PARENB);
  $cflag |= &POSIX::CLOCAL | &POSIX::CS8 | &POSIX::CREAD;
  $term->setcflag($cflag);

  $term->setlflag(0);
  $term->setiflag(0);
  $term->setoflag(0);

  $term->setattr($self->{fileno}, &POSIX::TCSANOW) || die($self->{device}.": setattr: $!\n");
  if ( $self->{debug} ) {
    print STDERR $self->{device}.": Serial device open at ".$self->{speed}." bps\n";
  }

  return bless($self, $class);
}


sub DESTROY {
  my $self = shift;
  return unless defined $self->{FD};

  close($self->{FD});

  if ( $self->{debug} ) {
    print STDERR $self->{device}.": Serial device closed\n";
  }
}


sub read {
  my $self = shift;

  my $char = undef;
  my $ret = sysread($self->{FD}, $char, 1);
  return undef unless $ret;

  if ( $self->{debug} ) {
    printf STDERR "[DEBUG] ".$self->{device}.": read 0x%02X\n", ord($char);
  }

  return $char;
}


sub write {
  my $self = shift;
  my $char = shift;

  if ( $self->{debug} ) {
    printf STDERR "[DEBUG] ".$self->{device}.": write 0x%02X\n", ord($char);
  }

  my $ret = syswrite($self->{FD}, $char, 1);
  return undef unless $ret;

  return 1;
}


sub rts {
  my $self = shift;
  my $state = shift;
  my $v = "";

  if ( ! ioctl($self->{FD}, &TIOCMGET, $v) ) {
    if ( $self->{debug} ) {
      print STDERR "[DEBUG] ".$self->{device}.": ioctl(TIOCMGET): $!\n";
    }
    return undef;
  }

  my @a = unpack "l", $v;

  if ( $state == 0 ) {
    $a[0] |= 0x0004;
  }
  else {
    $a[0] &= ~0x0004;
  }

  $v = pack "l", @a;
  if ( ! ioctl($self->{FD}, &TIOCMSET, $v) ) {
    if ( $self->{debug} ) {
      print STDERR "[DEBUG] ".$self->{device}.": ioctl(TIOCMSET): $!\n";
    }
    return undef;
  }

  return 1;
}


1;
