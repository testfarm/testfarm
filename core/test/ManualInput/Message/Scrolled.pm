#$CRITICITY MAJOR

## Created: Tue Jul 24 15:12:03 2007
## $Revision: 771 $
## $Date: 2007-10-11 15:56:44 +0200 (jeu., 11 oct. 2007) $

package Message::Scrolled;

use TestFarm::Exec;
use TestSystem;

sub TEST {
  my $text = "Hello World";

  for (my $i = 1; $i < 100; $i++) {
    printf "Line #%d\n", $i;
  }

  print "Press [PASSED] if the message \"$text\" appears below,\n";
  print "and the output area is scrolled\n";
  ($verdict) = TestFarm::Exec::ManualInput($text);
  return $verdict;
}

1;
