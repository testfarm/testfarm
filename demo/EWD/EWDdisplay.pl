#!/usr/bin/perl -w

package EWD::Needle;

use warnings;
use strict;

use FileHandle;
use Glib qw/TRUE FALSE/;
use Gtk2;
use Cairo;
use Math::Trig;
use Math::Trig ':pi';
use Math::Trig ':radial';

use Glib::Object::Subclass
  Gtk2::DrawingArea::,
  signals => {
    expose_event => \&expose,
  };


my $UDP_PORT = 6789;

my $DTHETA = 55;
my $THETA1 = 140;
my $THETA2 = $THETA1 + 4*$DTHETA;

my $BLINK_TIME = 500;
my $BLINK_STATE = 1;


sub min { return ($_[0] < $_[1] ? $_[0] : $_[1]); }


sub draw {
  my $self = shift;
  my $cr = $self->{cr};

  return FALSE unless $cr;

  my $theta_scale = ($THETA2 - $THETA1) / 100.0;
  my $red_theta = $THETA1 + $theta_scale * $self->{red_limit};
  my $max_theta = $THETA1 + $theta_scale * $self->{alm_limit};

  my $width  = $self->allocation->width;
  my $height = $self->allocation->height;
  my $min = min($width, $height);
  $cr->scale($min, $min);
  $cr->translate(0.5, 0.5);

  # Default line/font settings
  $cr->set_line_width($self->{line_width});
  $cr->set_font_size(0.12);
  $cr->select_font_face ('Nimbus Mono L', 'normal', 'bold');

  # Paint background in black
  $cr->set_source_rgb (0.0, 0.0, 0.0);
  $cr->paint;

  # Set default color
  $cr->set_source_rgb (1.0, 1.0, 0.8);

  # Draw indicator arc
  $cr->arc (0, 0, $self->{radius}, deg2rad($THETA1), deg2rad($red_theta));
  $cr->stroke;

  $cr->save;
  $cr->set_source_rgb (1.0, 0.0, 0.0);
  $cr->set_line_width($self->{line_width}*2);
  $cr->arc (0, 0, $self->{radius} - ($self->{line_width}/2), deg2rad($red_theta), deg2rad($THETA2));
  $cr->stroke;
  $cr->restore;

  # Analog indication dashes
  $cr->set_line_cap('round');

  my ($x, $y);
  my $rho1 = $self->{radius} - ($self->{radius} / 10);
  my $rho2 = $self->{radius};

  for (my $theta = $THETA1; $theta <= $THETA2; $theta += $DTHETA) {
    $cr->save;

    if ( $theta >= $red_theta ) {
      $cr->set_source_rgb (1.0, 0.0, 0.0);
    }

    ($x, $y) = cylindrical_to_cartesian($rho1, deg2rad($theta));
    $cr->move_to($x, $y);

    ($x, $y) = cylindrical_to_cartesian($rho2, deg2rad($theta));
    $cr->line_to($x, $y);

    $cr->stroke;
    $cr->restore;
  }

  # Max limit
  $cr->save;
  $cr->set_source_rgb (1.0, 0.0, 0.0);

  ($x, $y) = cylindrical_to_cartesian($rho2, deg2rad($max_theta));
  $cr->move_to($x, $y);

  my $rho3 = $rho2 + ($self->{radius} / 10);
  ($x, $y) = cylindrical_to_cartesian($rho3, deg2rad($max_theta));
  $cr->line_to($x, $y);

  $cr->stroke;
  $cr->restore;

  # Analog indication numbers
  $cr->save;

  my $txt = "0";
  ($x, $y) = cylindrical_to_cartesian($rho1-0.01, deg2rad($THETA1));
  $cr->move_to($x, $y);
  $cr->show_text ($txt);

  $txt = "5";
  my $extents = $cr->text_extents ($txt);
  ($x, $y) = cylindrical_to_cartesian($rho1-$extents->{height}-0.04, deg2rad($THETA1+2*$DTHETA));
  $cr->move_to($x-($extents->{width}), $y);
  $cr->show_text ($txt);

  $txt = "10";
  $extents = $cr->text_extents ($txt);
  ($x, $y) = cylindrical_to_cartesian($rho1-$extents->{width}-0.05, deg2rad($THETA2));
  $cr->move_to($x, $y+($extents->{height}/2));
  $cr->show_text ($txt);

  $cr->stroke;
  $cr->restore;

  # Digital area borders
  $cr->save;

  $cr->set_source_rgb (0.5, 0.55, 0.45);
  $cr->rectangle(-0.1, 0.15, 0.55, 0.2);

  $cr->stroke;
  $cr->restore;

  # Get and Clamp indicator value
  my $value = $self->{value};
  if ( defined $value ) {
    if ( $value < -5 ) {
      $value = -5;
    }
    elsif ( $value > 105 ) {
      $value = 105;
    }
  }

  $self->{blinking} = (defined $value) && ($value >= $self->{alm_limit});

  return TRUE unless defined $value;

  # Show needle
  $cr->save;

  $cr->set_line_cap('round');
  $cr->set_source_rgba (0.0, 1.0, 0.0, 0.9);
  $cr->set_line_width($self->{line_width}*2);
  $cr->move_to(0,0);

  ($x, $y) = cylindrical_to_cartesian($rho2, deg2rad($THETA1 + ($value * $theta_scale)));
  $cr->line_to($x, $y);

  $cr->stroke;
  $cr->restore;

  # Show digital indication
  unless ( $self->{blinking} && (! $BLINK_STATE) ) {
    $cr->save;

    if ( $value < $self->{red_limit} ) {
      $cr->set_source_rgb (0.0, 1.0, 0.0);
    }
    else {
      $cr->set_source_rgb (1.0, 0.0, 0.0);
    }

    $cr->move_to(-0.05, 0.31);

    my $txt1 = sprintf("%5.1f", $value);
    $txt1 =~ s/\.(.+)$//;
    my $txt2 = $1;

    $cr->set_font_size(0.17);
    $cr->show_text ($txt1);

    $cr->set_font_size(0.13);
    $cr->show_text ('.'.$txt2);

    $cr->stroke;
    $cr->restore;
  }

  return TRUE;
}


