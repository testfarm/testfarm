#$CRITICITY MAJOR

## Created: Tue Jul 24 15:12:58 2007
## $Revision: 771 $
## $Date: 2007-10-11 15:56:44 +0200 (jeu., 11 oct. 2007) $

package Verdict::Inconclusive;

use TestFarm::Exec;
use TestSystem;

sub TEST {
  print "Press [INCONCLUSIVE] and check the test case verdict\n";
  ($verdict) = TestFarm::Exec::ManualInput();
  return $verdict;
}

1;
