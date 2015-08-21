#!/usr/bin/perl -w

##
## TestFarm
## Test Suite Development Wizards - Makefile Generator
##
## Author: Sylvain Giroudon
## Creation: 21-JUN-2006
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

use File::Basename;
use POSIX;

use TestFarm::Env;

use TestFarm::Wiz qw(
  :DEFAULT
  @wizlib
  wizlib_init
  $verbose
);


############################################################
# Target list
############################################################

my @target_list = ();

sub target_add {
  my $file = shift;

  if ( $file =~ /\.tree$/ ) {
    if ( ! -e $file ) {
      print "[WARNING] Couldn't find file $file\n";
      return undef;
    }
  }

  $file =~ s/\.tree$//;

  if ( ! -e "$file.tree" ) {
    print "[WARNING] Couldn't find file $file.tree\n";
    return undef;
  }

  foreach ( @target_list ) {
    return undef if ( $file eq $_ );
  }

  push @target_list, $file;

  return $file;
}


############################################################
# Get options and arguments
############################################################

my $name = 'twiz-makefile';
my $rev = '$Revision: 1129 $ ';
$rev =~ /^\$Revision: (\S+)/;
$rev = $1;

my $colors = 0;

sub usage {
  print STDERR "TestFarm Makefile Wizard - version $rev\n" if defined $rev;
  print STDERR "Usage: $name [-v] [--colors] [-rev] [<tree-name>[.tree] ...]\n";
  exit(2);
}


my $target_argc = 0;

for (my $i = 0; $i <= $#ARGV; $i++) {
  my $argv = $ARGV[$i];
  if ( $argv =~ /^-/ ) {
    if ( $argv eq '-rev' ) {
      print "$rev\n";
      exit(0);
    }
    elsif ( $argv eq '-v' ) {
      $verbose = 1;
    }
    elsif ( $argv eq '--colors' ) {
      $colors = 1;
    }
    else {
      usage();
    }
  }
  else {
    $target_argc++;
    target_add($argv);
  }
}

INFO2(1, "TestFarm Makefile Wizard rev $rev");


############################################################
# Import WIZLIB path
############################################################

wizlib_init();

@wizlib2 = ();
my $pwd = getcwd();
foreach my $dir ( @wizlib ) {
  $dir =~ s/^$pwd\/*//;
  $dir =~ s/\/+$//;
  push @wizlib2, $dir;
}

if ( $#wizlib < 0 ) {
  print "[WARNING] No WIZ definition directory found\n";
}
else {
  INFO2(1, "WIZ definition files path: @wizlib2");
}


############################################################
# Generate Tree and Script Makefiles
############################################################

unless ( mkdir("Makefile.d", 0755) ) {
  if ( $! !~ /exists/ ) {
    print STDERR "*** Cannot create directory 'Makefile.d': $!\n";
    exit(1);
  }
}

my @depfiles = <Makefile.d/*.deps>;
if ( @depfiles ) {
  INFO2(1, "Deleting dependency files: @depfiles");
  unlink @depfiles;
}


############################################################
# Generate global Makefile
############################################################

# Retrieve target list
if ( $target_argc == 0 ) {
  foreach my $file ( <*.tree> ) {
    target_add($file);
  }
}

usage() if ( $#target_list < 0 );

INFO2(1, "Global make file: Makefile");
INFO2(0, "Available targets: @target_list");

# Retrieve system config list
my @system_list = ();
foreach my $xml ( <*.xml> ) {
  next if system("twiz-conf -c $xml &>/dev/null");
  my ($name) = fileparse($xml, (".xml"));
  push @system_list, $name;
}

INFO2(0, "Local System Configs: @system_list") if @system_list;

unless ( open(MAKEFILE, ">Makefile") ) {
  print STDERR "*** Cannot create file 'Makefile': $!\n";
  exit(1);
}

# Dump some information comments
print MAKEFILE "#\n";
print MAKEFILE "# This file was generated automatically using the TestFarm Makefile Wizard\n";
print MAKEFILE "# DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT\n";
print MAKEFILE "#\n\n";

print MAKEFILE "MAKEFILE_REV = $rev\n";
print MAKEFILE "TARGETS = @target_list\n";
if (@system_list) {
  print MAKEFILE "SYSTEMS =";
  foreach my $name (@system_list) {
    print MAKEFILE " $name.pm";
  }
  print MAKEFILE "\n";
}
print MAKEFILE "\n";

# Include optional Make pre-definitions
print MAKEFILE "-include Makefile.pre\n";

# Include standard TestFarm build rules
print MAKEFILE "include /opt/testfarm/lib/build.mk\n";
print MAKEFILE "\n";

# Add system config build rules
foreach my $name (@system_list) {
  print MAKEFILE "$name.pm: $name.xml\n";
}
print MAKEFILE "\n";

# Add targets defintion
foreach my $target ( @target_list ) {
  print MAKEFILE "$target: Makefile.d/$target.tree.deps Makefile.d/wizdef.deps\n";
  print MAKEFILE "Makefile.d/$target.tree.deps: $target.tree \$(shell [ -f Makefile.d/$target.tree.deps ] && cat Makefile.d/$target.tree.deps)\n";
}
print MAKEFILE "\n";

# Include optional Make post-definitions
print MAKEFILE "-include Makefile.post\n";

close(MAKEFILE);
