#$DESCRIPTION Sample PERL Test Script
#$CRITICITY Light

## $Revision: 42 $
## $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $

package TestSeq::PerlCase1;

use TestSystem;

sub TEST {
  print "Hello from Test Case 1\n";
  sleep 1;

  for (my $i = 0; $i < 40; $i++) {
    print "Fill text #", $i+1, "\n";
    sleep(1) if ( $i > 25 );
  }

  return PASSED;
}

1;
