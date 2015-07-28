#!/usr/bin/perl

use IO::Handle;
use Record;


# Disable i/o buffering
STDOUT->autoflush(1);
STDIN->autoflush(1);

# Retrieve program arguments
my $name = $ARGV[0] || 'DIGITS';
$name =~ s/\d+$//;

print STDERR "$name: @ARGV\n";

my @tab = ();

my $tprev = 0;


Record::Init($name, \&process_value);
$Record::debug = 1;

Record::Process();


sub process_value {
  my $id = shift;
  my $action = shift;
  my $msg = shift;
  my $t = shift;

  my $dt = $t - $tprev;
  $tprev = $t;
  #print "$action $id '$msg'\n";

  if ( $action eq 'DUMP' ) {
    if ( $id eq $ARGV[0] ) {
      @tab = ();
    }

    for (my $i = 0; $i <= $#ARGV; $i++) {
      if ( $id eq $ARGV[$i] ) {
	if ( $msg =~ s/@\S+\s+// ) {
	  $tab[$i] = $msg;
	}
	else {
	  $tab[$i] = undef;
	}
      }
    }
  }

  my $value = undef;
  if ( $#tab == $#ARGV ) {
    $value = "@tab";
    $value =~ s/\s//g;
    $value =~ s/\.+/./;

    # Invalidate value if it contains junk characters
    if ( $value =~ /^-?\d+(\.\d+)?$/ ) {
      print "VALUE=$value\n";
    }
    else {
      $value = undef;
    }
  }

  return $value;
}