sub refresh {
  my $self = shift;
  my $value = shift;

  if ( defined $value ) {
    $value = undef unless ( $value =~ /^-?\d+(\.\d+)?$/ );
    $self->{value} = $value;
  }

  my $alloc = $self->allocation;
  my $rect = Gtk2::Gdk::Rectangle->new(0, 0, $alloc->width, $alloc->height);
  $self->window->invalidate_rect($rect, FALSE);
}


sub expose {
  my ($self, $event) = @_;

  my $cr = Gtk2::Gdk::Cairo::Context->create($self->window);
  $cr->rectangle ($event->area->x,
		  $event->area->y,
		  $event->area->width,
		  $event->area->height);
  $cr->clip;
  $self->{cr} = $cr;

  $self->draw;

  return FALSE;
}


sub INIT_INSTANCE {
  my $self = shift;

  $self->{line_width} = 0.02;
  $self->{radius}     = 0.42;

  $self->{blink_time} = 500;
  $self->{red_limit} = 75;
  $self->{alm_limit} = 90;
}

sub FINALIZE_INSTANCE {
  my $self = shift;

  Glib::Source->remove($self->{timeout}) if $self->{timeout};
}

1;


package EWD::Tags;

use warnings;
use strict;

use Glib qw/TRUE FALSE/;
use Gtk2;
use Cairo;

use Glib::Object::Subclass
  Gtk2::DrawingArea::,
  signals => {
    expose_event => \&expose,
  };


sub min { return ($_[0] < $_[1] ? $_[0] : $_[1]); }


sub show_text_centered {
  my $self = shift;
  my $cr = $self->{cr};

  my $txt = shift;
  my $vpos = shift;

  my $extents = $cr->text_extents ($txt);
  $cr->move_to(0.5-($extents->{width}/2), $vpos);
  $cr->show_text ($txt);
}


sub draw {
  my $self = shift;
  my $cr = $self->{cr};

  my $width  = $self->allocation->width;
  my $height = $self->allocation->height;
  my $min = min($width, $height);
  $cr->scale($min, $min);

  # Paint background in black
  $cr->set_source_rgb (0.0, 0.0, 0.0);
  $cr->paint;

  # Set default font size
  $cr->set_font_size(0.14);
  $cr->select_font_face ('Sans', 'normal', 'normal');

  # Show mode tag
  $cr->set_source_rgb (1.0, 0.8, 0.0);
  $self->show_text_centered ($self->{value}, 0.2);

  # Show N1 tag
  $cr->set_source_rgb (1.0, 1.0, 0.8);
  $self->show_text_centered ('N1', 0.5);

  # Show % tag
  $cr->set_source_rgb (0.0, 0.0, 1.0);
  $self->show_text_centered ('%', 0.7);

  $cr->stroke;

  return FALSE unless $cr;
}


