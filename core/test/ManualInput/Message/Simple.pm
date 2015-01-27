#$CRITICITY MAJOR

## Created: Tue Jul 24 12:32:17 2007
## $Revision: 657 $
## $Date: 2007-07-25 09:08:42 +0200 (mer., 25 juil. 2007) $

package Message::Simple;

use TestFarm::Exec;
use TestSystem;

sub TEST {
  my $text = "Hello World";

  print "Press [PASSED] if the message \"$text\" appears below:\n";
  ($verdict) = TestFarm::Exec::ManualInput($text);
  return $verdict;
}

1;
