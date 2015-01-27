#$CRITICITY MAJOR

## Created: Tue Jul 24 15:28:17 2007
## $Revision: 771 $
## $Date: 2007-10-11 15:56:44 +0200 (jeu., 11 oct. 2007) $

package Message::Absent;

use TestFarm::Exec;
use TestSystem;

sub TEST {
  print "Press [PASSED] if no message is displayed when input area is activated\n";
  ($verdict) = TestFarm::Exec::ManualInput();
  return $verdict;
}

1;
