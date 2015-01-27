#$DESCRIPTION Messages from 1 input source
#$CRITICITY MAJOR

## Created: Fri Oct 30 13:52:24 2009
## $Revision: 1087 $
## $Date: 2009-10-30 14:59:45 +0100 (ven., 30 oct. 2009) $

package Input1;

use TestFarm::Trig;
use InputFlood;

sub TEST {
	my $verdict = FAILED;
	TrigVarDef($PERIODIC1, my $trig);

	$trig = '^\d+\s+EVENT\s+10\s*$';

	$PERIODIC1->tag('EVENT');
	$PERIODIC1->rate(500);

	print "Waiting for 10 messages from 1 input...\n";
	if ( TrigWait($trig, '6s') ) {
		print "IN_VERDICT All messages successfully seen\n";
		my $info = TrigInfo($trig);
		print "'$info'\n";
		$verdict = PASSED;
	}

	$PERIODIC1->rate(0);

	return $verdict;
}

1;

