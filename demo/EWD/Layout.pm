#$CRITICITY MAJOR
#$ABORT_IF_FAILED

## Created: Mon Mar 17 18:36:02 2008
## $Revision: 43 $
## $Date: 2008-03-27 14:41:06 +0100 (Thu, 27 Mar 2008) $

package Layout;

use EWDsystem;
use Color;


sub TEST {
  my $verdict = PASSED;

  #
  # Consider 3 Regions of equal size:
  # PWR1 (left), FLAGS (center), PWR2 (right)
  #
  my @regions = ( 'PWR1', 'FLAGS', 'PWR2' );
  my $width = $VISU->{WIDTH} / ($#regions + 1);
  my $height = $VISU->{HEIGHT};
  my $x = $VISU->{X};
  my $y = $VISU->{Y};

  # Set geometry of the whole VISU
  $VISU->{GEOMETRY} = $VISU->{WIDTH}.'x'.$VISU->{HEIGHT}.'+'.$x.'+'.$y;

  # Set geometry for each region
  my $xi = $x;
  foreach my $region ( @regions ) {
    my $geometry = $width.'x'.$height.'+'.$xi.'+'.$y;
    $VISU->{$region} = $geometry;
    print "$region Region geometry: $geometry\n";

    $xi += $width;
  }

  #
  # PWRx settings
  #
  $xi = $x;
  foreach my $region ( @regions ) {
    if ( $region =~ /^PWR/ ) {
      # Setup needle tracking parameters
      my %NEEDLE = (
        NAME   => $region,
        X      => $xi + ($width / 2),  # Center position (X)
        Y      => $y + ($height / 2),  # Center position (Y)
        RMIN   => 40,                  # Min radius
        RMAX   => 80,                  # Max radius
        RSTEP  => 3,                   # Radial sampling stride
        THETA  => '128 372',           # Angular range
        SCALE  => '-5 105',            # Angular scaling
        COLOR  => '#00F000',           # Spot color
        FUZZ   => '#1F0F1F',           # Spot color fuzz
        PITCH  => 3,                   # Spot edge size in pixels
      );

      print "$region Needle: +$NEEDLE{X}+$NEEDLE{Y}\n";
      $VISU->{$region.'_NEEDLE'} = \%NEEDLE;

      # Retrieve digits grey box
      my $border = 4;
      my $box = $VISU->pad('window='.$VISU->{$region}.' color='.Color::GREY.' fuzz=8 min='.(2*$border+10));
      if ( $box ) {
	$box =~ /^(\d+)x(\d+)\+(\d+)\+(\d+)/;
	my $w = $1 - (2*$border);
	my $h = $2 - (2*$border);
	my $x = $3 + $border;
	my $y = $4 + $border;

	if ( ($w >= 10) and ($h >= 10) ) {
	  $box = $w.'x'.$h.'+'.$x.'+'.$y;
	  print "$region Digits box: $box\n";
	}
	else {
	  print "IN_VERDICT $region Digit box: $box (less than 10pix)\n";
	  $verdict = FAILED;
	  $box = '';
	}
      }
      else {
	print "IN_VERDICT Digits grey box not found\n";
	$verdict = FAILED;
      }

      $VISU->{$region.'_DIGITS'} = $box;
    }

    $xi += $width;
  }

  return $verdict;
}

1;
