#$DESCRIPTION Sample PERL Test Script
#$CRITICITY Serious

## $Revision: 657 $
## $Date: 2007-07-25 09:08:42 +0200 (mer., 25 juil. 2007) $

package PerlCase;

use TestFarm::Trig;
use TestSystem;

my $verdict = 0;

sub EVENTS {
  $verdict = 0;

  # Define a trigger "MESSAGE" that will listen to message "Hello"
  # from our Test Interface instanciated as object $INTERFACE
  TrigDef($INTERFACE, "MESSAGE", "Hello");

  # Action: send command "echo Hello World"
  $INTERFACE->echo("Hello World");

  # Wait for the command to reply within 1 s
  my $trigged = TrigWait("MESSAGE", "1s");

  # Analyse the event if detected
  if ( $trigged ) {
    # Get the reply that caused the trigger to be raised
    my $reply = TrigInfo("MESSAGE");

    # Extract some interesting info from this reply,
    # using the PERL test extraction features
    $reply =~ /^(\d+) +ECHO +(.+)$/;

    print "EVENT CAUGHT: timestamp=", $1, " message='", $2, "'\n";
  }

  # Timeout
  else {
    print "TIMEOUT !\n";
    $verdict = 1;
  }

  sleep 1;
}

sub VERDICT {
  return $verdict;
}

1;
