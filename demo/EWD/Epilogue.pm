#
# This file was generated automatically using the TestFarm Script Wizard
#
# DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT
#

  #####
  ##### PREAMBLE
  #####
#$LINE wiz/PREAMBLE.wizdef 0


package Epilogue;

use TestFarm::Trig;
use EWDsystem;

sub TEST {
  $verdict = PASSED;
  $criticity = 0;

  $OBJECT_GEOMETRY = undef;
  %OBJECT_TRIGGERS = ();

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

  $OBJECT_GEOMETRY = undef;


  select(undef, undef, undef, 0.5);

  return ($verdict, $criticity);
}

1; # Because it is a package...


