#$CRITICITY MAJOR

## Created: Tue Jul 24 15:02:59 2007
## $Revision: 657 $
## $Date: 2007-07-25 09:08:42 +0200 (mer., 25 juil. 2007) $

package Entry;

use TestFarm::Exec;
use TestSystem;

sub TEST {
  my $text = "Hello World";

  print "Enter 2 lines of text in the input area and press [PASSED]\n";
  print "The input text appears in blue in both the output area and the test report\n";
  ($verdict, $text) = TestFarm::Exec::ManualInput();
  print "Entered text = '$text'\n";
  return $verdict;
}

1;
