#!/usr/bin/perl -w

##
## TestFarm
## System Config Viewer, Test Suite Launch Pad, Test Reports Management
##
## Author: Sylvain Giroudon
## Creation: 01-JUN-2004
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
use POSIX;
use Fcntl;
use FileHandle;
use IPC::Open3;

use XML::DOM;
use Gtk2;
use Gtk2::GladeXML;

BEGIN {
    $BIN_DIR = dirname($0);
    $BIN_DIR = getcwd() if ($BIN_DIR eq '.');
    $APP_DIR = dirname($BIN_DIR);
}

use TestFarm::Env;
use TestFarm::Version;
use TestFarm::FAM;
use TestFarm::Dialog;
use TestFarm::Config;


my $banner = 'testfarm-launch';
my $home = $ENV{HOME};
my $user_home = $home;
my $browser = $ENV{TESTFARM_BROWSER} || 'firefox';


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

my $verbose = 0;

sub usage {
  print STDERR "TestFarm Launch Pad - version $VERSION\n" if defined $VERSION;
  print STDERR "Usage: $banner [-v] [-v] [<user_home>]\n";
  exit(1);
}

for (my $i = 0; $i <= $#ARGV; $i++) {
  my $arg = $ARGV[$i];

  if ( $arg =~ /^-/ ) {
    if ( $arg eq "-v" ) {
      $verbose++;
    }
    else {
      usage();
    }
  }
  else {
      $user_home = `readlink -f '$arg'`;
      chomp($user_home);
  }
}

print STDERR "User directory = '$user_home'\n" if ($verbose > 0);

fam_verbose($verbose);


###########################################################
# Glade Interface definition
###########################################################

my $glade_interface_file = 'testfarm-launch.glade2';


sub glade_interface {
  my $root = shift;

  my $file = $glade_interface_file;
  if (! -f $file) {
      $file = '/opt/testfarm/lib/'.$glade_interface_file;
  }

  if (! -f $file) {
      print STDERR "PANIC: Cannot find interface definition file '$file'\n";
      return undef;
  }

  print STDERR "Loading interface definition file '$file'\n" if ( $verbose > 0 );
  return Gtk2::GladeXML->new($file, $root);
}


sub find_pixmap {
  my $file = shift;

  my $pixfile = $APP_DIR.'/icons/'.$file;
  return $pixfile if ( -f $pixfile );

  $pixfile = $home_dir.'/icons/'.$file;
  return $pixfile if ( -f $pixfile );

  print STDERR "WARNING: Cannot find image '$file'\n";

  return undef;
}


###########################################################
# Gtk environment
###########################################################

Gtk2->init();

my $g = glade_interface('window');
exit(2) unless ( defined $g );

$g->signal_autoconnect_from_package('main');

my $icon = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('testfarm.png'));
$g->get_widget('window')->set_icon($icon);

my $notebook = $g->get_widget('notebook');
$notebook->set_current_page(1);

my $rescan = $g->get_widget('rescan');
$rescan->set_sensitive(0);

my $text_renderer = Gtk2::CellRendererText->new();
my $pixbuf_renderer = Gtk2::CellRendererPixbuf->new();

my $suite_buttons = $g->get_widget('suite_buttons');
$suite_buttons->set_sensitive(0);

my $suite_add = $g->get_widget('suite_add');
my $suite_remove = $g->get_widget('suite_remove');

my $report_options = $g->get_widget('report_options');

my $report_build = $g->get_widget('report_build');
$report_build->set_sensitive(0);

my $output_buttons = $g->get_widget('output_buttons');
$output_buttons->set_sensitive(0);

my $output_log = $g->get_widget('output_log');

my $system_buttons = $g->get_widget('system_buttons');
$system_buttons->set_sensitive(0);

my $system_manual = $g->get_widget('system_manual');

my @queue_edit_widgets = (
  $g->get_widget('queue_remove'),
  $g->get_widget('queue_clear'),
  $g->get_widget('queue_up'),
  $g->get_widget('queue_down'),
);
foreach ( @queue_edit_widgets ) { $_->set_sensitive(0) }

my $queue_parallel = $g->get_widget('queue_parallel');

my @queue_exec_widgets = (
  $g->get_widget('queue_config'),
  $g->get_widget('queue_toolbar'),
);

$g->get_widget('system_view_icon')->set_from_file(find_pixmap('view.png'));
$g->get_widget('output_view_icon')->set_from_file(find_pixmap('view.png'));
$g->get_widget('output_log_icon')->set_from_file(find_pixmap('log.png'));
$g->get_widget('suite_exec_icon')->set_from_file(find_pixmap('launch-16.png'));

$icon = Gtk2::Image->new_from_file(find_pixmap('launch-32.png'));
$icon->show();
$g->get_widget('queue_start')->set_icon_widget($icon);

use constant GRAY => '#666';


###########################################################
# The Logo
###########################################################

my $logo = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('testfarm-logo.png'));
$g->get_widget('logo')->set_from_pixbuf($logo);


###########################################################
# Clipboard
###########################################################

my $clipboard = Gtk2::Clipboard->get(Gtk2::Gdk->SELECTION_PRIMARY);


###########################################################
# Console Model
###########################################################

use constant CONSOLE_COL_PID        => 0;
use constant CONSOLE_COL_DATE       => 1;
use constant CONSOLE_COL_TEXT       => 2;
use constant CONSOLE_COL_FOREGROUND => 3;

my $console_model = Gtk2::TreeStore->new(qw(Glib::String Glib::String Glib::String Glib::String));


###########################################################
# Console View
###########################################################

my $console_tree = $g->get_widget('console_treeview');
$console_tree->set_model($console_model);
$console_tree->get_selection()->set_mode('none');

my $console_text_renderer = Gtk2::CellRendererText->new();

$console_tree->insert_column_with_attributes(-1, "Date", $text_renderer,
					     'text'       => CONSOLE_COL_DATE);
$console_tree->get_column(0)->set_resizable(1);

$console_tree->insert_column_with_attributes(-1, "PID", $text_renderer,
					     'text'       => CONSOLE_COL_PID);
$console_tree->get_column(1)->set_resizable(1);

$console_tree->insert_column_with_attributes(-1, "Message", $console_text_renderer,
					     'text'       => CONSOLE_COL_TEXT,
					     'foreground' => CONSOLE_COL_FOREGROUND);


###########################################################
# Console Dump
###########################################################

my $console_length = 1000;

sub last_row_visible {
  my $tree = shift;
  return 0 unless $tree->realized;

  my $model = $tree->get_model();

  # Retrieve last row
  my $n = $model->iter_n_children();
  return 0 if ( $n <= 0 );
  my $last = $model->iter_nth_child(undef, $n-1);

  # Get last row visibility
  my $path = $model->get_path($last);
  my $rect = $tree->get_cell_area($path, $tree->get_column(0));
  my $visible = $tree->get_visible_rect();

  return (($rect->y - 1) <= $visible->height);
}


sub console_dump {
  my $pid = shift;
  my $color = shift;
  my $text = shift;

  # Get last row visibility
  my $scroll = last_row_visible($console_tree);

  # Show line on console
  my $date = strftime("%d-%b-%Y %H:%M:%S", localtime());

  my $iter = $console_model->append(undef);
  $console_model->set($iter,
		      CONSOLE_COL_DATE,       $date,
		      CONSOLE_COL_TEXT,       $text);

  if ( $pid ) {
    $console_model->set($iter, CONSOLE_COL_PID,  $pid);
  }

  if ( $color ) {
    $console_model->set($iter, CONSOLE_COL_FOREGROUND, $color);
  }

  $console_tree->realize();

  print STDERR "$text\n" if $verbose;

  # Clamp console length
  my $n = $console_model->get_string_from_iter($iter);
  if ( $n > $console_length ) {
    my $iter0 = $console_model->get_iter_from_string('0');
    $console_model->remove($iter0);
  }

  # Scroll down if last row was visible
  if ( $scroll ) {
    my $path = $console_model->get_path($iter);
    $console_tree->scroll_to_cell($path, undef, 1, 1.0, 0.0);
  }
}


###########################################################
# Proc Tree Model
###########################################################

use constant PROC_COL_PID     => 0;
use constant PROC_COL_COMMAND => 1;
use constant PROC_COL_TYPE    => 2;

my $proc_model = Gtk2::TreeStore->new(qw(Glib::String Glib::String Glib::String));


###########################################################
# Proc Tree View
###########################################################

my $proc_tree = $g->get_widget('proc_treeview');
$proc_tree->set_model($proc_model);
$proc_tree->get_selection()->set_mode('none');

$proc_tree->insert_column_with_attributes(-1, "PID", $text_renderer,
					  'text'       => PROC_COL_PID);
$proc_tree->get_column(0)->set_resizable(1);

$proc_tree->insert_column_with_attributes(-1, "Command Line", $text_renderer,
					  'text'       => PROC_COL_COMMAND);


###########################################################
# Proc Management
###########################################################

my %procs = ();

sub proc_stdout {
  my $fd = shift;
  my $cond = shift;
  my $pid = shift;

  my $proc = $procs{$pid};

  my $line = $$proc{stdout}->gets();

  unless ( defined $line ) {
    console_dump($pid, 'red', "EOF detected on STDOUT: channel disabled");
    $$proc{stdout_tag} = undef;
    return 0;
  }

  chomp($line);

  if ( $$proc{stdout_hdl} ) {
    &{$$proc{stdout_hdl}}($$proc{stdout_arg}, $line);
  }
  else {
    console_dump($pid, undef, $line);
  }

  return 1;
}

sub proc_stderr {
  my $fd = shift;
  my $cond = shift;
  my $pid = shift;

  my $proc = $procs{$pid};

  my $line = $$proc{stderr}->gets();

  unless ( defined $line ) {
    console_dump($pid, 'red', "EOF detected on STDERR: channel disabled");
    $$proc{stderr_tag} = undef;
    return 0;
  }

  chomp($line);
  console_dump($pid, undef, $line);

  return 1;
}

