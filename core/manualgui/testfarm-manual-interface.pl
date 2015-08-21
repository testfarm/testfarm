#!/usr/bin/perl -w

##
## TestFarm
## Manual User Interface - GladeXML engine
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
use IO::Handle;
use POSIX ":sys_wait_h";
use Cwd;

use TestFarm::Env;
use TestFarm::Version;

my $banner = 'testfarm-manual-interface';


###########################################################
# Wait for cryptoperl feeder termination (if any)
###########################################################

if ( defined $__feeder__ ) {
  #print STDERR "-- $banner: FEEDER $__feeder__\n";
  waitpid($__feeder__, 0);
}


###########################################################
# Get & Check arguments
###########################################################

my $glade_filein = undef;
my $verbose = 0;
my $ctl = undef;

sub usage {
  print STDERR "TestFarm Manual User Interface (GladeXML engine) - version $VERSION\n" if defined $VERSION;
  print STDERR "Usage: $banner [-v] [-c<fd>] <glade-file>\n";
  exit(1);
}

for (my $i = 0; $i <= $#ARGV; $i++) {
  my $arg = $ARGV[$i];

  if ( $arg =~ /^-/ ) {
    if ( $arg eq "-v" ) {
      $verbose++;
    }
    elsif ( $arg =~ /^-c(\d+)$/ ) {
      $ctl = $1;
    }
    else {
      usage();
    }
  }
  else {
    usage() if ( defined $glade_filein );
    $glade_filein = $arg;
  }
}

usage() unless ( defined $glade_filein );

STDOUT->autoflush(1);

print STDERR "# Library path: @libs\n" if $verbose;


###########################################################
# Retrieve manual GUI interface definition
###########################################################

unless ( -f $glade_filein ) {
  foreach ( $PWD, @libs ) {
    my $file = "$_/$glade_filein";
    if ( -f $file ) {
      $glade_filein = $file;
      last;
    }

    $file = "$_/$glade_filein.glade";
    if ( -f $file ) {
      $glade_filein = $file;
      last;
    }
  }

  unless ( -f $glade_filein ) {
    my ($base, $dir) = fileparse($glade_filein, (".glade"));
    print STDERR "Cannot find Interface Definition file: $base.glade\n";
    exit(2);
  }
}



###########################################################
# Setup Gtk environment
###########################################################

# Determine Interface Library
unless ( open(FI, $glade_filein) ) {
  print STDERR "Cannot open Interface Definition $glade_filein: $!\n";
  exit(2);
}

my $gtk = undef;

while ( <FI> ) {
  s/^\s+//;
  s/\s+$//;
  next if /^$/;

  if ( $_ eq "<GTK-Interface>" ) {
    $gtk = "Gtk";
    last;
  }

  if ( $_ eq "<glade-interface>" ) {
    $gtk = "Gtk2";
    last;
  }
}

close(FI);

unless ( defined $gtk ) {
  print STDERR "Cannot recognise format of Interface Definition $glade_filein\n";
  exit(2);
}

$glade_lib = $gtk."::GladeXML";
$glade_pm = $gtk."/GladeXML.pm";

# Setup Interface Library
print STDERR "Setting up Interface Library $gtk\n" if $verbose;
require "$gtk.pm";
$gtk->import();
require $glade_pm;
$glade_lib->import();

# Load Interface Definition
print STDERR "Loading Interface Definition $glade_filein\n" if $verbose;
$gtk->init;
$g = $glade_lib->new($glade_filein);
exit(2) unless ( defined $g );

my ($glade_base, $glade_dir) = fileparse($glade_filein, (".glade"));
$glade_dir =~ s/\/+$//;


###########################################################
# Load libraries
###########################################################

my $support_file = "$glade_dir/$glade_base.support.pm";
my $support = undef;

unless ( -f $support_file ) {
  $support_file = "$glade_dir/support.pm";
  unless ( -f $support_file ) {
    $support_file = undef;
  }
}

