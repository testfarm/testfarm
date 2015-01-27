#$DESCRIPTION Sample TestFarm Prologue
#$ABORT_IF_FAILED

## $Revision: 657 $
## $Date: 2007-07-25 09:08:42 +0200 (mer., 25 juil. 2007) $

package Prologue;

use TestSystem;

sub EVENTS {
  INFO();
}

sub VERDICT {
  return PASSED;
}

1;
