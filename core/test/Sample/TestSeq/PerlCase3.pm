#$DESCRIPTION Sample PERL Test Script
#$CRITICITY Major

## $Revision: 42 $
## $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $

package PerlCase3;

use TestSystem;

sub TEST {
  print "Hello from Test Case 3\n";
  sleep 1;
  return (PASSED, MAJOR);
}

1;
