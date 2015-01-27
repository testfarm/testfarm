#$DESCRIPTION Messages from 2 input sources
#$CRITICITY MAJOR

## Created: Fri Oct 30 14:28:49 2009
## $Revision: 1089 $
## $Date: 2009-10-30 17:06:33 +0100 (ven., 30 oct. 2009) $

package Input2;

use TestFarm::Trig;
use InputFlood;

sub TEST {
	my $verdict = FAILED;
	TrigVarDef($PERIODIC1, my $trig1);
	TrigVarDef($PERIODIC2, my $trig2);

	$trig1 = '^\d+\s+EVENT\s+10\s*$';
	$trig2 = '^\d+\s+EVENT\s+10\s*$';

	$PERIODIC1->tag('EVENT');
	$PERIODIC1->rate(500);
	$PERIODIC2->tag('EVENT');
	$PERIODIC2->rate(119);

	print "Waiting for 2*10 messages from 2 inputs...\n";
	if ( TrigWait("$trig1&$trig2", '6s') ) {
		print "IN_VERDICT All messages successfully seen\n";
		$verdict = PASSED;
	}

	$PERIODIC1->rate(0);
	$PERIODIC2->rate(0);

	return $verdict;
}

1;