sub expose {
  my ($self, $event) = @_;

  my $cr = Gtk2::Gdk::Cairo::Context->create($self->window);
  $cr->rectangle ($event->area->x,
		  $event->area->y,
		  $event->area->width,
		  $event->area->height);
  $cr->clip;
  $self->{cr} = $cr;

  $self->draw;

  return FALSE;
}


sub refresh {
  my $self = shift;
  my $value = shift;

  if ( defined $value ) {
    $self->{value} = $value;
  }

  my $alloc = $self->allocation;
  my $rect = Gtk2::Gdk::Rectangle->new(0, 0, $alloc->width, $alloc->height);
  $self->window->invalidate_rect($rect, FALSE);
}


sub INIT_INSTANCE {
  my $self = shift;

  $self->{value} = 'TOGA';
}

sub FINALIZE_INSTANCE {
  my $self = shift;
}

1;


package main;

use IO::Socket;
use Glib qw/TRUE FALSE/;
use Gtk2 '-init';


my $opt_grab = 0;

my %visu = ();

my $SOCKET;


sub grab {
  my $window = shift;

  return unless $opt_grab;

  $window->show;
  while ( Gtk2->events_pending ) { Gtk2->main_iteration() }

  my ($width, $height) = $window->window->get_size();
  my $pixbuf = Gtk2::Gdk::Pixbuf->new('rgb', 0, 8, $width, $height);
  $pixbuf->get_from_drawable($window->window, undef, 0, 0, 0, 0, $width, $height);
  $pixbuf->save("toto.png", 'png');

  if ( rename('toto.png', 'frame.png') == 0 ) {
    print STDERR "rename: $!\n";
  }
}


sub blink {
  my $window = shift;

  $BLINK_STATE = $BLINK_STATE ? 0:1;

  my $changed = 0;

  foreach my $id ( keys %visu ) {
    if ( $visu{$id}->{blinking} ) {
      $visu{$id}->refresh;
      $changed = 1;
    }
  }

  grab($window) if $changed;

  return TRUE;
}


sub input {
  my $line = shift;
  my $changed = 0;

  chomp($line);
  #print STDERR "-- INPUT: '$line'\n";

  $line =~ s/^(.+)=//;
  my $id = $1;

  if ( exists $visu{$id} ) {
    print "$id.value=$line\n";
    $visu{$id}->refresh($line);
    $changed = 1;
  }

  return $changed;
}


sub stdin_input {
  my ($fd, $cond, $window) = @_;

  my $changed = 0;

  my $line;
  while ( defined ($line = STDIN->gets()) ) {
    $changed |= input($line);
  }

  grab($window) if $changed;

  return TRUE;
}


sub socket_input {
  my ($fd, $cond, $window) = @_;

  my $changed = 0;

  my $line;
  while ( $SOCKET->recv($line, 1024) ) {
    $changed |= input($line);
  }

  grab($window) if $changed;

  return TRUE;
}


# Create service socket if UDP port is defined
if ( defined $UDP_PORT ) {
  $SOCKET = IO::Socket::INET->new(LocalPort => $UDP_PORT,
				  Proto => 'udp');
  unless ( $SOCKET ) {
    print STDERR "Cannot create UDP service socket at *:UDP_PORT: $!\n";
    exit(1);
  }
}

my $window = Gtk2::Window->new('toplevel');
$window->set_default_size(600,200);
$window->signal_connect(destroy => sub { Gtk2->main_quit; });
$window->set_title('EWD Display');

my $hbox = Gtk2::HBox->new(TRUE, 0);
$window->add($hbox);

my $w;

$w = EWD::Needle->new;
$hbox->add($w);
$visu{PWR1} = $w;

$w = EWD::Tags->new;
$hbox->add($w);
$visu{FLAGS} = $w;

$w = EWD::Needle->new;
$hbox->add($w);
$visu{PWR2} = $w;

$window->show_all;


my $timeout = Glib::Timeout->add($BLINK_TIME, \&blink, $window);

#STDIN->blocking(0);
#my $stdin_watch = Glib::IO->add_watch(fileno(STDIN), 'in', \&stdin_input, $window);

if ( defined $SOCKET ) {
  $SOCKET->blocking(0);
  my $socket_watch = Glib::IO->add_watch(fileno($SOCKET), 'in', \&socket_input, $window);
}

Gtk2->main;

0;
