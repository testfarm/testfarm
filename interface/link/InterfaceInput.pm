##
## TestFarm
## Interface Input link management
##
## (C) Basil Dev 2006
##
## $Revision: 155 $
## $Date: 2006-07-19 18:57:53 +0200 (mer., 19 juil. 2006) $
##

package TestFarm::InterfaceInput;
@ISA = qw ( TestFarm::Interface );

use TestFarm::Trig;
use TestFarm::Interface;


#
# new( [<Name>,] [(PROC|TCP|SERIAL)://]<Address> );
#
sub new {
  my $proto = shift;
  my $class = ref($proto) || $proto;
  my $self = TestFarm::Interface->new(@_);
  return bless($self, $class);
}


sub connect {
  my $self = shift;

  my $args = "@_";
  $args =~ /^\s*(\S+)\s+/;
  my $link = $1;

  my $ret = 0;

  my $tok = TrigDef($self, 'CONNECT', "CONNECT +$link +");
  my $terr = TrigDef($self, 'ERROR', "CONNECT +\\*ERROR\\* +");
  my $tdone = TrigDef($self, 'DONE', "DONE +$link +");

  $self->command("connect $args");

  my $w = TrigWait("$tok | $terr", '5s');

  if ( $w eq $tok ) {
    # Ensure the new subprocess does not exit prematurely
    if ( TrigWait($tdone, '500ms') ) {
      print "IN_VERDICT The $link subprocess exited prematurely\n";
      $ret = 1;
    }
  }
  else {
    print "IN_VERDICT Unable to start $link subprocess\n";
    $ret = 1;
  }

  TrigUndef($tok, $terr, $tdone);

  return $ret;
}

sub kill {
  my $self = shift;
  $self->command("kill @_");
}

sub send {
  my $self = shift;
  $self->command("send @_");
}

sub link {
  my $self = shift;
  my $tag = shift;
  my $ret = undef;

  if ( defined $tag ) {
    $ret = $self->command("link $tag @_", " LINK +$tag", '2s');
  }
  else {
    $self->command('link');
  }
  return $ret ? 0 : 1;
}

sub unlink {
  my $self = shift;
  $self->command("unlink @_");
}

1;
