#!/usr/bin/perl

use IO::Handle;
use Record;


# Construct the version id
my $version = '$Revision: 38 $ ';  # SVN puts the right value here
$version =~ s/[\s\$:]//g;
$version =~ s/Revision//;
$version = 0 if ( $version eq "" );

# Disable i/o buffering
STDOUT->autoflush(1);
STDIN->autoflush(1);

# Retrieve program arguments
my $NAME = $ARGV[0];
my $RHO_N = $ARGV[1];
my $THETA_N = $ARGV[2];
my $SCALE = $ARGV[3];

my $pref = $NAME.'_';

print STDERR "$NAME: RHO=0..$RHO_N THETA=0..$THETA_N SCALE=$SCALE\n";

# Compute scaling coefficients
my @scale_x = (0, $THETA_N);
my @scale_y = split /\s+/, $SCALE;
my $scale_k = 1;
my $scale_ofs = 0;

compute_scaling();

# Init position recording
my $position = undef;
my @tab = ();

Record::Init($NAME, \&process_value, \&calibrate);
Record::Process();


sub compute_scaling {
  $scale_k = ($scale_y[1] - $scale_y[0]) / ($scale_x[1] - $scale_x[0]);
  $scale_ofs = $scale_y[1] - ($scale_k * $scale_x[1]);
  printf "SCALING position=(@scale_x) value=(@scale_y) : K=%.3f OFS=%.3f\n", $scale_k, $scale_ofs;
}


sub calibrate {
  my $index = shift;
  my $value = shift;

  printf "CALIBRATE position[$index]=%.3f value[$index]=%.3f\n", $position, $value;

  $scale_x[$index] = $position;
  $scale_y[$index] = $value;
  compute_scaling();
}


sub process_value {
  my $id = shift;
  my $action = shift;

  return unless $id =~ s/^$pref//;
  return unless $id =~ /^(\d+)R(\d+)/;
  my $theta = $1;
  my $rho = $2;

  my $list = $tab[$rho];
  unless ( defined $list ) {
    my %list0 = ();
    $list = \%list0;
    $tab[$rho] = $list;
  }

  if ( $action eq 'APPEAR' ) {
##    $tab[$rho] = $theta;
    $$list{$theta} = 1;
  }
  else {
##    if ( $theta == $tab[$rho] ) {
##      $tab[$rho] = undef;
##    }
    delete $$list{$theta};
  }

  $position = 0;
  my $n = 0;
  my $str = '';
  for (my $rho = 0; $rho <= $#tab; $rho++) {
    my $list = $tab[$rho];

    $str .= ' ' if ( $str ne '' );
    if ( defined $list ) {
      foreach ( keys %$list ) {
	$position += ($rho+1) * $_;
	$n += ($rho+1);
	$str .= sprintf("%3d,", $_);
      }
    }
    else {
      $str .= '---';
    }
  }

  if ( $n > 0 ) {
    $position /= $n;
  }
  else {
    $position = undef;
  }

  if ( $Record::debug ) {
    print "EVENT POSITION $id : $str : ".Record::show_value($position)."\n";
  }

  my $value = undef;
  if ( defined $position ) {
    $value = $scale_ofs + ($scale_k * $position);
  }

  return $value;
}
