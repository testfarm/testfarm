#$DESCRIPTION Sample TestFarm Epilogue

## $Revision: 657 $
## $Date: 2007-07-25 09:08:42 +0200 (mer., 25 juil. 2007) $

package Epilogue;

use TestSystem;

sub EVENTS {
  STOP();
}

sub VERDICT {
  return PASSED;
}

1;
