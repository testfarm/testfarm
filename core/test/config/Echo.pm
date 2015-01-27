##
## Sample Test Interface Library
##

## $Revision: 1236 $
## $Date: 2013-06-28 14:38:02 +0200 (ven., 28 juin 2013) $

package Echo;
@ISA = qw ( TestFarm::Interface );

use TestFarm::Interface;

my %TEMPLATE = (
  'DESCRIPTION' => "Dummy Test Interface",
  'COMMAND'     => "/var/testfarm/lib/echo.pl",
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

1;

