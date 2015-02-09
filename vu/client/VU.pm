##
## TestFarm Virtual User Interface
## Interface Library
##
## (C) Basil Dev 2006
##
## $Revision: 991 $
## $Date: 2008-03-21 17:06:20 +0100 (ven., 21 mars 2008) $
##

package TestFarm::VU;
@ISA = qw ( TestFarm::InterfaceInput );

use TestFarm::InterfaceInput;

my %TEMPLATE = (
  'DESCRIPTION' => "TestFarm Virtual User Interface",
);


#
# new( [<Name>,] [(PROC|TCP|SERIAL)://]<Address> );
#
sub new {
  my $proto = shift;
  my $class = ref($proto) || $proto;

  my $self = TestFarm::Interface->new(\%TEMPLATE, @_);
  $self->open();
  bless($self, $class);

  $self->capture_geometry();

  return $self;
}

sub status {
  my $self = shift;
  $self->command('status');
}

sub mode {
  my $self = shift;
  $self->command("mode @_");
}

sub sleep {
  my $self = shift;
  $self->command("sleep @_");
}

sub capture_refresh(;$) {
  my $self = shift;
  my $info = $self->command("capture refresh @_", '^\d+\s+CAPTURE\s+REFRESH\s+', '2s');
  if ( $info ) {
    $info =~ s/^\d+\s+CAPTURE\s+REFRESH\s+//;
  }
  return $info;
}

sub frame_refresh(;$) {
  my $self = shift;
  $self->capture_refresh(@_);
}

sub capture_geometry(;$) {
  my $self = shift;
  my $info = $self->command("capture geometry @_", '^\d+\s+CAPTURE\s+GEOMETRY\s+', '2s');
  if ( $info ) {
    $info =~ s/^\d+\s+CAPTURE\s+GEOMETRY\s+//;
    $info =~ /^(\d+)x(\d+)\+(\d+)\+(\d+)/;
    $self->{WIDTH} = $1;
    $self->{HEIGHT} = $2;
    $self->{X} = $3;
    $self->{Y} = $4;
    printf $self->{ID}." ROI: %dx%d+%d+%d\n", $self->{WIDTH}, $self->{HEIGHT}, $self->{X}, $self->{Y};
  }
  return $info;
}

sub frame_geometry(;$) {
  my $self = shift;
  $self->capture_geometry(@_);
}

sub kp_press($) {
  my $self = shift;
  my $keysym = shift;
  $self->command("kp press $keysym");
}

sub kp_release($) {
  my $self = shift;
  my $keysym = shift;
  $self->command("kp release $keysym");
}

sub kp_dial {
  my $self = shift;
  $self->command("kp dial @_");
}

sub kp_type($) {
  my $self = shift;
  my $text = shift;
  $self->command("kp type \"$text\"");
}

sub kp_delay {
  my $self = shift;
  $self->command("kp delay @_");
}

sub mouse_move {
  my $self = shift;
  $self->command("mouse move @_");
}

sub mouse_press {
  my $self = shift;
  $self->command("mouse press @_");
}

sub mouse_release {
  my $self = shift;
  $self->command("mouse release @_");
}

sub mouse_click {
  my $self = shift;
  $self->command("mouse click @_");
}

sub mouse_doubleclick {
  my $self = shift;
  $self->command("mouse click *2 @_");
}

sub mouse_scroll {
  my $self = shift;
  $self->command("mouse scroll @_");
}

sub mouse_delay {
  my $self = shift;
  $self->command("mouse delay @_");
}

sub match_add {
  my $self = shift;
  $self->command("match add @_");
}

sub match_reset {
  my $self = shift;
  $self->command("match reset @_");
}

sub match_remove {
  my $self = shift;
  $self->command("match remove @_");
}

sub grab {
  my $self = shift;
  my $info = $self->command("grab @_", '^\d+\s+GRAB\s+', '5s');

  if ( $info ) {
    $info =~ s/^\d+\s+GRAB\s+//;
  }

  return $info;
}

sub pad_add {
  my $self = shift;
  my $id = shift;

  my $regex = '^\d+\s+PAD\s+'.$id.'\s+';
  return $self->command("pad add $id @_", $regex, '500ms');
}

sub pad_remove {
  my $self = shift;
  my $id = shift;
  $self->command("pad remove $id");
}

sub pad_show {
  my $self = shift;
  my $id = shift;
  $self->command("pad show $id");
}

sub pad {
  my $self = shift;
  my $id = $self->{ID}.'_PAD';
  my $ret = $self->pad_add($id, @_);
  $self->pad_remove($id);

  if ( $ret ) {
    $ret =~ s/^\d+\s+PAD\s+\S+\s+frame=\S+\s*//;
  }

  return $ret;
}

1;
