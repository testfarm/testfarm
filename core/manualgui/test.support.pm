##
## $Revision: 42 $
## $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
##

package Test;

sub Insert {
  my $W = shift;
  my $state = shift;

  if ( $state ) {
    print "InsertCard()\n";
  }
  else {
    print "WithdrawCard()\n";
  }
}

BEGIN {
  print STDERR "# SUPPORT::BEGIN\n";
}

END {
  print STDERR "# SUPPORT::END\n";
}

1;
