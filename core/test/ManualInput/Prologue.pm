#$DESCRIPTION TestFarm Test Suite Prologue
#$ABORT_IF_FAILED

package Prologue;

use TestSystem;

sub TEST {
  INFO();
  return PASSED;
}

1;
