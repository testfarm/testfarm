##
## Test Interface Library for the periodic send command tool
##

## $Revision: 1087 $
## $Date: 2009-10-30 14:59:45 +0100 (ven., 30 oct. 2009) $

package PeriodicSend;
@ISA = qw ( TestFarm::Interface );

use TestFarm::Interface;

my %TEMPLATE = (
  DESCRIPTION => "Periodic Send Interface",
  COMMAND     => './periodic.pl',
);


#
# new(["Name",] ["PROC|TCP|SERIAL",] "Address");
#
sub new {
  my $proto = shift;
  my $class = ref($proto) || $proto;
  my $self = TestFarm::Interface->new(\%TEMPLATE, @_);  # Inherits the base interface class
  $self->open();
  return bless($self, $class);
}


sub tag {
  my $self = shift;
  $self->command("tag @_");
}


sub rate {
  my $self = shift;
  $self->command("rate @_");
}
