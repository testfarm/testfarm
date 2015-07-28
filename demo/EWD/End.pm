#
# This file was generated automatically using the TestFarm Script Wizard
#
# DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT
#

  #####
  ##### PREAMBLE
  #####
#$LINE wiz/PREAMBLE.wizdef 0


package End;

use File::Basename;
use TestFarm::Trig;
use EWDsystem;
use EWDdisplay;
use Color;
use Needle;
use Digits;

sub TEST {
  $verdict = PASSED;
  $criticity = 0;

  $OBJECT_WINDOW = undef;
  %OBJECT_TRIGGERS = ();

POSTAMBLE:
  #####
  ##### POSTAMBLE
  #####
#$LINE wiz/POSTAMBLE.wizdef 0



  #####
  ##### TRIG_OBJECT_CLEAR
  #####
#$LINE wiz/TRIG_OBJECT_CLEAR.wizdef 0









foreach ( keys %OBJECT_TRIGGERS ) {
  $VISU->match_remove($_);
  TrigUndef($_);
}
%OBJECT_TRIGGERS = ();



#$LINE wiz/POSTAMBLE.wizdef 3

  $OBJECT_WINDOW = undef;


  sleep 1;

  return ($verdict, $criticity);
}

1; # Because it is a package...