sub proc_start {
  my $cmd = shift;
  my $rootdir = shift;
  my $done_hdl = shift;
  my $done_arg = shift;
  my $stdout_hdl = shift;
  my $stdout_arg = shift;

  # Change working directory
  my $pwd_save = getcwd();
  if ( defined $rootdir ) {
    unless ( chdir($rootdir) ) {
      console_dump(0, 'red', "Cannot chdir to $rootdir: $!");
      return undef;
    }
  }

  # Setup new proc entry
  my %proc = ();
  $proc{cmd} = $cmd;
  $proc{rootdir} = $rootdir;
  $proc{done_hdl} = $done_hdl;
  $proc{done_arg} = $done_arg;
  $proc{stdout_hdl} = $stdout_hdl;
  $proc{stdout_arg} = $stdout_arg;

  # Prepare communication with process standard i/o
  $proc{stdin} = FileHandle->new;
  $proc{stdout} = FileHandle->new;
  $proc{stderr} = FileHandle->new;

  # Spawn process w/ redirected i/o
  my $pid = open3($proc{stdin}, $proc{stdout}, $proc{stderr}, $cmd);
  if ( $pid ) {
    my $msg = "Process started: $cmd";
    $msg .= " (wd=$rootdir)" if ( defined $rootdir );
    console_dump($pid, 'blue', $msg);

    $procs{$pid} = \%proc;

    # Setup proc i/o handling
    $proc{stdout}->autoflush(1);
    $proc{stderr}->autoflush(1);
    $proc{stdout_tag} = Glib::IO->add_watch(fileno($proc{stdout}), 'in', \&proc_stdout, $pid);
    $proc{stderr_tag} = Glib::IO->add_watch(fileno($proc{stderr}), 'in', \&proc_stderr, $pid);

    # Update proc list
    my $iter = $proc_model->append(undef);
    $proc_model->set($iter,
		     PROC_COL_PID,    $pid,
		     PROC_COL_COMMAND, $cmd);
    $proc{path} = $proc_model->get_string_from_iter($iter);
  }
  else {
    console_dump(0, 'red', "Cannot start process '$cmd': $!");

    close($proc{stdin});
    close($proc{stdout});
    close($proc{stderr});
  }

  # Restore working directory
  if ( defined $rootdir ) {
    chdir($pwd_save);
  }

  return $pid;
}


sub proc_find_by_cmd {
  my $cmd = shift;

  foreach my $pid ( keys %procs ) {
    my $proc = $procs{$pid};
    return $proc if ( $$proc{cmd} eq $cmd );
  }

  return undef;
}


sub proc_done {
  my $pid = shift || return;
  my $status = shift;
  my $proc = $procs{$pid} || return;

  my $status_str = (defined $status) ? $status : 'UNKNOWN';
  console_dump($pid, 'blue', "Process terminated (status=$status_str)");

  Glib::Source->remove($$proc{stdout_tag}) if ( defined $$proc{stdout_tag} );
  Glib::Source->remove($$proc{stderr_tag}) if ( defined $$proc{stderr_tag} );

  close($$proc{stdin}) if ( defined $$proc{stdin} );
  close($$proc{stdout}) if ( defined $$proc{stdout} );
  close($$proc{stderr}) if ( defined $$proc{stderr} );

  if ( defined $$proc{done_hdl} ) {
    &{$$proc{done_hdl}}($$proc{done_arg}, $pid, $status);
  }

  my $iter = $proc_model->get_iter_from_string($$proc{path});
  if ( defined $iter ) {
    $proc_model->remove($iter);
  }

  delete $procs{$pid};
}


sub proc_term {
  my $pid = shift || return;
  proc_done($pid);
  kill('TERM', $pid);
}


sub proc_terminate {
  foreach my $pid ( keys %procs ) {
    proc_term($pid);
  }
  %procs = ();
}


sub proc_write {
    my $pid = shift || return;
    my $proc = $procs{$pid} || return;

    foreach (@_) {
	$proc->{stdin}->print($_);
    }
}


###########################################################
# Workspace Tree Model
###########################################################

use constant SUITE_COL_TREENAME    => 0;
use constant SUITE_COL_STATUS      => 1;
use constant SUITE_COL_DESCRIPTION => 2;
use constant SUITE_COL_COLOR       => 3;
use constant SUITE_COL_TREEROOT    => 4;
use constant SUITE_COL_SYSTEM      => 5;

my $suite_model = Gtk2::TreeStore->new(qw(Glib::String Glib::String Glib::String Glib::String Glib::String Glib::String));


###########################################################
# Workspace Tree View
###########################################################

my $suite_tree = $g->get_widget('suite_treeview');
$suite_tree->set_model($suite_model);

my $suite_selection = $suite_tree->get_selection();
$suite_selection->set_mode('single');
$suite_selection->set_select_function(\&suite_select);

my $suite_text_renderer = Gtk2::CellRendererText->new();

$suite_tree->insert_column_with_attributes(-1, "Test Suites", $suite_text_renderer,
					   'text'       => SUITE_COL_TREENAME,
					   'foreground' => SUITE_COL_COLOR);
$suite_tree->get_column(0)->set_resizable(1);

$suite_tree->insert_column_with_attributes(-1, "Status", $suite_text_renderer,
					   'text'       => SUITE_COL_STATUS,
					   'foreground' => SUITE_COL_COLOR);

$suite_tree->insert_column_with_attributes(-1, "System", $suite_text_renderer,
					   'text'       => SUITE_COL_SYSTEM,
					   'foreground' => SUITE_COL_COLOR);
$suite_tree->get_column(2)->set_resizable(1);

$suite_tree->insert_column_with_attributes(-1, "Description", $suite_text_renderer,
					   'text'       => SUITE_COL_DESCRIPTION,
					   'foreground' => SUITE_COL_COLOR);


###########################################################
# Workspace Tree Scan: Test Tree files
###########################################################

sub tree_scan_entry {
  my $tree_tab = shift;
  my $tree = shift;

  my $entry = $$tree_tab{$tree};

  unless ( defined $entry ) {
    my %new_entry = ( 'refs' => [] );
    $entry = \%new_entry;
    $$tree_tab{$tree} = $entry;
  }

  return $entry;
}


sub tree_scan_file {
  my $tree_tab = shift;
  my $tree = shift;
  my $dir = shift;
  my $treename = shift;

  local *TREE;
  open(TREE, $tree)
    or return;

  my $entry = tree_scan_entry($tree_tab, $tree);

  while ( <TREE> ) {
    Gtk2->main_iteration while Gtk2->events_pending;

    s/\s+$//;
    s/^\s+//;
    next if /^$/;

    # Check for directives
    if ( /^#/ ) {
      if ( s/^#\$// ) {
	/^(\S+)\s+(.+)$/;
	my $directive = uc($1);
	my $arg = $2;

	$$entry{$directive} = $arg;
      }
      next;
    }

    # Other items should be one word per line : the test case/seq name
    # Otherwise formatted lines depict that the file is not a TestFarm tree
    unless ( /^\w+$/ ) {
      print STDERR "   *** Garbage detected in this file: deleting.\n" if ( $verbose > 0 );
      delete $$tree_tab{$tree};
      last;
    }

    print STDERR "   + '$_'\n" if ( $verbose > 1 );

    my $subdir = $dir.'/'.$_;
    next unless ( -d $subdir );

    my $subtree = $subdir.'/'.$treename.'.tree';
    $subtree = $subdir.'/.tree' unless ( -f $subtree );
    if ( -f $subtree ) {
      my $subentry = tree_scan_entry($tree_tab, $subtree);
      push @{$$subentry{'refs'}}, $tree;
      tree_scan_file($tree_tab, $subtree, $subdir, $treename);
    }
  }

  close(TREE);
}


sub tree_scan_dir {
  my $tree_tab = shift;
  my $dir = shift;

  local *DIR;
  opendir(DIR, $dir);

  my @files = ();
  my @dirs = ();

  my $entry;
  while ( defined ($entry = readdir(DIR)) ) {
    Gtk2->main_iteration while Gtk2->events_pending;

    next if $entry =~ /^\./;

    my $file = $dir."/".$entry;

    if ( -f $file ) {
      if ( $entry =~ /\.tree$/ ) {
	push @files, $file;
      }
    }
    elsif ( (-d $file) && (! -l $file) ) {
      push @dirs, $file;
    }
  }

  closedir(DIR);

  foreach ( @files ) {
    my $treename = fileparse($_, ('.tree'));
    next if ( $treename eq "" );
    print STDERR "-> '$treename' : $_\n" if ( $verbose > 0 );
    tree_scan_file($tree_tab, $_, $dir, $treename);
  }

  foreach ( @dirs ) {
    tree_scan_dir($tree_tab, $_)
  }
}


###########################################################
# Workspace Tree Scan
###########################################################

my %suites = ();


