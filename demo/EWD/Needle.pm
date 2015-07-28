package Needle;

use Math::Trig;
use Math::Trig ':pi';
use Math::Trig ':radial';

use TestFarm::Trig;
use EWDsystem;


# Accepted config parameters with their default values
my %DEFAULT_PARAMS = (
  X => undef,          # Center position (X)
  Y => undef,          # Center position (Y)
  RMIN => 10,          # Min radius
  RMAX => undef,       # Max radius
  RSTEP => 1,          # Radial sampling step
  THETA => '140 360',  # Angular range
  SCALE => '0 100',    # Angular scaling
  COLOR => undef,      # Spot color
  FUZZ  => 0,          # Spot color fuzz
  PITCH => 3,          # Spot edge length in pixels
);


%PATTERNS = ();
%LINKS = ();


sub Setup($$) {
  my $name = shift;
  my $PARAMS = shift;

  Destroy($name);

  # Check no parameter is left undefined
  my $err = 0;
  foreach my $key ( keys %$PARAMS ) {
    unless ( defined $$PARAMS{$key} ) {
      print STDERR "$name: Parameter '$key' needs to be defined\n";
      $err++;
    }
  }
  return -1 if ( $err > 0 );

  # Generate spot constellation
  my $pitch = $$PARAMS{PITCH};

  my $rho_min = int($$PARAMS{RMIN} / $pitch);
  my $rho_max = int($$PARAMS{RMAX} / $pitch);
  my $rho_inc = $$PARAMS{RSTEP};

  my $theta_inc = 1 / $$PARAMS{RMAX};  # Perimeter in pixels
  my $ntheta = POSIX::ceil(abs(log($theta_inc)));  # theta precision in number of decimals

  my @theta = split /\s+/, $$PARAMS{THETA};
  my $theta1 = deg2rad($theta[0], 1);
  my $theta2 = deg2rad($theta[1], 1);
  my $theta_inc_sign = +1;
  if ( $theta1 > $theta2 ) {
    $theta_inc = (- $theta_inc);
    $theta_inc_sign = -1;
  }

  my $theta_step = 0; # Angular step number
  my $theta_n = 0;
  my $theta_prev = undef;

  print "$name: Center = +$$PARAMS{X}+$$PARAMS{Y}\n";
  print "$name: Radius = $$PARAMS{RMIN}..$$PARAMS{RMAX} by steps of $$PARAMS{RSTEP} spots\n";
  printf "$name: Angle = %.".$ntheta."f..%.".$ntheta."f rad. by increments of %.".$ntheta."f rad.\n", $theta1, $theta2, $theta_inc;
  print "$name: Spot = square of $pitch pixels, color=$$PARAMS{COLOR} fuzz=$$PARAMS{FUZZ}\n";

  my $source = "image=$$PARAMS{COLOR}:$pitch fuzz=$$PARAMS{FUZZ} appear disappear retrigger brief";
  my $geom = $pitch.'x'.$pitch;

  my %xytab = ();
  my @ids = ();

  my $rho_n = 0;

  for (my $theta = $theta1; ($theta_inc_sign * ($theta - $theta2)) < 0; $theta += $theta_inc) {
    if ( $theta_step > $theta_n ) { $theta_n = $theta_step };

    my $rho_step = 0;

    for (my $rho = $rho_min; $rho <= $rho_max; $rho += $rho_inc) {
      if ( $rho_step > $rho_n ) { $rho_n = $rho_step };

      my ($xx, $yy) = cylindrical_to_cartesian($rho, $theta, $z);
      my $x = sprintf("%.0f", $xx);
      my $y = sprintf("%.0f", $yy);

      my $xy = sprintf("%+d%+d", $x, $y);
      unless ( exists $xytab{$xy} ) {
	my $id = $$PARAMS{NAME}.'_'.$theta_step.'R'.$rho_step;
	# $id .= sprintf('_%.'.$ntheta.'f', $theta);
	$xytab{$xy} = $id;
	push @ids, $id;

	my $window = sprintf($geom.'%+d%+d', ($$PARAMS{X}+$x*$pitch), ($$PARAMS{Y}+$y*$pitch));
	$PATTERNS{$id} = [ $name, "window=$window $source" ];
      }

      $rho_step++;
    }

    if ( (defined $theta_prev) && ($theta_prev != $theta) ) {
      $theta_step++;
    }
    $theta_prev = $theta;
  }

  print "$name: Constellation of ".($#ids+1)." spots\n";
  print "$name: Radius range = 0..".$rho_n."\n";
  print "$name: Angle range = 0..".$theta_n."\n";

  #print STDERR "-- link $name @ids\n";
  $VISU->link($name, @ids);

  my @argv = ($$PARAMS{NAME}, $rho_n, $theta_n, '"'.$$PARAMS{SCALE}.'"');
  my $ret = $VISU->connect($name, "./Needle.pl @argv");

  if ( $ret == 0 ) {
    $LINKS{$name} = 1;
    $VISU->sync();

    foreach my $id ( @ids ) {
      my $pattern = $PATTERNS{$id};
      $VISU->match_add($id, $$pattern[1]);
    }

    $VISU->sync();
  }

  return $ret;
}


sub Destroy($) {
  my $name = shift;

  if ( exists $LINKS{$name} ) {
    $VISU->kill($name);
    $VISU->unlink($name);
    delete $LINKS{$name};
  }

  my $pref = $name.'_';
  foreach my $id ( keys %PATTERNS ) {
    my $pattern = $PATTERNS{$id};
    if ( $$pattern[0] eq $name ) {
      $VISU->match_remove($id);
      delete $PATTERNS{$id};
    }
  }

  $VISU->sync();
}


sub DestroyAll() {
  foreach my $name ( keys %LINKS ) {
    $VISU->kill($name);
    $VISU->unlink($name);
  }

  foreach my $id ( keys %PATTERNS ) {
    $VISU->match_remove($id);
  }

  %LINKS = ();
  %PATTERNS = ();

  $VISU->sync();
}


sub SendCommand($$) {
  my $name = shift;
  my $msg = shift;

  $VISU->send($name, '++ '.$msg);
}

sub Clear($) {
  my $name = shift;
  SendCommand($name, 'clear');
}

sub Calibrate {
  my $name = shift;
  SendCommand($name, "calibrate @_");
}

sub Show($) {
  my $name = shift;

  my $regex = '^\d+\s+'.$name.'\s+VALUE=';
  my $trig = TrigDef($VISU, $name, $regex);
  SendCommand($name, 'show');

  my $ret = TrigWaitInfo($name, '500ms');
  if ( $ret ) {
    $ret =~ s/^\d+\s+\S+\s+//;
  }

  TrigUndef($name);

  return $ret;
}

1;