if ( $support_file ) {
  local *FI;
  if ( open(FI, $support_file) ) {
    while ( <FI> ) {
      if ( /^\s*package\s+(\w+)/ ) {
        $support = $1;
        last;
      }
    }
    close(FI);
  }

  if ( $support ) {
    print STDERR "Loading Interface Support library $support_file (package $support)\n" if $verbose;
    require $support_file;
  }
  else {
    print STDERR "WARNING: Interface Support library $support_file is not a valid package\n";
  }
}


###########################################################
# Connect signals
###########################################################

my @gtk_windows = ( $g->get_widget('window') );

if ( $gtk eq "Gtk" ) {
  $g->signal_autoconnect_full(\&_autoconnect);
}
else {
  $g->signal_autoconnect(\&_autoconnect);
}

sub _parameters {
    return @_;
}

sub _handler {
  my $W = shift;
  my $handler = shift;

  # Split function name / parameter list
  $handler =~ /^(\w+)(.+)$/;
  my $func = $1;
  my $args = $2;

  # Evaluate parameter list
  my @parms = eval("_parameters $args");

  # If function exists in support package, call it directly */
  if ( $support ) {
    no strict 'refs';
    if ( ${$support."::"}{$func} ) {
      &{$support."::".$func}($W, @parms);
      $func = undef;
    }
  }

  # If function does not exist in support package, call it remotely as a Test Feature action */
  if ( defined $func ) {
    my $parms_str = '';

    foreach ( @parms ) {
      $parms_str .= ',' if $parms_str;

      if ( defined $_ ) {
        $parms_str .= " \"$_\"";
      }
      else {
        $parms_str .= ' undef';
      }
    }

    print $func.$parms_str."\n";
  }
}


sub _autoconnect {
    my $handler = shift;
    my $W = shift;
    my $signal = shift;

#    print "Autoconnect!\n";
#    print "  widget='$W'\n";
#    print "  signal='$signal'\n";

    # If widget is a Window, add it to the window list
    my $name = "$W";
    if ( $name =~ /^Gtk\S*::Window=/ ) {
#      print "  widget='$name'\n";
      push @gtk_windows, $W;
    }

    $handler =~ s/^\s+//;
    $handler =~ s/\s+$//;

    if ( $handler eq "gtk_main_quit" ) {
      $W->signal_connect($signal, \&_quit);
    }
    else {
      $handler =~ s/&gt;/>/g;
      $handler =~ s/&lt;/</g;
      $handler =~ s/&apos;/\'/g;
      $handler =~ s/&quot;/\"/g;
      $handler =~ s/&amp;/&/g;

      $widget = $W->get_name;
      print STDERR "# $widget:$signal => $handler\n" if $verbose;

      $W->signal_connect($signal, \&_handler, $handler);
    }
}

sub _quit {
  ctl_done();
  $gtk->main_quit;
}


###########################################################
# Setup control pipe
###########################################################

sub ctl_handler {
  my $line = <CTL_RD>;

  unless ( defined $line ) {
    _quit();
    return 0;
  }

  chomp($line);
  #print STDERR "CTL: '$line'\n";

  if ( $line eq "+" ) {
    foreach my $W ( @gtk_windows ) {
      $W->set_sensitive(1);
    }
  }
  elsif ( $line eq "-" ) {
    foreach my $W ( @gtk_windows ) {
      $W->set_sensitive(0);
    }
  }
  elsif ( $line eq "R" ) {
    foreach my $W ( @gtk_windows ) {
      $W->present();
    }
  }

  return 1;
}

sub ctl_done {
  return unless ( defined $ctl_id );
  if ( $gtk eq "Gtk" ) {
    Gtk::Gdk->input_remove($ctl_id);
  }
  else {
    Glib::Source->remove($ctl_id);
  }

  $ctl_id = undef;

  close(CTL_RD);
}


if ( defined $ctl ) {
  unless ( open(CTL_RD, "<&=$ctl") ) {
    print STDERR "$banner: Cannot open control pipe fd: $!\n";
    exit(3);
  }

  if ( $gtk eq "Gtk" ) {
    $ctl_id = Gtk::Gdk->input_add($ctl, 'read', \&ctl_handler);
  }
  else {
    $ctl_id = Glib::IO->add_watch($ctl, 'in', \&ctl_handler);
  }
}


###########################################################
# Enter main loop
###########################################################

$gtk->main;