sub suite_scan {
  $suite_selection->unselect_all();
  $suite_model->clear();
  %suites = ();

  my %trees = ();
  tree_scan_dir(\%trees, $user_home);

  my %workspaces = ();

  foreach ( keys %trees ) {
    my $entry = $trees{$_};
    my $refs = $$entry{'refs'};

    unless ( @$refs ) {
      my ($treename, $dirname) = fileparse($_, ('.tree'));
      $dirname =~ s{/+$}{};

      my $workspace = $dirname;
      $workspace =~ s{^$home/*}{};

      my %suite = (
	'workspace'   => $workspace,
	'rootdir'     => $dirname,
	'reportdir'   => $dirname.'/report',
	'treeroot'    => $_,
	'treename'    => $treename,
	'description' => $$entry{'DESCRIPTION'} || "",
      );

      # Search for system configuration files
      my $system_name = $$entry{'SYSTEM'};
      if ( $system_name ) {
	$suite{'system'} = $system_name;

	foreach my $dir ( $dirname, @libs ) {
	  my $file = $dir.'/'.$system_name.'.xml';
	  if ( -f $file ) {
	    $suite{'system_xml'} = $file;
	    last;
	  }
	}

	foreach my $dir ( $dirname, @libs ) {
	  my $file = $dir.'/'.$system_name.'.pm';
	  if ( -f $file ) {
	    $suite{'system_pm'} = $file;
	    last;
	  }
	}
      }

      $suites{$_} = \%suite;
      $workspaces{$workspace} ||= [];
      push @{$workspaces{$workspace}}, \%suite;
    }
  }

  foreach ( sort keys %workspaces ) {
    my $refs = $workspaces{$_};

    my ($basename, $dirname) = fileparse($_, ());

    my $n = $#$refs + 1;
    my $description = "$_: $n Test Suite";
    $description .= "s" if ( $n > 1 );

    my $iter = $suite_model->append(undef);
    $suite_model->set($iter,
		      SUITE_COL_TREENAME,    $basename,
		      SUITE_COL_DESCRIPTION, $description,
		      SUITE_COL_COLOR,       GRAY);

    foreach my $suite ( @$refs ) {
      my $child_iter = $suite_model->append($iter);
      $$suite{iter} = $suite_model->get_string_from_iter($child_iter);

      $suite_model->set($child_iter,
			SUITE_COL_TREENAME,    $$suite{treename},
			SUITE_COL_DESCRIPTION, $$suite{description},
			SUITE_COL_TREEROOT,    $$suite{treeroot});

      if ( $$suite{system} ) {
	$suite_model->set($child_iter,
			  SUITE_COL_SYSTEM,      $$suite{system});
      }
    }
  }
}


###########################################################
# Workspace Tree Selection and Operations
###########################################################

my $suite_selected = undef;


sub suite_set_sensitive() {
  $suite_buttons->set_sensitive(defined $suite_selected);

  my $present = defined queue_find_suite($suite_selected);
  $suite_add->set_sensitive(!$present);
  $suite_remove->set_sensitive($present);
}


sub suite_select {
  my $selection = shift;
  my $model = shift;
  my $path = shift;
  my $is_currently_selected = shift;

  # Reject selection on Workspace row
  return 0 if ( $path->get_depth <= 1 );

  # Get selected Test Suite
  my @row = $model->get($model->get_iter($path));
  my $treeroot = $row[SUITE_COL_TREEROOT];
  my $suite = $suites{$treeroot};

  if ( $is_currently_selected ) {
    if ( (defined $suite_selected) && ($suite_selected eq $suite) ) {
      output_scan(undef);
      $suite_selected = undef;
    }
  }
  else {
    if ( (! defined $suite_selected) || ($suite_selected ne $suite) ) {
      output_scan($suite);
      $suite_selected = $suite;
    }
  }

  # Set Test Suite buttons sensitivity
  suite_set_sensitive();

  return 1;
}


sub suite_exec($) {
  my $suite = shift;

  my $pid = $$suite{pid_exec};

  if ( defined $pid ) {
    suite_raise($suite);
  }
  else {
    # Show error message if the system is already in use
    my $system = $$suite{system_xml};
    return if system_check($system);

    # Start the test suite
    suite_start($suite);
  }
}


sub suite_exec_clicked {
  my $suite = $suite_selected || return;
  suite_exec($suite);
}


sub suite_activated {
  my $tree = shift;
  my $path = shift;

  # Reject activation of Workspace row
  return if ( $path->get_depth <= 1 );

  # Get activated Test Suite
  my @row = $suite_model->get($suite_model->get_iter($path));
  my $treeroot = $row[SUITE_COL_TREEROOT];

  # Open Test Suite
  suite_exec($suites{$treeroot});
}


sub suite_add_clicked {
  my $suite = $suite_selected || return;
  queue_add_suite($suite);
  suite_set_sensitive();
}


sub suite_remove_clicked {
  my $suite = $suite_selected || return;
  queue_remove_suite($suite);
}


###########################################################
# Test Suite builder actions
###########################################################

sub suite_edit_done {
  my $suite = shift;
  delete $$suite{pid_edit};
}


sub suite_edit_clicked {
  my $suite = $suite_selected || return;

  my $pid = $$suite{pid_edit};

  if ( defined $pid ) {
      # Raise the Test Suite builder
      system('xdotool search --name "TestFarm Test Suite Builder" windowraise %@');
  }
  else {
    # Start the Test Suite builder
    my $cmd = 'testfarm-build '.$$suite{treename};
    $pid = proc_start($cmd, $$suite{rootdir}, \&suite_edit_done, $suite);
    if ( defined $pid ) { $$suite{pid_edit} = $pid }
  }
}


###########################################################
# New Test Suite
###########################################################

sub suite_new_destroy {
    $file_chooser_g = undef;
    $file_chooser_w = undef;
}

sub suite_new_done {
    $file_chooser_w->destroy() if defined $file_chooser_w;
    $file_chooser_g = undef;
    $file_chooser_w = undef;
}


sub suite_new_open {
    return unless defined $file_chooser_w;
    my $filename = $file_chooser_w->get_filename();
    suite_new_done();

    my ($basename, $dirname) = fileparse($filename, ('.tree'));

    # Start the Test Suite builder
    my $cmd = 'testfarm-build '.$basename;
    proc_start($cmd, $dirname, \&suite_scan, undef);
}

sub suite_new_clicked {
    $file_chooser_g = glade_interface('file_chooser');
    $file_chooser_g->signal_autoconnect_from_package('main');
    $file_chooser_w = $file_chooser_g->get_widget('file_chooser');

    # If a test suite is selected, use its parent directory
    my $dir = (defined $suite_selected) ? $$suite_selected{rootdir} : $ENV{HOME};
    print STDERR "---- NEW $dir\n";
    if (defined $dir) {
	$file_chooser_w->set_current_folder($dir);
    }
}


###########################################################
# Test Suite operations
###########################################################

sub suite_start($;$) {
  my $suite = shift;
  my $args = shift;

  # Build Test Suite User Interface command
  my $cmd = 'testfarm-run -c ';
  $cmd .= $args.' ' if ( defined $args );
  $cmd .= $$suite{treename}.'.tree';

  # Launch Test Suite User Interface
  my $pid = proc_start($cmd, $$suite{rootdir}, \&suite_done, $suite, \&suite_stdout, $suite);
  return undef unless $pid;

  $$suite{pid_exec} = $pid;

  # Lock system resources
  system_busy($$suite{system_xml}, $pid);

  return $pid
}


sub suite_done($) {
  my $suite = shift;
  $$suite{pid_exec} = undef;

  # Unlock system resources
  system_busy($$suite{system_xml}, undef);

  # Tell the Launch Queue that the Test Suite is done
  queue_done($suite);

  if ( $$suite{iter} ) {
    my $iter = $suite_model->get_iter_from_string($$suite{iter});
    $suite_model->set($iter, SUITE_COL_STATUS, '');
  }
}


sub suite_stdout {
  my $suite = shift;
  return unless $$suite{iter};

  my $text = shift;
  #print "-- '$text'\n";

  # Update the Workspace Tree
  my $iter = $suite_model->get_iter_from_string($$suite{iter});
  $suite_model->set($iter, SUITE_COL_STATUS, $text);

  # Update the Launch Queue Tree
  queue_status($suite, $text);
}


sub suite_raise {
  my $suite = shift;
  proc_write($suite->{pid_exec}, "R\n");
}


###########################################################
# System Tree Model
###########################################################

use constant SYSTEM_COL_NAME        => 0;
use constant SYSTEM_COL_DESCRIPTION => 1;
use constant SYSTEM_COL_XML_SHORT   => 2;
use constant SYSTEM_COL_XML         => 3;
use constant SYSTEM_COL_COLOR       => 4;

my $system_model = Gtk2::TreeStore->new(qw(Glib::String Glib::String Glib::String Glib::String Glib::String));


###########################################################
# System Tree View
###########################################################

my $system_tree = $g->get_widget('system_treeview');
$system_tree->set_model($system_model);

my $system_selection = $system_tree->get_selection();
$system_selection->set_mode('single');
$system_selection->set_select_function(\&system_select);

my $system_selected = undef;

my $system_text_renderer = Gtk2::CellRendererText->new();

$system_tree->insert_column_with_attributes(-1, "System / Feature", $system_text_renderer,
					    'text'       => SYSTEM_COL_NAME,
					    'foreground' => SYSTEM_COL_COLOR);

$system_tree->insert_column_with_attributes(-1, "Description", $system_text_renderer,
					    'text'       => SYSTEM_COL_DESCRIPTION,
					    'foreground' => SYSTEM_COL_COLOR);
$system_tree->get_column(1)->set_resizable(1);

$system_tree->insert_column_with_attributes(-1, "Configuration File", $system_text_renderer,
					    'text'       => SYSTEM_COL_XML_SHORT,
					    'foreground' => SYSTEM_COL_COLOR);


###########################################################
# System Tree Scan
###########################################################

sub system_scan_dir {
  my $dir = shift;
  my $list = shift;

  print STDERR "Scanning System Configuration Files in '$dir' ...\n" if ( $verbose > 0 );

  local *DIR;
  opendir(DIR, $dir);

  my $entry;
  while ( defined ($entry = readdir(DIR)) ) {
    Gtk2->main_iteration while Gtk2->events_pending;

    next if $entry =~ /^\./;
    next unless $entry =~ /\.xml$/;

    my $file = $dir."/".$entry;
    next unless ( -f $file );

    print STDERR "-> '$file'\n" if ( $verbose > 0 );
    push @$list, $file;
  }

  closedir(DIR);

  print STDERR "System Configuration scan completed\n" if ( $verbose > 0 );
}


sub system_get_description {
  my $element = shift;

  my $desc = "";
  foreach my $node ( $element->getElementsByTagName('DESCRIPTION', 0) ) {
    foreach ( $node->getChildNodes() ) {
      next unless ( $_->getNodeType() == TEXT_NODE );
      my $str = $_->getData();
      $str =~ s/^\s+//;
      next if ( $str =~ /^$/ );
      $str =~ s/\s+$//;
      $str =~ s/\s/ /g;
      $desc .= $str;
    }
  }

  return $desc;
}


sub system_scan {
  $system_selected = undef;
  $system_selection->unselect_all();
  $system_model->clear();

  my @list = ();

  foreach my $dir ( @libs ) {
    system_scan_dir($dir, \@list);
  }

  foreach ( keys %suites ) {
    my $suite = $suites{$_};
    print STDERR "Scanning System Configuration File from ".$$suite{treename}."\n" if ( $verbose > 0 );

    my $file = $$suite{system_xml};
    if ( $file ) {
      foreach ( @list ) {
	if ( $_ eq $file ) {
	  $file = undef;
	  last;
	}
      }

      if ( $file ) {
	print STDERR "-> '$file'\n" if ( $verbose > 0 );
	push @list, $file;
      }
    }

    print STDERR "System Configuration scan completed\n" if ( $verbose > 0 );
  }

  foreach my $file ( sort @list ) {
    my $filename = $file;
    $filename =~ s/^$user_home\///;
    my $name = fileparse($filename, ('.xml'));

    my $parser = new XML::DOM::Parser();
    my $doc = $parser->parsefile($file);

    my $root = $doc->getElementsByTagName('CONFIG', 0);
    next if ( $#$root < 0 );
    my $config = $$root[0];

    my $desc = system_get_description($config);

    my $iter = $system_model->append(undef);
    $system_model->set($iter,
		       SYSTEM_COL_NAME,        $name,
		       SYSTEM_COL_DESCRIPTION, $desc,
		       SYSTEM_COL_XML_SHORT,   $filename,
		       SYSTEM_COL_XML,         $file);

    foreach my $interface ( $config->getElementsByTagName('INTERFACE', 0) ) {
      foreach my $feature ( $interface->getElementsByTagName('FEATURE', 0) ) {
	my $id =  $feature->getAttribute("id");
	my $desc = system_get_description($feature);

	my $child = $system_model->append($iter);
	$system_model->set($child,
			   SYSTEM_COL_NAME,        $id,
			   SYSTEM_COL_DESCRIPTION, $desc,
			   SYSTEM_COL_COLOR,       GRAY);
      }
    }

    $doc->dispose();
    $parser = undef;
  }
}


###########################################################
# System Tree Selection
###########################################################


sub system_select {
  my $selection = shift;
  my $model = shift;
  my $path = shift;
  my $is_currently_selected = shift;

  # Get selected System
  my @row = $model->get($model->get_iter($path));
  my $system = $row[SYSTEM_COL_XML];
  return 0 unless ( defined $system );

  if ( $is_currently_selected ) {
    if ( (defined $system_selected) && ($system_selected eq $system) ) {
      $system_selected = undef;
      print STDERR "System unselected\n" if ($verbose > 1);
    }
  }
  else {
    if ( (! defined $system_selected) || ($system_selected ne $system) ) {
      $system_selected = $system;
      print STDERR "System selected: $system\n" if ($verbose > 1);
    }
  }

  $system_buttons->set_sensitive($system_selected);

  my $glade = $system;
  $glade =~ s/\.xml$/.glade/;
  $system_manual->set_sensitive(-f $glade);

  return 1;
}


sub system_activated {
  my $tree = shift;
  my $path = shift;

  # Get activated System
  my @row = $system_model->get($system_model->get_iter($path));
  my $system = $row[SYSTEM_COL_XML];

  system_view($system);
}


###########################################################
# System Tree Drag-n-Drop
###########################################################

$system_tree->enable_model_drag_source('GDK_BUTTON1_MASK',         # Gtk2::Gdk::ModifierType
				       'copy',                     # Gtk2::Gdk::DragAction
				       ['text/plain',    [], 1],   # Gtk2::TargetEntries
				       ['text/uri-list', [], 2],
				       ['_NETSCAPE_URL', [], 3]);

sub system_drag_begin {
  my $widget = shift;
  my $context = shift;

  my $pixbuf = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('file.png'));
  $context->set_icon_pixbuf($pixbuf, 0, 0);

  return 1;
}

sub system_drag_data_get {
  my $widget = shift;
  my $context = shift;
  my $selection_data = shift;
  my $info = shift;

  return 0 unless $system_selected;

  my $target = $selection_data->target();

  my $text = undef;

  if ( $info == 1 ) {  # text/plain
    $text = $system_selected;
  }
  elsif ( ($info == 2) || ($info == 3) ) { # text/uri-list, _NETSCAPE_URL
    $text = 'file://'.$system_selected;
  }

  return 0 unless $text;

  #print "   ", $target->name(), " -> '", $text, "'\n";
  $selection_data->set($target, 8, $text);

  return 1;
}


###########################################################
# System Resources reservation
###########################################################

# The following hash table contains one entry for each system in use.
# The key is the XML System Configuration file name, the content is the pid of
# the process that uses the system (which could be a Test Suite or a
# Manual User Interface)
my %systems = ();


sub system_check($;$) {
  my $system = shift;
  my $silent = shift;

  if ( defined $system ) {
    my $pid = $systems{$system};
    if ( defined $pid ) {
      # Retrieve the Test Suite that uses the system (if any)
      my $treename = undef;
      foreach my $treeroot ( keys %suites ) {
	my $suite = $suites{$treeroot};
	my $pid_exec = $$suite{pid_exec};
	if ( (defined $pid_exec) && ($pid_exec eq $pid) ) {
	  $treename = $$suite{treename};
	  last;
	}
      }

      # Show error message
      my $name = basename($system);
      my $msg = "System $name is already used\n By ";
      if ( defined $treename ) {
	$msg .= "Test Suite ".$treename;
      }
      else {
	$msg .= "Manual Interface";
      }
      $msg .= " (pid=$pid)";

      TestFarm::Dialog::error($msg) unless $silent;
      return $msg;
    }
  }

  return undef;
}


sub system_busy($;$) {
  my $system = shift;
  my $pid = shift;

  if ( defined $system ) {
    if ( defined $pid ) {
      $systems{$system} = $pid;
    }
    else {
      delete $systems{$system};
    }
  }
}


###########################################################
# System Buttons
###########################################################

sub system_view {
  my $system = shift;
  proc_start($browser.' file://'.$system);
}

sub system_view_clicked {
  my $system = $system_selected || return;
  system_view($system);
}


my $system_manual_ctl = 0;


sub system_manual_ctl_open {
  # Return if already open
  if ( $system_manual_ctl ) {
    return 0;
  }

  # Create interface control pipe
  unless ( pipe(MANUAL_CTL, MANUAL_CTL_WR) ) {
    console_dump(0, 'red', "Cannot create Manual Interface control pipe: $!");
    return -1;
  }

  # Disable close-on-exec mode on read endpoint
  unless ( fcntl(MANUAL_CTL, &F_SETFD, 0) ) {
    console_dump(0, 'red', "Cannot setup Manual Interface control pipe: $!");
    close(MANUAL_CTL);
    close(MANUAL_CTL_WR);
    return -1;
  }

  # Enable close-on-exec mode on write endpoint
  unless ( fcntl(MANUAL_CTL_WR, &F_SETFD, &FD_CLOEXEC) ) {
    console_dump(0, 'red', "Cannot setup Manual Interface control pipe: $!");
    close(MANUAL_CTL);
    close(MANUAL_CTL_WR);
    return -1;
  }

  MANUAL_CTL_WR->autoflush(1);

  $system_manual_ctl = 1;
  return 0;
}


sub system_manual_ctl_close {
  if ( $system_manual_ctl ) {
    close(MANUAL_CTL_WR);
  }
  $system_manual_ctl = 0;
}


sub system_manual_clicked {
  # If a manual interface is already running, bring its window to front
  if ( $system_manual_ctl ) {
    print MANUAL_CTL_WR "R\n";
    return;
  }

  # Get the selected system and check it is not being used
  my $system = $system_selected || return;
  return if system_check($system);

  # Open Manual Interface control pipe; Abort if failed.
  return if system_manual_ctl_open();

  my ($basename, $dirname) = fileparse($system, ('.xml'));
  my $cmd = 'testfarm-manual -c'.fileno(MANUAL_CTL).' '.$system;

  # Start the Manual User Interface for the System
  my $pid = proc_start($cmd, $dirname, \&system_manual_done, $system);

  # Close useless control pipe endpoint
  close(MANUAL_CTL);

  # Set system busy if Manual Interface launch succeeded;
  if ( $pid ) {
    system_busy($system, $pid);
  }
  # Close control pipe if Manual Interface launch failed
  else {
    system_manual_ctl_close();
  }
}


sub system_manual_done {
  my $system = shift;
  system_busy($system, undef);
  system_manual_ctl_close();
}


###########################################################
# Output Tree Model
###########################################################

use constant OUTPUT_COL_TYPE      => 0;
use constant OUTPUT_COL_NAME      => 1;
use constant OUTPUT_COL_SIZE      => 2;
use constant OUTPUT_COL_DATE      => 3;
use constant OUTPUT_COL_FILE      => 4;
use constant OUTPUT_COL_SIGNATURE => 5;
use constant OUTPUT_COL_TREEROOT  => 6;
use constant OUTPUT_COL_IMAGE     => 7;

my $output_model = Gtk2::TreeStore->new(qw(Glib::String Glib::String Glib::String Glib::String Glib::String Glib::String Glib::String Gtk2::Gdk::Pixbuf));


###########################################################
# Output Tree View
###########################################################

my $output_tree = $g->get_widget('output_treeview');
$output_tree->set_model($output_model);

my $output_column = Gtk2::TreeViewColumn->new();
$output_column->set_title('File Name');

my $cell_renderer = Gtk2::CellRendererPixbuf->new;
$output_column->pack_start($cell_renderer, FALSE);
$output_column->add_attribute($cell_renderer, 'pixbuf', OUTPUT_COL_IMAGE);

$cell_renderer = Gtk2::CellRendererText->new;
$output_column->pack_start($cell_renderer, TRUE);
$output_column->add_attribute($cell_renderer, 'text', OUTPUT_COL_NAME);

$output_tree->append_column($output_column);

$output_tree->insert_column_with_attributes(-1, "Size", $text_renderer,
					    'text' => OUTPUT_COL_SIZE);
$output_tree->get_column(1)->set_resizable(1);

$output_tree->insert_column_with_attributes(-1, "Date", $text_renderer,
					    'text' => OUTPUT_COL_DATE);

my $output_selection = $output_tree->get_selection();
$output_selection->set_mode('single');
$output_selection->set_select_function(\&output_select);


###########################################################
# Output Tree Scan
###########################################################

my @output_dirs = ();
my $output_rescan = undef;

my %output_pixbuf = ();


sub output_set_stat {
  my $iter = shift || return;
  my $file = shift;

  my @st = stat($file);

  if ( -f $file ) {
    my $size = $st[7] || 0;
    if ( $size > 1024 ) {
      $size /= 1024;
      if ( $size > 1024 ) {
	$size /= 1024;
	$size = int($size).'M';
      }
      else {
	$size = int($size).'K';
      }
    }

    $output_model->set($iter, OUTPUT_COL_SIZE, $size);
  }

  my $mtime = $st[9];
  my $time = '?';
  if ( defined $mtime ) {
    $time = strftime("%d-%b-%Y %H:%M:%S", localtime($mtime));
  }

  $output_model->set($iter, OUTPUT_COL_DATE, $time);

  return $mtime;
}


sub output_feed {
  my $type = shift;
  my $parent = shift;
  my $rootdir = shift;
  my $file = shift;
  my $md5sum = shift;

  my $name = $file;
  $name =~ s/^$rootdir(\/+)*//;

  my $iter = $output_model->append($parent);
  $output_model->set($iter,
		     OUTPUT_COL_TYPE, $type,
		     OUTPUT_COL_NAME, $name,
		     OUTPUT_COL_FILE, $file);

  my $mtime = output_set_stat($iter, $file);

  my $signature = undef;
  if ( $md5sum ) {
    $signature = $md5sum.' '.$mtime;
    $output_model->set($iter, OUTPUT_COL_SIGNATURE, $signature);
  }


  my $pixbuf = $output_pixbuf{$type};
  if ( ! defined $pixbuf ) {
    my $file = find_pixmap($type.'.png');
    if ( defined $file ) {
      $pixbuf = Gtk2::Gdk::Pixbuf->new_from_file($file);
      $output_pixbuf{$type} = $pixbuf;
    }
  }
  if ( defined $pixbuf ) {
    $output_model->set($iter, OUTPUT_COL_IMAGE, $pixbuf);
  }

  return ($iter, $signature);
}


sub output_feed_html {
  my $parent = shift;
  my $rootdir = shift;
  my $htmls = shift;

  my $count = 0;

  # Show HTML reports with their HTML logs
  foreach my $report ( @$htmls ) {
    next unless ( $$report[1] eq 'REPORT' );
    next if ( $$report[2] );
    $$report[2] = 1;
    $count++;

    (my $iter) = output_feed('report', $parent, $rootdir, $$report[0]);

    foreach my $log ( @$htmls ) {
      next unless ( $$log[1] eq 'LOG' );
      next if ( $$log[2] );
      $$log[2] = 1;
      $count++;

      output_feed('log', $iter, $rootdir, $$log[0]);
    }
  }

  # Show lost HTML logs
  foreach my $log ( @$htmls ) {
    next unless ( $$log[1] eq 'LOG' );
    next if ( $$log[2] );
    $$log[2] = 1;
    $count++;

    output_feed('log', $parent, $rootdir, $$log[0]);
  }

  return $count;
}


sub output_scan_dir {
  my $dirname = shift;
  my $treename = shift;
  my $dir_list = shift;
  my $out_list = shift;
  my $html_list = shift;

  # Scan this directory
  local *DIR;
  opendir(DIR, $dirname)
    or return undef;

  my $count = 0;

  my $entry;
  while ( defined ($entry = readdir(DIR)) ) {
    Gtk2->main_iteration while Gtk2->events_pending;

    next if ( $entry =~ /^\./ );

    if ( defined $treename ) {
      next unless ( $entry =~ /^$treename/ );
    }

    my $path = $dirname.'/'.$entry;

    if ( -f $path ) {
      if ( $entry eq 'output.xml' ) {
	push @$out_list, $path;
	$count++;
      }
      elsif ( $entry =~ /\.html$/ ) {
	push @$html_list, [$path];
	$count++;
      }
    }
    elsif ( (-d $path) && (! -l $path) ) {
      output_scan_dir($path, undef, $dir_list, $out_list, $html_list);
    }
  }

  closedir(DIR);

  # Add name to the list af scanned directories
  if ( $count > 0 ) {
    push @$dir_list, $dirname;
  }

  return $count;
}


sub output_scan {
  my $suite = shift;

  # Cancel currently monitored dirs
  foreach ( @output_dirs ) {
    fam_cancel($_);
  }
  @output_dirs = ();
  $output_rescan = undef;

  $output_selection->unselect_all();
  $output_model->clear();
  return unless defined $suite;

  my $workspace = $$suite{workspace};
  my $treeroot = $$suite{treeroot};
  my $treename = $$suite{treename};
  my $reportdir = $$suite{reportdir};

  return unless -d $reportdir;

  print STDERR "Scanning Test Output files in $reportdir\n" if ( $verbose > 0 );

  # Collect Test Output and Report files
  my @out_list = ();
  my @html_list = ();
  output_scan_dir($reportdir, $treename, \@output_dirs, \@out_list, \@html_list);

  # Monitor the scanned directories
  fam_monitor($reportdir, \&output_fam, $suite);
  foreach ( @output_dirs ) {
    fam_monitor($_, \&output_fam, $suite);
  }

  # Retrieve HTML Test Report files if any
  my %html_signature = ();
  foreach my $html ( @html_list ) {
    local *HTML;
    open(HTML, $$html[0]) or next;
    while ( <HTML> ) {
      s/^\s+//;
      s/\s+$//;
      next if /^$/;
      if ( s/^<meta\s+// ) {
	if ( /name="signature"\s+content="(.+)"/ ) {
	  my $signature = $1;
	  $signature =~ s/^(\S+)\s+//;
	  $$html[1] = $1;
	  #print "HTML $$html[0]: '$signature'\n";

	  $html_signature{$signature} ||= [];
	  push @{$html_signature{$signature}}, $html;
	  last;
	}
      }
    }
    close(HTML);
  }

  my @list = sort { -M $a <=> -M $b } @out_list;
  foreach my $file ( @list ) {
    # Ensure the file still exists, beacause during a bulk file erase,
    # it may have disappeared since the FAM event was triggered
    next unless -f $file;

    (my $md5sum) = split /\s+/, `md5sum $file 2>/dev/null`;
    return unless defined $md5sum;

    # Create Test Output entry
    my $dirname = dirname($file);
    my ($iter) = output_feed('entry', undef, $reportdir, $dirname);

    # Feed the Test Output branch
    my ($child, $signature) = output_feed('output', $iter, $reportdir, $file, $md5sum);
    $output_model->set($child, OUTPUT_COL_TREEROOT, $treeroot);
    #print "OUT $file: '$signature'\n";

    # Feed the Test Report subtree
    output_feed_html($iter, $reportdir, $html_signature{$signature});
  }

  # Show Lost+Found reports if any
  my $iter = $output_model->append(undef);
  $output_model->set($iter,
		     OUTPUT_COL_TYPE, 'lost',
		     OUTPUT_COL_NAME, "Lost+Found Reports");
  my $lost = 0;

  foreach my $htmls ( %html_signature ) {
    $lost += output_feed_html($iter, $reportdir, $htmls);
  }

  if ( $lost <= 0 ) {
    $output_model->remove($iter);
  }

  print STDERR "Test Output scan completed.\n" if ( $verbose > 0 );
}


sub output_find {
  my $file = shift;
  my $iter = shift;

  while ( $iter ) {
    my @row = $output_model->get($iter);
    my $col_file = $row[OUTPUT_COL_FILE];
    return $iter if ( (defined $col_file) && ($col_file eq $file) );

    my $child_iter = output_find($file, $output_model->iter_children($iter));
    return $child_iter if ( defined $child_iter );

    $iter = $output_model->iter_next($iter);
  }

  return undef;
}


sub output_fam_event {
  my $suite = shift;
  my $file = shift;
  my $type = shift;

  return unless ( $file =~ /\.(xml)|(html)$/ );
  my $iter = output_find($file, $output_model->get_iter_first()) || return;

  if ( $type eq 'change' ) {
    #print "OUTPUT-CHANGE : ".$output_model->get_string_from_iter($iter)." : $file\n";
    output_set_stat($iter, $file);
  }
  elsif ( $type eq 'delete' ) {
    #print "OUTPUT-DELETE: ".$output_model->get_string_from_iter($iter)." : $file\n";
    $output_selection->unselect_iter($iter);

    my $parent = $output_model->iter_parent($iter);
    my $child = $output_model->iter_children($iter);

    # Tree node parent has parent and children: Move children one level up
    if ( $parent && $child ) {
      # Get the list of children files
      my @files = ();
      while ( $child ) {
	my @row = $output_model->get($child);
	push @files, [$row[OUTPUT_COL_TYPE], $row[OUTPUT_COL_FILE], $row[OUTPUT_COL_SIGNATURE]];
	$child = $output_model->iter_next($child);
      }

      # Refeed children one level up
      my $reportdir = $$suite{reportdir}.'/';
      foreach my $file ( @files ) {
	output_feed($$file[0], $parent, $reportdir, $$file[1], $$file[2]);
      }

      # Remove the row and children of the deleted file
      $output_model->remove($iter);
    }

    # Row has no parent nor children: Remove the row
    else {
      $output_model->remove($iter);

      # If the Lost+Found entry is empty, remove it
      if ( ! $output_model->iter_has_child($parent) ) {
	my @row = $output_model->get($parent);
	unless ( $row[OUTPUT_COL_FILE] ) {
	  $output_selection->unselect_iter($parent);
	  $output_model->remove($parent);
	}
      }
    }
  }
}


sub output_fam {
  my $suite = shift;
  my $events = shift;

  return if defined $output_rescan;

  # Rescan the whole output tree upon the following events:
  # - When a Test Output or a HTML file is created
  # - When an Test Output file is deleted
  foreach my $ev ( @$events ) {
    my $type = $$ev{type};

    my ($basename, $dirname, $suffix) = fileparse($$ev{file}, ('.xml', '.html'));
    #$suffix ||= '';

    if ( ($type eq 'create') ||
	 (($type eq 'change') && ($suffix eq '.html')) ||
	 (($type eq 'delete') && ($suffix eq '.xml')) ) {
      $output_rescan = $suite;
      print STDERR "Deffered Test Output scan requested.\n" if ( $verbose > 0 );
      return;
    }
  }


  # At this point, the event list contains only 'change' and 'delete'
  # events, which are processed one by one.
  foreach my $ev ( @$events ) {
    my $file = $$ev{monitor}.'/'.$$ev{file};
    output_fam_event($suite, $file, $$ev{type});
  }
}


sub output_fam_timeout {
  return unless defined $output_rescan;

  # Retrieve the list of expanded Test Output nodes
  my $iter = $output_model->get_iter_first();
  my %expanded = ();
  my %selected = ();
  while ( $iter ) {
    my @row = $output_model->get($iter);
    my $file = $row[OUTPUT_COL_FILE];
    if ( defined $file ) {
      if ( $output_tree->row_expanded($output_model->get_path($iter)) ) {
	$expanded{$file} = 1;
      }
      if ( $output_selection->iter_is_selected($iter) ) {
	$selected{$file} = 1;
      }
    }
    $iter = $output_model->iter_next($iter);
  }

  # Rescan Test Output and Report files
  output_scan($output_rescan);

  # Re-expand nodes that were expanded before
  $iter = $output_model->get_iter_first();
  while ( $iter ) {
    my @row = $output_model->get($iter);
    my $file = $row[OUTPUT_COL_FILE];
    if ( defined $file ) {
      if ( $expanded{$file} ) {
	$output_tree->expand_row($output_model->get_path($iter), 0);
      }
      if ( $selected{$file} ) {
	$output_selection->select_iter($iter);
      }
    }
    $iter = $output_model->iter_next($iter);
  }
}


###########################################################
# Output Tree Selection
###########################################################

my $output_selected = undef;

sub output_set_sensitive {
  my $state1 = shift || 0;
  my $state2 = $state1;

  if ( defined $output_selected ) {
    $state2 = 0 unless ( $output_selected =~ /\.xml$/ );
  }
  else {
    $state1 = 0;
    $state2 = 0;
  }

  $output_buttons->set_sensitive($state1);
  $output_log->set_sensitive($state2);
  $report_build->set_sensitive($state2);
}


sub output_select {
  my $selection = shift;
  my $model = shift;
  my $path = shift;
  my $is_currently_selected = shift;

  # Get selected Test Output row
  my @row = $model->get($model->get_iter($path));
  my $file = $row[OUTPUT_COL_NAME];

  if ( $is_currently_selected ) {
    if ( (defined $output_selected) && ($output_selected eq $file) ) {
      $output_selected = undef;
    }
  }
  else {
    if ( (! defined $output_selected) || ($output_selected ne $file) ) {
      $output_selected = $file;
    }
  }

  # Set Output buttons sensitivity
  output_set_sensitive(1);

  return 1;
}


sub output_activated {
  my $tree = shift;
  my $path = shift;

  # Get activated Test Output row
  my @row = $output_model->get($output_model->get_iter($path));
  my $file = $row[OUTPUT_COL_NAME];

  output_view($file);
}


###########################################################
# Output Tree Drag-n-Drop
###########################################################

$output_tree->enable_model_drag_source('GDK_BUTTON1_MASK',         # Gtk2::Gdk::ModifierType
				       'copy',                     # Gtk2::Gdk::DragAction
				       ['text/plain',    [], 1],   # Gtk2::TargetEntries
				       ['text/uri-list', [], 2],
				       ['_NETSCAPE_URL', [], 3]);

sub output_drag_begin {
  my $widget = shift;
  my $context = shift;

  my $pixbuf = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('file.png'));
  $context->set_icon_pixbuf($pixbuf, 0, 0);

  return 1;
}

sub output_drag_data_get {
  my $widget = shift;
  my $context = shift;
  my $selection_data = shift;
  my $info = shift;

  my @list = output_copy();

  my $target = $selection_data->target();

  my $text = undef;

  if ( $info == 1 ) {  # text/plain
    $text = "@list";
  }
  elsif ( $info == 2 ) { # text/uri-list
    $text = "";
    foreach ( @list ) {
      $text.= "file://$_\r\n";
    }
  }
  elsif ( $info == 3 ) { # _NETSCAPE_URL
    $text = 'file://'.$list[0];
  }

  return 0 unless $text;

  #print "   ", $target->name(), " -> '", $text, "'\n";
  $selection_data->set($target, 8, $text);

  return 1;
}


#
# The Output tree is not a Drag destination,
# but I leave the code below as an example, since a spent hours
# setting-up this tricky DND feature (while the sun was shining. arggg).
#$output_tree->enable_model_drag_dest('copy', ['text/plain', [], 1]);

sub output_drag_data_received {
  my ($widget, $context, $x, $y, $selection_data, $info) = @_;

  #print "-- DRAG RECEIVED $widget $info\n";

  my @targets = $context->targets();
  foreach ( @targets ) {
    my $text = $selection_data->data();
    print "   ", $_->name, " -> '", $text, "'\n";
  }

  return 1;
}


###########################################################
# Test Output/Report buttons
###########################################################

my $report_options_pid = undef;

sub report_options_clicked {
  if ( $report_options_pid ) {
      proc_write($report_options_pid, "R\n");
  }
  else {
    $report_options_pid = proc_start('testfarm-config', undef, \&report_options_done);
  }
}


sub report_options_done {
  $report_options_pid = undef;
}


sub report_build_clicked {
  my $file = $output_selected || return;
  my $suite = $suite_selected || return;

  my $cmd = 'testfarm-report '.$file;
  proc_start($cmd, $$suite{reportdir}, \&report_build_done);
  output_set_sensitive(0);
}


sub report_build_done {
  output_set_sensitive(1);
}


sub output_is_viewable {
  my $fname = shift;
  my $file = shift;

  local *FI;
  unless ( open(FI, $fname) ) {
    TestFarm::Dialog::error("Cannot open Test Output file\n".$file.":\n$!");
    return 0;
  }

  my $viewable = 0;

  seek(FI, -100, 2);
  while ( <FI> ) {
    s/^\s+//;
    s/\s+$//;
    if ( $_ eq '</RESULT>' ) {
      $viewable = 1;
      last;
    }
  }
  close(FI);

  return $viewable;
}


sub output_view {
  my $file = shift;

  my $suite = $suite_selected || return;
  my $fname = $$suite{reportdir}.'/'.$file;

  # Check Test Output file is viewable
  if ( $file =~ /\.xml$/ ) {
    unless ( output_is_viewable($fname, $file) ) {
      TestFarm::Dialog::error("Cannot display the Test Output file\n".$file."\nbecause it is uncomplete or corrupted");
      return;
    }
  }

  proc_start($browser.' file://'.$fname);
}


sub output_view_clicked {
  my $file = $output_selected || return;
  output_view($file);
}


sub output_log_clicked {
  my $file = $output_selected || return;
  return unless ( $file =~ /\.xml$/ );

  my $suite = $suite_selected || return;

  my $fname = $$suite{reportdir}.'/'.$file;

  # Check wether the Test Output file is viewable
  # If not, check whether a .log file is available instead
  unless ( output_is_viewable($fname, $file) ) {
    $fname =~ s/\.xml$/.log/;
    unless ( -f $fname ) {
      TestFarm::Dialog::error("Cannot display the Test Output file\n".$file."\nbecause it is uncomplete or corrupted");
      return;
    }
  }

  # If a Log Viewer is already open on this file raise its window;
  # If not, launch a new Log Viewer
  my $cmd = 'testfarm-logview -c -t '.$file.' '.$fname;
  my $proc = proc_find_by_cmd($cmd);
  if ( $proc ) {
    $$proc{stdin}->print("R\n");
  }
  else {
    proc_start($cmd);
  }
}


sub output_copy_iter {
  my $iter = shift || return;
  my $list = shift;

  my @row = $output_model->get($iter);
  my $file = $row[OUTPUT_COL_FILE];

  if ( $file ) {
    push @$list, $file;
  }
  output_copy_children($iter, $list);
}

sub output_copy_children {
  my $iter = shift || return;
  my $list = shift;

  my $child = $output_model->iter_children($iter);
  while ( $child ) {
    output_copy_iter($child, $list);
    $child = $output_model->iter_next($child);
  }
}

sub output_copy {
  my $iter = $output_selection->get_selected() || return;

  my @list = ();
  output_copy_iter($iter, \@list);

  return @list;
}


sub output_copy_clicked {
  my @list = output_copy();
  $clipboard->set_text("@list") if @list;
}


sub output_delete_directory {
  my $dir = shift;
  my $errs = shift;

  my @subdirs = ();

  local *DIR;
  if ( opendir(DIR, $dir) ) {
    my $name;
    while ( defined ($name = readdir(DIR)) ) {
      next if ($name eq '.') || ($name eq '..');
      my $path = $dir.'/'.$name;

      if ( -f $path ) {
	unlink($path) || push @$errs, "$path: $!";
      }
      elsif ( (-d $path) && (! -l $path) ) {
	push @subdirs, $path;
      }
    }

    closedir(DIR);
  }

  foreach ( @subdirs ) {
    output_delete_directory($_, $errs);
  }

  rmdir($dir) || push @$errs, "$dir: $!";
}


sub output_delete_iter {
  my $iter = shift;
  my $errs = shift;

  my @row = $output_model->get($iter);
  my $file = $row[OUTPUT_COL_FILE];
  return unless defined $file;
  if ( -f $file ) {
    unlink($file) || push @$errs, "$file: $!";
  }
  elsif ( -d $file ) {
    output_delete_directory($file, $errs);
  }
}


sub output_delete_children {
  my $iter = shift;
  my $errs = shift;

  my $child = $output_model->iter_children($iter);
  while ( $child ) {
    output_delete_iter($child, $errs);
    output_delete_children($child, $errs);
    $child = $output_model->iter_next($child);
  }
}


sub output_delete {
  my $iter = $output_selection->get_selected();
  my @errs = ();

  $output_selection->unselect_iter($iter);

  output_delete_children($iter, \@errs);
  output_delete_iter($iter, \@errs);

  if ( $#errs >= 0 ) {
    my $text = "Failed to remove files:\n";
    foreach ( @errs ) {
      $text .= "$_\n";
    }
    Glib::Idle->add(\&TestFarm::Dialog::error, $text);
  }
}


sub output_delete_clicked {
  my $iter = $output_selection->get_selected() || return;

  my @row = $output_model->get($iter);
  my $type = $row[OUTPUT_COL_TYPE];
  my $name = $row[OUTPUT_COL_NAME];

  my $text = "Please confirm deletion of ";

  # HTML file
  if ( $type eq 'entry' ) {
    $text .= "Test Output directory\n".$name;
    $text .= "\nAnd all its content";
  }
  elsif ( $type eq 'output' ) {
    $text .= "Test Output file\n".$name;
  }
  elsif ( $type eq 'report' ) {
    $text .= "HTML Test Report file\n".$name;
    $text .= "\nAnd all the associated HTML Log files" if $output_model->iter_has_child($iter);
  }
  elsif ( $type eq 'log' ) {
    $text .= "HTML Test Log file\n".$name;
  }
  elsif ( $type eq 'lost' ) {
    $text .= "\nAll the Lost+Found HTML files";
  }

  TestFarm::Dialog::warning($text, \&output_delete);
}


###########################################################
# Launch Queue Tree Model
###########################################################

use constant QUEUE_COL_NAME        => 0;
use constant QUEUE_COL_STATUS      => 1;
use constant QUEUE_COL_DESCRIPTION => 2;
use constant QUEUE_COL_COLOR       => 3;
use constant QUEUE_COL_FILE        => 4;

my $queue_model = Gtk2::TreeStore->new(qw(Glib::String Glib::String Glib::String Glib::String Glib::String));


###########################################################
# Launch Queue Tree View
###########################################################

my $queue_tree = $g->get_widget('queue_treeview');
$queue_tree->set_model($queue_model);

my $queue_text_renderer = Gtk2::CellRendererText->new();

$queue_tree->insert_column_with_attributes(-1, "System / Test Suite", $queue_text_renderer,
					   'text'       => QUEUE_COL_NAME,
					   'foreground' => QUEUE_COL_COLOR);
$queue_tree->get_column(0)->set_resizable(1);

$queue_tree->insert_column_with_attributes(-1, "Status", $queue_text_renderer,
					   'text'       => QUEUE_COL_STATUS,
					   'foreground' => QUEUE_COL_COLOR);

$queue_tree->insert_column_with_attributes(-1, "Description", $queue_text_renderer,
					   'text'       => QUEUE_COL_DESCRIPTION,
					   'foreground' => QUEUE_COL_COLOR);

my $queue_selection = $queue_tree->get_selection();
$queue_selection->set_mode('single');
$queue_selection->set_select_function(\&queue_select);


###########################################################
# Launch Queue Tree Selection and Buttons
###########################################################

my $queue_modified = 0;
my $queue_selected = undef;
my $queue_args = "";


sub queue_select {
  my $selection = shift;
  my $model = shift;
  my $path = shift;
  my $is_currently_selected = shift;

  # BUG: to be removed when Gtk2 is fixed
  # The move Up/Down actions seem to SegFault on first level tree nodes,
  # So we disable selection on System rows to prevent user from doing it
  return 0 if ( $path->get_depth <= 1 );

  my $str = $model->get_string_from_iter($model->get_iter($path));

  if ( $is_currently_selected ) {
    if ( (defined $queue_selected) && ($queue_selected eq $str) ) {
      $queue_selected = undef;
    }
  }
  else {
    if ( (! defined $queue_selected) || ($queue_selected ne $str) ) {
      $queue_selected = $str;
    }
  }

  # Set Launch Queue buttons sensitivity
  foreach ( @queue_edit_widgets ) { $_->set_sensitive(defined $queue_selected) }

  #printf STDERR "-- SELECT %s\n", defined $queue_selected ? $queue_selected : '(none)';

  return 1;
}


sub queue_up_clicked {
  return unless ( defined $queue_selected );

  # Get Previous node in the same tree level
  my $iter = $queue_model->get_iter_from_string($queue_selected);
  my $path = $queue_model->get_path($iter);
  $path->prev();
  my $prev = $queue_model->get_iter($path);

  if ( (defined $prev) && $queue_model->iter_is_valid($prev) ) {
    $queue_selected = $queue_model->get_string_from_iter($prev);
    #print STDERR "-- UP -> $queue_selected\n";
    $queue_model->move_before($iter, $prev);
    #print STDERR "   OK\n";
    $queue_modified = 1;
  }
}


sub queue_down_clicked {
  return unless ( defined $queue_selected );

  # Get Next node in the same tree level
  my $iter = $queue_model->get_iter_from_string($queue_selected);
  my $path = $queue_model->get_path($iter);
  $path->next();
  my $next = $queue_model->get_iter($path);

  if ( (defined $next) && $queue_model->iter_is_valid($next) ) {
    $queue_selected = $queue_model->get_string_from_iter($next);
    #print STDERR "-- DOWN -> $queue_selected\n";
    $queue_model->move_after($iter, $next);
    #print STDERR "   OK\n";
    $queue_modified = 1;
  }
}


sub queue_remove_clicked {
  return unless ( defined $queue_selected );
  my $iter = $queue_model->get_iter_from_string($queue_selected);
  queue_remove($iter);
}


sub queue_clear_clicked {
  # Clear selection
  $queue_selection->unselect_all();

  # Clear Launch Queue
  $queue_model->clear();

  # Refresh Test Suite buttons sensitivity
  suite_set_sensitive();
}


sub queue_start_clicked {
  # Get Operator name
  my $operator = $g->get_widget('queue_entry_operator')->get_text();
  $operator =~ s/^\s+//;
  $operator =~ s/\s+$//;

  # Get Output release name
  my $release = $g->get_widget('queue_entry_release')->get_text();
  $release =~ s/^\s+//;
  $release =~ s/\s+$//;

  # Set command arguments
  $queue_args = '--go --quit';
  $queue_args .= ' --operator "'.$operator.'"' if $operator;
  $queue_args .= ' --release "'.$release.'"' if $release;

  queue_start();
}


###########################################################
# Launch Queue Batch management
###########################################################

my $queue_batch_name = undef;
my $queue_batch_combo = $g->get_widget('queue_batch_combo');
my $queue_batch_lock = 0;


sub queue_batch_save_clicked {
  return if $queue_batch_lock;
  my $name = queue_batch_accept();
  queue_batch_save($name);
}


sub queue_batch_delete_clicked {
  return if $queue_batch_lock;
  my $row = $queue_batch_combo->get_active();
  return if ( $row < 0 );

  my $model = $queue_batch_combo->get_model();
  my $iter = $model->get_iter_from_string($row);
  (my $name) = $model->get($iter);

  # Get batch file name
  my $file = TestFarm::Config::set_conf('batch', $name);

  # Delete batch file
  if ( unlink($file) ) {
    print STDERR "Batch File $file deleted\n" if $verbose;

    # Select 'Default' batch
    TestFarm::Config::set_conf('defaults');
    TestFarm::Config::set_string('batch', 'Default');
  }
  else {
    console_dump(0, 'red', "Cannot delete batch file $file: $!");
  }

  queue_batch_scan();
}


sub queue_batch_combo_changed {
  return if $queue_batch_lock;
  my $row = $queue_batch_combo->get_active();

  if ( $row >= 0 ) {
    my $name = $queue_batch_combo->child->get_text();

    if ( $queue_modified ) {
      TestFarm::Dialog::question("Batch '$queue_batch_name' modified.\nSave it before loading '$name' ?",
			\&queue_batch_change, \&queue_batch_load, $name);
    }
    else {
      queue_batch_load($name);
    }
  }
}


sub queue_batch_scan {
  $queue_batch_lock = 1;

  my $model = $queue_batch_combo->get_model();
  $model->clear();

  my $row = 0;
  my $active_row;
  my $active_name;

  # Select batch configuration context
  my $dir = dirname(TestFarm::Config::set_conf('batch'));
  print STDERR "Scanning Batch Files in $dir\n" if $verbose;

  # Get the list of batches
  local *DIR;
  if ( opendir(DIR, $dir) ) {
    # Get currently selected batch
    TestFarm::Config::set_conf('defaults');
    $active_name = TestFarm::Config::get_string('batch');

    my $name;
    while ( defined ($name = readdir(DIR)) ) {
      next if $name =~ /^\./;

      my $iter = $model->append();
      $model->set($iter, 0, $name);

      if ( (defined $active_name) && ($name eq $active_name) ) {
	$active_row = $row;
      }
      $row++;
    }

    closedir(DIR);
  }

  print STDERR "Batch file scan completed\n" if ( $verbose > 0 );

  # Append a Default batch if list is empty
  if ( $row == 0 ) {
    $active_row = 0;
    $active_name = 'Default';

    my $iter = $model->append();
    $model->set($iter, 0, $active_name);
  }

  if ( defined $active_row ) {
    $queue_batch_combo->set_active($active_row);
  }

  if ( defined $active_name ) {
    queue_batch_load($active_name);
  }

  $queue_batch_name = $active_name;
  $queue_batch_lock = 0;
}


sub queue_batch_accept {
  my $model = $queue_batch_combo->get_model();
  my $row = $queue_batch_combo->get_active();

  my $iter;

  # New batch name
  if ( $row < 0 ) {
    my $name = $queue_batch_combo->child->get_text();
    $name =~ s/^\s+//;
    $name =~ s/\s+$//;
    $name = 'Default' if ( $name eq '' );

    # Ensure the new name is not already present in the list
    $iter = $model->get_iter_first();
    while ( $iter ) {
      my @tab = $model->get($iter);
      if ( $tab[0] eq $name ) {
	$name = undef;
	last;
      }
      $iter = $model->iter_next($iter);
    }

    if ( defined $name ) {
      print STDERR "New batch: $name\n" if ( $verbose > 0 );
      $iter = $model->prepend();
      $model->set($iter, 0, $name);
    }
  }

  # Batch name selected from list
  else {
    $iter = $model->get_iter_from_string($row);
  }

  my @tab = $model->get($iter);
  $name = $tab[0];
  $name = 'Default' if ( $name eq '' );

  # Set newly selected batch
  TestFarm::Config::set_conf('defaults');
  TestFarm::Config::set_string('batch', $name);
  $queue_batch_name = $name;

  return $name;
}


sub queue_batch_save {
  my $name = shift || return undef;
  my $file = TestFarm::Config::set_conf('batch', $name);
  return queue_save($file, $name);
}


sub queue_batch_load {
  my $name = shift || return undef;

  # Set newly selected batch
  TestFarm::Config::set_conf('defaults');
  TestFarm::Config::set_string('batch', $name);
  $queue_batch_name = $name;

  my $file = TestFarm::Config::set_conf('batch', $name);
  return queue_load($file);
}


sub queue_batch_change {
  my $name = shift || return undef;
  queue_batch_save($queue_batch_name);
  queue_batch_load($name);
}


###########################################################
# Launch Queue Edition
###########################################################

use constant QUEUE_UNDEFINED_SYSTEM => 'Undefined System';

sub queue_find_system {
  my $system = shift || QUEUE_UNDEFINED_SYSTEM;

  my $parent = $queue_model->get_iter_first();
  while ( $parent ) {
    my @row = $queue_model->get($parent);
    return $parent if ( $row[QUEUE_COL_FILE] eq $system );
    $parent = $queue_model->iter_next($parent);
  }

  return undef;
}


sub queue_add_system($) {
  my $system = shift;

  my $parent = queue_find_system($system);
  if ( defined $parent ) {
    return $parent;
  }

  # Create parent System if none exists
  $parent = $queue_model->append(undef);

  if ( $system ) {
    my $name = fileparse($system, ('.xml'));
    $queue_model->set($parent,
		      QUEUE_COL_NAME, $name,
		      QUEUE_COL_FILE, $system);
  }
  else {
    $queue_model->set($parent,
		      QUEUE_COL_NAME, QUEUE_UNDEFINED_SYSTEM,
		      QUEUE_COL_FILE, QUEUE_UNDEFINED_SYSTEM);
  }

  # NOTE: to be removed when the System rows are selectable,
  # i.e. when the Gtk2 bug on Up/Down operations will be fixed
  $queue_model->set($parent,
		    QUEUE_COL_COLOR, GRAY);

  return $parent;
}


sub queue_find_treeroot($) {
  my $treeroot = shift;

  my $parent = $queue_model->get_iter_first();
  while ( $parent ) {
    my $iter = $queue_model->iter_children($parent);
    while ( $iter ) {
      my @row = $queue_model->get($iter);
      return $iter if ( $row[QUEUE_COL_FILE] eq $treeroot );
      $iter = $queue_model->iter_next($iter);
    }

    $parent = $queue_model->iter_next($parent);
  }

  return undef;
}


sub queue_find_suite {
  my $suite = shift || return undef;
  return queue_find_treeroot($$suite{treeroot});
}


sub queue_add_suite($) {
  my $suite = shift;
  #print STDERR "ADD $$suite{treeroot}\n";

  # Do nothing if Test Suite is already present in the queue
  return if defined queue_find_suite($suite);

  # Get parent System node
  my $system = $$suite{system_xml};
  my $parent = queue_add_system($system);

  my $iter = $queue_model->append($parent);
  $queue_model->set($iter,
		    QUEUE_COL_NAME,        $$suite{treename},
		    QUEUE_COL_DESCRIPTION, $$suite{description},
		    QUEUE_COL_FILE,        $$suite{treeroot});
  $queue_tree->expand_to_path($queue_model->get_path($iter));
  $queue_modified = 1;
}


sub queue_remove {
  my $iter = shift || return;

  # Clear selection
  $queue_selection->unselect_all();

  # Get parent System node
  my $parent = $queue_model->iter_parent($iter);

  # Remove Test Suite from Launch Queue
  $queue_model->remove($iter);
  $queue_modified = 1;

  # Also remove parent System if empty
  if ( defined $parent ) {
    unless ( $queue_model->iter_has_child($parent) ) {
      $queue_model->remove($parent);
    }
  }

  # Refresh Test Suite buttons sensitivity
  suite_set_sensitive();
}


sub queue_remove_suite {
  my $suite = shift;
  #print STDERR "REMOVE $$suite{treeroot}\n";

  queue_remove(queue_find_suite($suite));
}


sub queue_save($;$) {
  my $file = shift || return undef;
  my $name = shift;

  local *FOUT;
  unless ( open(FOUT, ">$file") ) {
    console_dump(0, 'red', "Cannot save batch file $file: $!");
    return undef;
  }

  print STDERR "Saving Batch File $file\n" if $verbose;

  print FOUT "<?xml version=\"1.0\"?>\n";
  print FOUT "<BATCH";
  if ( defined $name ) {
    print FOUT " name=\"", $name, "\"";
  }
  print FOUT ">\n";

  my $parent = $queue_model->get_iter_first();
  while ( $parent ) {
    my @tab = $queue_model->get($parent);
    my $system = $tab[QUEUE_COL_NAME];

    my $iter = $queue_model->iter_children($parent);
    while ( $iter ) {
      my @tab = $queue_model->get($iter);
      my $treename = $tab[QUEUE_COL_NAME];

      print FOUT "  <suite system=\"", $system, "\">", $treename, "</suite>\n";
      $iter = $queue_model->iter_next($iter);
    }

    $parent = $queue_model->iter_next($parent);
  }

  print FOUT "</BATCH>\n";

  close(FOUT);

  $queue_modified = 0;

  return $file;
}


sub queue_load($) {
  my $file = shift;

  local *FIN;
  unless ( open(FIN, $file) ) {
    console_dump(0, 'red', "Cannot access batch file $file: $!");
    return undef;
  }
  close(FIN);

  print STDERR "Loading Batch File $file\n" if $verbose;

  my $parser = new XML::DOM::Parser();
  my $doc = $parser->parsefile($file);

  my $root = $doc->getDocumentElement();
  return undef unless defined $root;

  # Clear Launch Queue
  $queue_model->clear();

  # Feed Launch Queue with Batch items
  foreach my $node ( $root->getElementsByTagName('suite', 0) ) {
    my $system = $node->getAttribute('system');

    foreach ( $node->getChildNodes() ) {
      next unless ( $_->getNodeType() == TEXT_NODE );
      my $treename = $_->getData();
      $treename =~ s/^\s+//;
      $treename =~ s/\s+$//;

      print STDERR "-> system='$system' treename='$treename'\n" if $verbose;

      # Retrieve test suite corresponding to this item
      my $suite = undef;
      foreach ( keys %suites ) {
	$suite = $suites{$_};

	# Check system if defined, otherwise ignore it
	if ( (defined $$suite{system}) && (defined $system) &&
	     ($$suite{system} ne $system) ) {
	  $suite = undef;
	  next;
	}

	# Check tree name
	if ( $$suite{treename} eq $treename ) {
	  last;
	}

	$suite = undef;
      }

      if ( defined $suite ) {
	queue_add_suite($suite);
      }
      else {
	my $name = basename($file);
	console_dump(0, 'blue', "Unknown Test Suite $treename referenced in batch $name");
      }
    }
  }

  $doc->dispose();
  $parser = undef;

  $queue_modified = 0;
}


###########################################################
# Launch Queue Execution
###########################################################

my %queue_tasks = ();


sub queue_setup() {
  # Clear task list
  my @tasks = ();

  my @busy = ();

  # Build the task list, and
  # Ensure all the scheduled Systems and Test Suites are not in use
  my $first = 1;
  my $parent = $queue_model->get_iter_first();
  while ( $parent ) {
    my @row = $queue_model->get($parent);
    my $system = $row[QUEUE_COL_FILE];

    # Check the System is not in use
    my $msg = system_check($system, 1);
    push @busy, $msg if $msg;

    # If no system is attached to the Test Suite, take a dummy one
    $system ||= QUEUE_UNDEFINED_SYSTEM;

    # If parallel mode is enabled, the first Test Suite of each System branch
    # is scheduled for execution
    $first = 1 if $queue_parallel->get_active();

    my $iter = $queue_model->iter_children($parent);
    while ( $iter ) {
      my @row = $queue_model->get($iter);
      my $suite = $suites{$row[QUEUE_COL_FILE]};

      # Feed the task list with first item
      push @tasks, $suite if $first;
      $first = 0;

      # Check the Test Suite is not in use
      my $pid = $$suite{pid_exec};
      if ( $pid ) {
	my $msg = "Test Suite ".$$suite{treename}." is in use (pid=$pid)";
	push @busy, $msg;
      }

      # Clear Status
      $queue_model->set($iter, QUEUE_COL_STATUS, "");

      $iter = $queue_model->iter_next($iter);
    }

    $parent = $queue_model->iter_next($parent);
  }

  # Show error message if busy resources have been detected
  foreach my $msg ( @busy ) {
    TestFarm::Dialog::error($msg);
    return undef;
  }

  return @tasks;
}


sub queue_set_sensitive() {
  my $not_running = (scalar keys %queue_tasks <= 0);
  foreach ( @queue_exec_widgets ) { $_->set_sensitive($not_running) }
}


sub queue_status($$) {
  my $suite = shift;
  my $status = shift;

  my $iter = queue_find_suite($suite);
  if ( defined $iter ) {
    $queue_model->set($iter, QUEUE_COL_STATUS, $status);
  }
}


sub queue_exec($) {
  my $suite = shift || return;

  my $status = 'Start Failed';
  my $pid = suite_start($suite, $queue_args);
  if ( $pid ) {
    $status = 'Started';
    $queue_tasks{$$suite{treeroot}} = $pid;
    #print "-- TASK STARTED: ", $$suite{treeroot}, "\n";
  }

  # Set Status
  queue_status($suite, $status);
}


sub queue_start() {
  # Setup task list
  my @tasks = queue_setup();

  # Do nothing if no task scheduled
  return if ( $#tasks < 0 );

  %queue_tasks = ();
  foreach my $suite ( @tasks ) {
    queue_exec($suite);
  }

  queue_set_sensitive();
}


sub queue_done($) {
  my $suite = shift;

  # Retrieve Test Suite from the Launch Queue list
  my $treeroot = $$suite{treeroot};
  my $iter = queue_find_treeroot($treeroot);

  # Delete Test Suite from task list (if present)
  unless ( defined $queue_tasks{$treeroot} ) {
    if ( defined $iter ) {
      $queue_model->set($iter, QUEUE_COL_STATUS, '');
    }
    return;
  }
  delete $queue_tasks{$treeroot};

  # If batch is in progress, go next
  if ( defined $iter ) {
    # Set Status
    my $status = 'Terminated '.strftime("%d-%b-%Y %H:%M:%S", localtime());
    $queue_model->set($iter, QUEUE_COL_STATUS, $status);

    my $parent = $queue_model->iter_parent($iter);

    # Get next scheduled Test Suite
    $iter = $queue_model->iter_next($iter);

    # If the System branch is finished and the parallel mode is disabled,
    # execute the first test suite of the next System branch
    unless ( (defined $iter) || $queue_parallel->get_active() ) {
      $parent = $queue_model->iter_next($parent);
      if ( defined $parent ) {
	$iter = $queue_model->iter_children($parent);
      }
    }

    if ( defined $iter ) {
      my @row = $queue_model->get($iter);
      queue_exec($suites{$row[QUEUE_COL_FILE]});
    }
  }

  queue_set_sensitive();
}


###########################################################
# Signal Handling
###########################################################

sub sig_chld {
  my $str = '';

  # Collect pid's of terminated children
  while ( 1 ) {
    my $pid = waitpid(-1, WNOHANG);
    last if ( $pid <= 0 );
    my $status = $?;
    if ( $str ne '' ) {
      $str .= ' ';
    }
    $str .= $pid.':'.$status;
  }

  #print STDERR "--- SIGCHLD $str\n";
  if ( $str ne '' ) {
    print SIGNAL_WR "$str\n";
  }
}


sub sig_timeout {
  output_fam_timeout();
  return 1;
}


sub sig_process {
  my $str = <SIGNAL_RD>;

  unless ( defined $str ) {
    main_quit();
    return 0;
  }

  #print STDERR "--- SIGNAL_RD $str\n";
  my @pids = split /\s+/, $str;

  foreach ( @pids ) {
      my ($pid, $status) = split /:/;
      proc_done($pid, $status);
  }

  return 1;
}


# Create signal handling pipe
unless ( pipe(SIGNAL_RD, SIGNAL_WR) ) {
  print STDERR "$banner: Cannot create Signal pipe: $!\n";
  exit(3);
}

# Enable close-on-exec mode on read endpoint
unless ( fcntl(SIGNAL_RD, &F_SETFD, &FD_CLOEXEC) ) {
  print STDERR "$banner: Cannot setup Signal pipe read endpoint: $!\n";
  exit(3);
}

# Enable close-on-exec mode on write endpoint
unless ( fcntl(SIGNAL_WR, &F_SETFD, &FD_CLOEXEC) ) {
  print STDERR "$banner: Cannot setup Signal pipe write endpoint: $!\n";
  exit(3);
}

SIGNAL_RD->autoflush(1);
SIGNAL_WR->autoflush(1);

my $sig_io_tag = Glib::IO->add_watch(fileno(SIGNAL_RD), 'in', \&sig_process);

# BUGFIX - BUGFIX - BUGFIX - BUGFIX - BUGFIX - BUGFIX - BUGFIX
# Appenrently, a watchdog is necessary to wake up the Gtk main loop
# periodically, because it does not react to kernel signals asynchronously...
my $sig_timeout_tag = Glib::Timeout->add(500, \&sig_timeout);


$SIG{CHLD} = \&sig_chld;
$SIG{PIPE} = 'IGNORE';
$SIG{HUP} = \&main_quit;
$SIG{QUIT} = \&main_quit;
$SIG{TERM} = \&main_quit;


sub sig_terminate {
  Glib::Source->remove($sig_timeout_tag) if ( defined $sig_timeout_tag );
  $sig_timeout_tag = undef;

  Glib::Source->remove($sig_io_tag) if ( defined $sig_io_tag );
  $sig_io_tag = undef;
}


###########################################################
# Whole Environment Scanning
###########################################################

my $rescan_tag = undef;

sub rescan {
  print STDERR "Scanning Workspaces in $user_home\n" if ( $verbose > 0 );

  fam_clear();
  suite_scan();
  system_scan();
  queue_batch_scan();

  $rescan->set_sensitive(1);

  $rescan_tag = undef;

  print STDERR "Workspace scan completed.\n" if ( $verbose > 0 );
  return 0;
}

sub rescan_clicked {
  $rescan->set_sensitive(0);
  if ($rescan_tag) {
      Glib::Source->remove($rescan_tag);
  }
  $rescan_tag = Glib::Idle->add(\&rescan);
}


###########################################################
# TVU Display Tool
###########################################################

sub vu_display_raised {
    my $arg = shift;
    my $pid = shift;
    my $status = shift;

    if ($status) {
	print STDERR "TestFarm Virtual User Display not started. Starting...\n" if ($verbose > 0);
	proc_start('testfarm-vu-display');
    }
    else {
	print STDERR "TestFarm Virtual User Display already started. Should be raised now.\n" if ($verbose > 0);
    }
}

sub vu_display_clicked {
    print STDERR "Raising TestFarm Virtual User Display...\n" if ($verbose > 0);
    proc_start('xdotool search --name "TestFarm Virtual User Display" windowraise %@', undef, \&vu_display_raised);
}


###########################################################
# Main processing loop
###########################################################

sub main_quit {
  $SIG{CHLD} = 'IGNORE';
  $SIG{HUP} = 'IGNORE';
  $SIG{QUIT} = 'IGNORE';
  $SIG{TERM} = 'IGNORE';

  sig_terminate();
  fam_terminate();
  proc_terminate();
  system_manual_ctl_close();

  Gtk2->main_quit();

  exit(0);
}

fam_init();
rescan_clicked();

Gtk2->main;
