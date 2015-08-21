##
## TestFarm
## Test Suite Execution Library - Main Execution Loop
##
## This file is part of TestFarm, 
## the Test Automation Tool for Embedded Software.
## Please visit http://www.testfarm.org.
##
## TestFarm is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## TestFarm is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
##

package TestFarm::Exec;

use IO::Handle;
use Sys::Hostname;
use TestFarm::Engine;
use TestFarm::Locate;
use TestFarm::Config;


my $system = undef;
local *REQUEST;
local *REPLY;


##
## Test Suite program driven by the Graphical User Interface
##

sub MainInit($$) {
  my $report_dir = shift;
  my $global_log = shift;

  # STDERR redirection and filtering for WIZ error location retrieval
  TestFarm::Locate::filter_client();

  # Test Engine setup
  TestFarm::Engine::ReportDir($report_dir);

  # Test Engine start
  STDOUT->autoflush(1);
  TestFarm::Engine::start($global_log);
  TestFarm::Engine::version();

  my $local_log = 'local.log';
  if ( defined $report_dir ) {
    $local_log = $report_dir.'/'.$local_log;
  }
  TestFarm::Engine::LocalLog($local_log);

  if ( defined &{"Devel::ptkdb::"."brkonsub"} ) {
    print STDERR "TestFarm::Exec: PERL debug mode enabled\n";
  }
}


sub ManualAction($) {
  my $action = shift;

  if ( $system ) {
    my $prefix = ($action =~ s/^\$//) ? '$':'';
    my $func = $prefix.$system."::".$action;
    print "*** MANUAL ACTION: $func\n";
    eval($func);
  }
}


sub MainLoop($$$\@) {
  # Setup request pipe
  my $CTL_request = shift;
  open(REQUEST, "<&=$CTL_request") or die "TestFarm::Exec: Cannot setup request pipe: $!";
  REQUEST->autoflush(1);

  # Setup reply pipe
  my $CTL_reply = shift;
  open(REPLY, ">&=$CTL_reply") or die "TestFarm::Exec: Cannot setup reply pipe: $!";
  REPLY->autoflush(1);

  # Send acknowledge message
  $system = shift;
  my $ack = $system ? "-3":"-2";
  $ack .= " ".$TestFarm::Engine::pid if $TestFarm::Engine::pid;
  print REPLY "$ack\n";

  my $tab = shift;

  while ( 1 ) {
    my $request_line = <REQUEST>;
    last unless defined $request_line;
    chomp($request_line);

    # Test Case request
    if ( $request_line =~ s/^=// ) {
      my @request_tab = split(' ', $request_line);
      my $num = $request_tab[0];
      my $entry = $$tab[$num];
      my $verdict = $request_tab[1];

      if ( $verdict < 0 ) {
        # Get Test Case function settings
        my $handler = $$entry{'HANDLER'} || \&DefaultTestCase;
        my $id = $$entry{'ID'};
        my $package = $$entry{'PACKAGE'} || $id;
        my $criticity;

        # If a breakpoint is requested in debug mode, put it
        if ( $verdict == -2 ) {
          no strict 'refs';
          my $func = ${$package."::"}{'EVENTS'} ? "EVENTS" : "TEST";

	  if ( defined &{"Devel::ptkdb::"."brkonsub"} ) {
            Devel::ptkdb::brkonsub($package."::".$func);
          }
        }

        # Invoke Test Case function
        ($verdict, $criticity) = &{$handler}($id, $package);

        # Assume verdict is INCONCLUSIVE if an undefined value is returned
        $verdict = 2 unless defined $verdict;

        if ( $criticity ) {
          TestFarm::Engine::verdict($VerdictId[$verdict], $CriticityId[$criticity]);
        } else {
          TestFarm::Engine::verdict($VerdictId[$verdict]);
        }
        my $reply = $verdict;
        $reply .= " ".$criticity if $criticity;
        print REPLY "$reply\n";
      }

      $$entry{'VERDICT'} = $verdict;
    }

    # Manual interface action
    else {
      ManualAction($request_line);
    }
  }

  print STDERR "TestFarm::Exec: Terminated\n";
  exit(0);
}


sub ManualInput(;$) {
  my $str = shift || '';
  print REPLY "99 0 $str\n";

  my $verdict = undef;
  my $text = undef;

  while ( ! defined $verdict ) {
    my $request_line = <REQUEST>;
    last unless defined $request_line;
    chomp($request_line);

    # Input reply
    if ( $request_line =~ s/^=// ) {
      if ( $request_line =~ s/^INPUT\s+(\d+)\s*// ) {
	$verdict = $1;
	$text = $request_line;
      }
      else {
	print STDERR "*PANIC* Unexpected reply from User Interface\n";
	exit(1);
      }
    }

    # Manual interface action
    else {
      ManualAction($request_line);
    }
  }

  return ($verdict, $text);
}


##
## Standard Test Case processing
##

sub DefaultTestCase($$) {
  no strict 'refs';
  my $id = shift;
  my $package = shift;
  my $func = ${$package."::"}{'EVENTS'} ? "EVENTS" : "TEST";
  my $verdict;
  my $criticity;
  TestFarm::Engine::case($id);
  ($verdict, $criticity) = &{$package."::".$func}();
  TestFarm::Engine::done();
  ($verdict, $criticity) = &{$package."::VERDICT"}() if defined &{$package."::VERDICT"};
  return ($verdict, $criticity);
}


1;
