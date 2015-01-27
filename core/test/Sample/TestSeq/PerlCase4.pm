#$DESCRIPTION Sample PERL Test Script
#$CRITICITY Catastrophic

## $Revision: 42 $
## $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $

package PerlCase4;

use TestSystem;

sub TEST {
  print "Hello from Test Case 4\n";
  sleep 1;
  return FAILED;
}

1;
