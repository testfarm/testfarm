#!/usr/bin/perl -w

##
## TestFarm
## test Program for Child processes management
##
## Author: Sylvain Giroudon
## Creation: 14-OCT-2003
##
## $Revision: 29 $
## $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
##

sub sigterm {
  print STDERR "[CHILD] SIGTERM caught.\n";
  sleep 3;
  print STDERR "[CHILD] SIGTERM exiting...\n";
  exit(0);
}

$SIG{TERM} = \&sigterm;

print "[CHILD] Press enter to exit:\n";
<STDIN>;
