#$CRITICITY MAJOR

## Created: Tue Jul 24 15:13:11 2007
## $Revision: 771 $
## $Date: 2007-10-11 15:56:44 +0200 (jeu., 11 oct. 2007) $

package Abort;

use TestFarm::Exec;
use TestSystem;

sub TEST {
  print "Press the [Abort] button when input area is activated,\n";
  print "and check for proper abort operation\n";
  ($verdict) = TestFarm::Exec::ManualInput("...Please Abort Now...");
  return $verdict;
}

1;
