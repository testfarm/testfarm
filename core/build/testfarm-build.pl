#!/usr/bin/perl -w

##
## TestFarm
## Test Tree Builder
##
## Author: Sylvain Giroudon
## Creation: 30-AUG-2005
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
use FileHandle;
use POSIX;
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

my $banner = 'testfarm-build';


###########################################################
# Wait for cryptoperl feeder termination (if any)
###########################################################

if ( defined $__feeder__ ) {
  waitpid($__feeder__, 0);
}


###########################################################
# Get & Check arguments
###########################################################

my $verbose = 0;
my $tree_spec = undef;

sub usage {
  print STDERR "TestFarm Test Suite Builder - version $VERSION\n" if defined $VERSION;
  print STDERR "Usage: $banner [-v] [-v] [<tree-file-or-dir>]\n";
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
    if ( defined $tree_spec ) {
      usage();
    }
    else {
      $tree_spec = $arg;
    }
  }
}


fam_verbose($verbose);


###########################################################
# Debug messages
###########################################################

my $STDERR_nl = 1;

sub STDERR_msg {
  my $tag = shift;
  my $msg = "@_";
  print STDERR $tag.': ' if $STDERR_nl;
  $STDERR_nl = ($msg =~ /\n$/);
  print STDERR $msg;
}


sub _DEBUG {
  return if $verbose < 3;
  STDERR_msg('DEBUG  ', @_);
}


sub _VERBOSE {
  my $n = shift;
  return if $verbose < $n;
  STDERR_msg('INFO   ', @_);
}


sub _WARNING {
  STDERR_msg('WARNING', @_);
}


sub _ERROR {
  STDERR_msg('ERROR  ', @_);
}


sub _PANIC {
  STDERR_msg('PANIC  ', @_);
}


###########################################################
# Glade Interface definition
###########################################################

my $glade_interface_file = getcwd().'/testfarm-build.glade';

my $glade_interface = << '__GLADE_INTERFACE__';
__GLADE_INTERFACE__


sub glade_interface {
  my $root = shift;

  my $g = undef;
  if ( $glade_interface =~ /^\s*$/ ) {
    _VERBOSE(1, "Loading Glade interface from XML file $glade_interface_file\n");
    $g = Gtk2::GladeXML->new($glade_interface_file, $root);
  }
  else {
    $g = Gtk2::GladeXML->new_from_buffer($glade_interface, $root);
  }

  return $g;
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
unless ( defined $g ) {
  _PANIC("Failed to setup main window\n");
  exit(2);
}

$g->signal_autoconnect_from_package('main');

my $icon = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('testfarm.png'));
$g->get_widget('window')->set_icon($icon);

my $pixbuf_tree = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('node_tree.png'));
my $pixbuf_seq = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('node_seq.png'));
my $pixbuf_case = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('node_case.png'));
my $pixbuf_unknown = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('node_unknown.png'));
my $pixbuf_break = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('break.png'));
my $pixbuf_abort = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('abort.png'));
my $pixbuf_ignore = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('ignore.png'));
my $pixbuf_passed = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('passed.png'));
my $pixbuf_info = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('info.png'));
my $pixbuf_warning = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('warning.png'));
my $pixbuf_error = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('error.png'));
my $pixbuf_panic = Gtk2::Gdk::Pixbuf->new_from_file(find_pixmap('panic.png'));

use constant GRAY => '#666';

#my $tooltips = Gtk2::Tooltips->new();


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
# Test Suite Tree Model
###########################################################

use constant SUITE_COL_KEY        => 0;
use constant SUITE_COL_NAME       => 1;
use constant SUITE_COL_PIX        => 2;
use constant SUITE_COL_FOREGROUND => 3;
use constant SUITE_COL_FLG        => 4;
use constant SUITE_COL_FILE       => 5;
use constant SUITE_COL_IGNORE     => 6;

my $suite_model = Gtk2::TreeStore->new(qw(Glib::String Glib::String Gtk2::Gdk::Pixbuf Glib::String Gtk2::Gdk::Pixbuf Glib::String Gtk2::Gdk::Pixbuf));


###########################################################
# Retrieve Test Suite identification
###########################################################

my $tree_dir = undef;
my $tree_file = undef;
my $tree_name = undef;

# Retrieve Test Tree directory
if ( defined $tree_spec ) {
  if ( -d $tree_spec ) {
    $tree_dir = $tree_spec;
  }
  else {
    ($tree_file, $tree_dir) = fileparse($tree_spec, ('.tree'));
    $tree_file .= '.tree';
  }

  chdir($tree_dir);
}

$tree_dir = getcwd();
_VERBOSE(1, "Tree directory: $tree_dir\n");

# Retrieve Test Tree name
if ( defined $tree_file ) {
  _VERBOSE(1, "Test Suite file: $tree_file\n");
}
else {
  $tree_file = 'NewTree.tree';
  my $i = 2;
  while ( -f $tree_file ) {
    $tree_file = sprintf('NewTree%d.tree', $i++);
  }
}

$tree_name = fileparse($tree_file, ('.tree'));


###########################################################
# Retrieve available System Config files
###########################################################

my @systems = ();


sub system_number($) {
  my $id = shift || return 0;

  for (my $i = 1; $i <= $#systems; $i++) {
    return $i if ( $id eq $systems[$i][0] );
  }

  return 0;
}


sub system_scan {
  @systems = (['-', undef]);

  _VERBOSE(1, "Scanning System Configuration files: ");

  foreach my $dir ( @_ ) {
    while ( (my $file = <$dir/*.xml>) ) {
      my $basename = fileparse($file, ('.xml'));

      # Check for System Name redundancy
      next if ( system_number($basename) > 0 );

      # Check XML file is a valid System Config
      my $parser = new XML::DOM::Parser();
      my $doc = $parser->parsefile($file);
      my $root = $doc->getElementsByTagName('CONFIG', 0);
      next if ( $#$root < 0 );

      $root = undef;
      $doc->dispose();
      $parser = undef;

      _VERBOSE(1, "$basename ");
      push @systems, [ $basename, $file ];
    }
  }

  if ( $#systems <= 0 ) { _VERBOSE(1, "None found.") }
  _VERBOSE(1, "\n");
}


system_scan($tree_dir, @TestFarm::Env::libs);


###########################################################
# Node directives edition
###########################################################

my @directives = ('DESCRIPTION', 'REFERENCE', 'SYSTEM', 'CRITICITY', 'BREAK_IF_FAILED', 'ABORT_IF_FAILED');

sub directive_ok($) {
  my $key = shift;
  foreach ( @directives ) {
    return 1 if ( $key eq $_ );
  }
  return 0;
}


sub directive_changed($) {
  my $changed = shift;

  foreach my $key ( @directives ) {
    return 1 if exists $$changed{$key};
  }
  return 0;
}


sub directive_update($$) {
  my ($filename, $changed) = @_;

  # Check for directive change
  return 1 unless directive_changed($changed);

  if ( $verbose ) {
    my $msg = '';
    foreach ( keys %$changed ) {
      $msg .= ' #$'.$_;
    }
    _VERBOSE(1, "Updating directives in $filename:$msg\n");
  }

  # Open input file
  local *FI;
  unless ( open(FI, $filename) ) {
    _ERROR("Cannot open file $filename for read: $!\n");
    return 0;
  }

  # Backup old file
  my $bakfile = $filename;
  $bakfile =~ s/\/+/_/g;
  $bakfile = '/tmp/'.$$.'.{'.$bakfile.'}';
  _DEBUG("   BAKFILE: $bakfile\n");

  local *FO;
  unless ( open(FO, ">$bakfile") ) {
    _ERROR("Cannot open backup file $bakfile for write: $!\n");
    close(FI);
    return 0;
  }

  while ( <FI> ) { print FO $_ }

  close(FO);
  close(FI);

  # Open backup file as input
  local *FI;
  unless ( open(FI, $bakfile) ) {
    _ERROR("Cannot open backup file $bakfile for read: $!\n");
    return 0;
  }

  # Collect available directives (within the first 100 lines)
  my %present = ();
  while ( <FI> ) {
    last if $. > 100;
    next unless s/^\s*#\$//;
    s/\s+$//;
    s/^(\S+)\s*//;
    my $key = $1;
    if ( directive_ok($key) ) {
      s/\s+$//;
      $present{$key} = $_;
      _DEBUG("   PRESENT: $key => '$_'\n");
    }
  }

  # Update file
  unless ( open(FO, ">$filename") ) {
    _ERROR("Cannot open file $filename for write: $!\n");
    close(FI);
    return 0;
  }

  # Dump directives
  foreach my $key ( @directives ) {
    my $value = undef;

    if ( exists $$changed{$key} ) {
      $value = $$changed{$key};
      $present{$key} = $value;
    }
    elsif ( exists $present{$key} ) {
      $value = $present{$key};
    }
    else {
      next;
    }

    if ( $key eq 'CRITICITY' ) {
      next if ( $value eq '-' );
    }
    elsif ( $key eq 'SYSTEM' ) {
      next if ( $value eq '-' );
    }
    elsif ( $key eq 'BREAK_IF_FAILED' ) {
      next unless ( defined $value );
      $value = undef;
    }
    elsif ( $key eq 'ABORT_IF_FAILED' ) {
      next unless ( defined $value );
      $value = undef;
    }
    else {
      next if ( $value eq '' );
    }

    my $str = '#$'.$key;
    if ( defined $value ) { $str .= ' '.$value }
    print FO $str."\n";
    _DEBUG("   DUMP: '$str'\n");
  }

  # Copy old to new while skipping directives
  seek(FI,0,0);
  while ( <FI> ) {
    if ( /^\s*#\$(\S+)/ ) {
      next if exists $present{$1};
    }

    print FO $_;
  }

  close(FO);
  close(FI);

  # Remove backup file
  unlink($bakfile);

  return 1;
}


###########################################################
# Tree branch file edition
###########################################################

sub branch_load($) {
  my $filename = shift || return undef;

  _VERBOSE(1, "Loading branch file $filename\n");

  # Open file
  local *FI;
  unless ( open(FI, $filename) ) {
    _PANIC("Cannot open branch file $filename for read: $!\n");
    return undef;
  }

  # Load file content
  my @header = ();
  my @content = ();
  my $empty = 1;

  while ( <FI> ) {
    chomp;
    my $txt = $_;
    s/^\s+//;

    if ( $empty ) {
      if ( (!/^#~/) && ((/^$/) || (/^#/)) ) {
	push @header, $txt;
      }
      else {
	$empty = 0;
      }
    }

    if ( ! $empty ) {
      next if /^$/;
      next if /^#/ && !/^#~/;

      # Reject duplicate nodes
      foreach ( @content ) {
	if ( $_ eq $txt ) {
	  $txt = undef;
	  last;
	}
      }

      if ( defined $txt ) {
	push @content, $txt;
      }
    }
  }

  # Close file
  close(FI);

  my %source = ( FILE => $filename,
		 HEADER => \@header,
		 CONTENT => \@content
	       );
  return \%source;
}


sub branch_save($;$) {
  my ($source, $filename) = @_;

  $filename ||= $$source{FILE};
  _VERBOSE(1, "Saving branch file $filename\n");

  local *FO;
  unless ( open(FO, ">$filename") ) {
    _PANIC("Cannot open branch file $filename for write: $!\n");
    return undef;
  }

  my $header = $$source{HEADER};
  my $content = $$source{CONTENT};
  foreach ( @$header, @$content ) {
    print FO $_."\n";
  }

  close(FO);

  # Add saved filename to the saved list
  fam_touch($filename);
}


sub branch_index($$) {
  my $source = shift || return -1;
  my $id = shift || return -1;

  my $content = $$source{CONTENT};
  for (my $i = 0; $i <= $#$content; $i++) {
    my $id2 = $$content[$i];
    $id2 =~ s/^#~//;
    if ( $id eq $id2 ) {
      return $i;
    }
  }

  return -1;
}


sub branch_new_id {
  my $source = shift;

  my @ids = ();

  # Prevent duplicate id's within the target sequence
  foreach ( @_ ) {
    my $id = $_;
    my $id0 = $id;
    $id0 =~ s/(\d+)$//;
    my $n = $1 || 1;
    while ( branch_index($source, $id) >= 0 ) {
      $n++;
      $id = $id0.$n;
    }
    push @ids, $id;
  }

  return @ids;
}


sub branch_rename($$$;$) {
  my ($source, $old_id, $new_id, $ignore) = @_;

  my $content = $$source{CONTENT};
  for (my $i = 0; $i <= $#$content; $i++) {
    my $id = $$content[$i];

    my $ignore0 = ($id =~ s/^#~//);
    if ( defined $ignore ) { $ignore0 = $ignore }

    if ( ($id eq $old_id) && (defined $new_id) ) {
      if ( $ignore0 ) { $new_id = '#~'.$new_id }
      _VERBOSE(1, "Editing branch file ".$$source{FILE}.": RENAME ".$$content[$i]." -> $new_id\n");
      $$content[$i] = $new_id;
    }
  }
}


sub branch_ignore($$$) {
  my ($source, $old_id, $ignore) = @_;
  branch_rename($source, $old_id, $old_id, $ignore);
}


sub branch_delete($$) {
  my ($source, $old_id) = @_;

  my $n = branch_index($source, $old_id);
  return if ( $n < 0 );

  _VERBOSE(1, "Editing branch file ".$$source{FILE}.": DELETE $old_id\n");
  my $content = $$source{CONTENT};
  splice @$content, $n, 1;
}


sub branch_replace {
  my $source = shift;
  my $old_id = shift;

  my $n = branch_index($source, $old_id);
  return if ( $n < 0 );

  _VERBOSE(1, "Editing branch file ".$$source{FILE}.": REPLACE $old_id with (@_)\n");
  my $content = $$source{CONTENT};

  my @tail = splice @$content, $n;
  shift @tail;
  push @$content, @_, @tail;

  return $n;
}


sub branch_insert_before {
  my $source = shift;
  my $old_id = shift;

  my $n = branch_index($source, $old_id);
  my $content = $$source{CONTENT};

  # If the insertion node is not found, insert at the begining
  if ( $n < 0 ) {
    _VERBOSE(1, "Editing branch file ".$$source{FILE}.": PREPEND (@_)\n");
    unshift @$content, @_;
  }
  else {
    _VERBOSE(1, "Editing branch file ".$$source{FILE}.": INSERT (@_) BEFORE $old_id\n");
    my @tail = splice @$content, $n;
    push @$content, @_, @tail;
  }

  return $n;
}


sub branch_insert_after {
  my $source = shift;
  my $old_id = shift;

  my $n = branch_index($source, $old_id);
  my $content = $$source{CONTENT};

  # If the insertion node is not found, insert at the end
  if ( $n < 0 ) {
    _VERBOSE(1, "Editing branch file ".$$source{FILE}.": APPEND (@_)\n");
    push @$content, @_;
  }
  else {
    _VERBOSE(1, "Editing branch file ".$$source{FILE}.": INSERT (@_) AFTER $old_id\n");
    my @tail = splice @$content, $n+1;
    push @$content, @_, @tail;
  }

  return $n;
}


sub branch_swap($$$) {
  my ($source, $old_id, $delta) = @_;

  my $n = branch_index($source, $old_id);
  return if ( $n < 0 );

  _VERBOSE(1, "Editing branch file ".$$source{FILE}.": MOVE $old_id BY $delta\n");

  my $content = $$source{CONTENT};

  my $n2 = $n + $delta;
  return if ( ($n2 < 0) || ($n2 > $#$content) );

  my $tmp = $$content[$n2];
  $$content[$n2] = $$content[$n];
  $$content[$n] = $tmp;
}


###########################################################
# Test Node Display
###########################################################

my %nodes = ();

my $node_selected = undef;
my $node_frozen = 0;

my $node_name_pix = $g->get_widget('node_name_pix');
my $node_name_label = $g->get_widget('node_name_label');
#TODO: $node_name_label->set_ellipsize(3);

my $node_config_box = $g->get_widget('node_config_box');
my $node_id_entry = $g->get_widget('node_id_entry');
my $node_type_label1 = $g->get_widget('type_label1');
my $node_type_label2 = $g->get_widget('type_label2');
my $node_type_box = $g->get_widget('type_box');
my $node_type_combo = $g->get_widget('type_combo');
my $node_attr_box = $g->get_widget('attr_box');
my $node_desc_entry = $g->get_widget('desc_entry');
my $node_ref_entry = $g->get_widget('ref_entry');
my $node_case_box = $g->get_widget('case_box');
my $node_case_compile = $g->get_widget('case_compile');
my $node_failure_combo = $g->get_widget('failure_combo');
my $node_system_box = $g->get_widget('system_box');
my $node_ignore_check = $g->get_widget('ignore_check');
my $node_buttons = $g->get_widget('node_buttons');


# Populate the system selection combo
my $node_system_combo = Gtk2::ComboBox->new_text();
foreach my $entry ( @systems ) {
  $node_system_combo->append_text($$entry[0]);
}
$node_system_combo->signal_connect('changed', \&node_buttons_sensitive);
$node_system_combo->show();
$g->get_widget('system_box')->pack_start($node_system_combo, 1, 1, 0);

# Populate the criticity selection combo
my $node_criticity_combo = Gtk2::ComboBox->new_text();
foreach ( @CriticityId ) {
  $node_criticity_combo->append_text($_);
}
$node_criticity_combo->signal_connect('changed', \&node_buttons_sensitive);
$node_criticity_combo->show();
$g->get_widget('criticity_box')->pack_start($node_criticity_combo, 1, 1, 0);

# Set some buttons icon
$g->get_widget('ignore_check_pix')->set_from_pixbuf($pixbuf_ignore);

$g->get_widget('case_compile_pix')->set_from_file(find_pixmap('script-16.png'));

$icon = Gtk2::Image->new_from_file(find_pixmap('script-32.png'));
$icon->show();
$g->get_widget('build_make')->set_icon_widget($icon);


my %node_label = (
  'tree'    => 'Test Tree',
  'seq'     => 'Sequence',
  'case'    => 'Test Case',
);

my %node_pixbuf = (
  'tree'    => $pixbuf_tree,
  'seq'     => $pixbuf_seq,
  'case'    => $pixbuf_case,
  'unknown' => $pixbuf_unknown,
);

my @node_type_id = ( '', 'tree', 'pm', 'wiz' );

my @node_type_pixbuf = ( $pixbuf_unknown, $pixbuf_seq, $pixbuf_case, $pixbuf_case );


sub node_get_type($) {
  my $props = shift;

  my $type = $$props{TYPE};

  if ( (defined $type) && ($type ne '') ) {
    if ( $type eq 'tree' ) {
      unless ( $$props{ROOT} ) {
	$type = 'seq';
      }
    }
    else {
      $type = 'case';
    }
  }
  else {
    $type = 'unknown';
  }

  return $type;
}


sub node_get_label($) {
  my $props = shift;
  return $node_label{node_get_type($props)};
}


sub node_get_pixbuf($) {
  my $props = shift;
  return $node_pixbuf{node_get_type($props)};
}


sub node_get_name($) {
  my $key = shift;

  if ( $key eq '.' ) { $key = $tree_name  }
  else               { $key =~ s/\/+/::/g }

  return $key;
}


sub node_get_changed() {
  my %changed = ();

  return %changed unless defined $node_selected;

  my $props = $nodes{$node_selected};
  return %changed unless defined $props;

  my $type = $$props{TYPE};

  # Create Tree Root file if not present
  my $txt;
  if ( $node_selected eq '.' ) {
    $txt = (-f $tree_file) ? $tree_name : '';
  }
  else {
    $txt = basename($node_selected);
  }
  my $new_txt = $node_id_entry->get_text();
  $new_txt =~ s/^\s+//;
  $new_txt =~ s/\s+$//;
  if ( $new_txt eq '' ) {
    return %changed;
  }
  if ( $new_txt ne $txt ) {
    $changed{ID} = $new_txt;
  }

  # Manage ignore flag for non-root nodes
  if ( ! $$props{ROOT} ) {
    my $state = $$props{IGNORE} || 0;
    $new_state = $node_ignore_check->get_active();
    $changed{IGNORE} = $new_state if ( $new_state != $state );
  }

  if ( $type eq '' ) {
    my $n = $node_type_combo->get_active();
    $changed{TYPE} = $n if ( $n > 0 );
  }
  else {
    $txt = $$props{DESCRIPTION} || '';
    $new_txt = $node_desc_entry->get_text();
    $changed{DESCRIPTION} = $new_txt if ( $new_txt ne $txt );

    $txt = $$props{REFERENCE} || '';
    $new_txt = $node_ref_entry->get_text();
    $changed{REFERENCE} = $new_txt if ( $new_txt ne $txt );

    my $state = defined $$props{BREAK_IF_FAILED};
    my $new_state = ($node_failure_combo->get_active() == 1);
    $changed{BREAK_IF_FAILED} = $new_state if ( $new_state != $state );

    $state = defined $$props{ABORT_IF_FAILED};
    $new_state = ($node_failure_combo->get_active() == 2);
    $changed{ABORT_IF_FAILED} = $new_state if ( $new_state != $state );

    if ( $type eq 'tree' ) {
      if ( $$props{ROOT} ) {
	$txt = $$props{SYSTEM} || '-';
	my $n = $node_system_combo->get_active() || 0;
	$new_txt = $systems[$n][0];
	$changed{SYSTEM} = $new_txt if ( $new_txt ne $txt );
      }
    }
    else {
      $txt = uc($$props{CRITICITY} || '-');
      my $n = $node_criticity_combo->get_active() || 0;
      $new_txt = uc($CriticityId[$n]);
      $changed{CRITICITY} = $new_txt if ( $new_txt ne $txt );
    }
  }

  return %changed;
}


sub node_buttons_sensitive() {
  return if $node_frozen;
  my @changed = node_get_changed();
  $node_buttons->set_sensitive($#changed >= 0);
}


sub node_type_changed {
  my $combo = shift;

  my $n = $combo->get_active();
  $g->get_widget('type_pix')->set_from_pixbuf($node_type_pixbuf[$n]);

  node_buttons_sensitive();
}


sub node_rename_entry($$) {
  my ($old_key, $new_key) = @_;

  my $iter0 = suite_get_iter($old_key);

  foreach my $old_key2 ( keys %nodes ) {
    my $new_key2 = $old_key2;

    if ( ($old_key2 eq $old_key) ) {
      $new_key2 = $new_key;
    }
    else {
      next unless $new_key2 =~ s/^$old_key\//$new_key\//;
    }
    _DEBUG("   ENTRY $old_key2 -> $new_key2\n");

    # Replace node entry
    my $props = $nodes{$old_key2};
    delete $nodes{$old_key2};
    $nodes{$new_key2} = $props;

    # Update node FILE name
    my $type = $$props{TYPE};
    if ( $type ne '' ) {
      my $old_file = $$props{FILE};
      my $new_file;

      if ( $type eq 'tree' ) {
	$new_file = $new_key2.'/'.basename($old_file);
      }
      else {
	$new_file = $new_key2.'.'.$type;
      }
      _DEBUG("     FILE $old_file -> $new_file\n");
      $$props{FILE} = $new_file;
    }

    # Update node in treeview
    my $iter = suite_get_iter($old_key2, $iter0);
    _DEBUG("     VIEW ".$suite_model->get_string_from_iter($iter)."\n");
    suite_set_node($new_key2, $iter);

    return 1;
  }

  return 0;
}


sub node_rename_file($$) {
  my ($old_key, $new_key) = @_;

  # Get old node properties
  my $props = $nodes{$old_key} || return undef;

  # Rename node file/directory
  my $type = $$props{TYPE};
  if ( $type ne '' ) {
    if ( $type eq 'tree' ) {
      _VERBOSE(1, "Renaming directory $old_key -> $new_key\n");
      fam_rename($old_key, $new_key);
    }
    else {
      my $old_file = $$props{FILE};
      if ( defined $old_file ) {
	my $new_file = $new_key.'.'.$type;
	_VERBOSE(1, "Renaming file $old_file -> $new_file\n");
        rename($old_file, $new_file);
      }
    }
  }

  return $new_key;
}


sub node_rename($$) {
  my ($old_key, $new_id) = @_;

  my $old_id = basename($old_key);
  _DEBUG("NODE RENAME $old_key : $old_id -> $new_id\n");

  # Load branch file that references the node
  my $parent_key = dirname($old_key);
  my $parent = $nodes{$parent_key};
  my $source = branch_load($$parent{FILE}) || return $old_key;

  # Check for id duplicates
  if ( branch_index($source, $new_id) >= 0 ) {
    TestFarm::Dialog::error("Cannot rename node '$old_id' to '$new_id':\nNode '$new_id' already exists.\n");
    return $old_key;
  }

  # Construct new entry key
  my $new_key = $parent_key.'/'.$new_id;
  $new_key =~ s/^\.\/+//;
  _DEBUG("   KEY $old_key -> $new_key\n");

  # Rename node file
  node_rename_file($old_key, $new_key) || return $old_key;

  # Rename node entries (hash and treeview)
  while ( node_rename_entry($old_key, $new_key) ) {}

  # Rename node reference in parent .tree
  branch_rename($source, $old_id, $new_id);
  branch_save($source);

  return $new_key;
}


sub node_create_file($) {
  my $filename = shift;

  _VERBOSE(1, "Creating file $filename\n");
  local *FO;
  unless ( open(FO, ">$filename") ) {
    _PANIC("Cannot create file $filename: $!\n");
    return undef;
  }

  print FO "\n";
  print FO "## Created: ".scalar(localtime)."\n";
  print FO "## \$Revision\$\n";
  print FO "## \$Date\$\n";
  print FO "\n";

  close(FO);

  return $filename;
}


sub node_create {
  my $key = shift;
  return undef if ( $key eq '.' );

  my $type = shift;
  return undef if ( $type eq '' );

  my $filename;

  if ( $type eq 'tree' ) {
    _VERBOSE(1, "Creating directory $key\n");
    unless ( mkdir($key, 0777) ) {
      _PANIC("Cannot create directory $key: $!\n");
      return undef;
    }
    $filename = $key.'/.tree';
  }
  else {
    $filename = $key.'.'.$type;
  }

  node_create_file($filename);

  return $filename;
}



sub node_ignore {
  my $key = shift;
  my $state = shift;

  # Load branch file that references the node
  my $id = basename($key);
  my $parent_key = dirname($key);
  my $parent = $nodes{$parent_key};
  my $source = branch_load($$parent{FILE}) || return;

  branch_ignore($source, $id, $state);

  branch_save($source);
}


sub node_case_compile_clicked {
  # Get currently selected node
  return unless defined $node_selected;
  my $props = $nodes{$node_selected};
  return unless defined $props;

  my $file = $$props{FILE};
  if ( $file =~ s/\.wiz$/.pm/ ) {
    build_compile($file);
  }
}


sub node_apply_clicked {
  # Get currently selected node
  return unless defined $node_selected;
  my $props = $nodes{$node_selected};
  return unless defined $props;

  # Retrieve changed attributes
  my %changed = node_get_changed();

  # Rename node if id changed
  if ( exists $changed{ID} ) {
    if ( $node_selected eq '.' ) {
      tree_rename($changed{ID});
    }
    else {
      $node_selected = node_rename($node_selected, $changed{ID});
    }
    delete $changed{ID};
  }

  # Create node if type changed
  if ( exists $changed{TYPE} ) {
    node_create($node_selected, $node_type_id[$changed{TYPE}]);
    delete $changed{TYPE};
  }

  # Update ignore state if changed
  if ( exists $changed{IGNORE} ) {
    node_ignore($node_selected, $changed{IGNORE});
    delete $changed{IGNORE};
  }

  # Update directives if changed
  directive_update($$props{FILE}, \%changed);

  node_buttons_sensitive();
}


sub node_revert_clicked {
  node_show($node_selected);
}


sub node_show_tree {
  my $props = shift;

  $node_case_box->hide();

  $node_type_label2->set_text('');

  if ( $$props{ROOT} ) {
    $node_system_box->show();
    $node_system_combo->set_active(system_number($$props{SYSTEM}));
  }
  else {
    $node_system_box->hide();
  }
}


sub node_show_case {
  my $props = shift;

  $node_system_box->hide();
  $node_case_box->show();

  my $type = $$props{TYPE};
  $node_type_label2->set_text('(.'.$type.')');
  $node_case_compile->set_sensitive(($type eq 'wiz'));

  my $i = 0;
  my $criticity = uc($$props{CRITICITY});
  if ( defined $criticity ) {
    $i = 1;
    while ( ($i <= $#CriticityId) && ($criticity ne uc($CriticityId[$i])) ) {
      $i++;
    }
    if ( $i > $#CriticityId ) {
      $i = 0;
    }
  }

  $node_criticity_combo->set_active($i);
}


sub node_show {
  my $key = shift;

  # Set main areas sensitivity
  $node_config_box->set_sensitive(defined $key);
  $node_buttons->set_sensitive(0);

  # Retrieve selected node
  my $key2 = $key || '.';
  my $props = $nodes{$key2};

  # Set toolbar buttons sensitivity
  tree_buttons_sensitive($key, $props);
  build_buttons_sensitive();

  $node_frozen++;

  # Show node name
  $node_name_pix->set_from_pixbuf(node_get_pixbuf($props));
  $node_name_label->set_text(node_get_name($key2));

  # Show node id
  my $id = ($key2 eq '.') ? $tree_name : basename($key2);
  $node_id_entry->set_text($id);

  # Show Failure Shortcut flag
  my $n = 0;
  if ( defined $$props{ABORT_IF_FAILED} ) {
    $n = 2;
  }
  elsif ( defined $$props{BREAK_IF_FAILED} ) {
    $n = 1;
  }
  $node_failure_combo->set_active($n);

  # Show Ignore state
  $node_ignore_check->set_sensitive(! $$props{ROOT});
  $node_ignore_check->set_active($$props{IGNORE});

  my $type = $$props{TYPE} || '';
  if ( $type eq '' ) {
    $node_attr_box->hide();
    $node_system_box->hide();
    $node_type_box->show();
    $node_type_combo->set_active(0);
  }
  else {
    $node_type_box->hide();
    $node_attr_box->show();

    $node_type_label1->set_text(node_get_label($props));

    my $txt = $$props{DESCRIPTION} || '';
    $node_desc_entry->set_text($txt);

    $txt = $$props{REFERENCE} || '';
    $node_ref_entry->set_text($txt);

    if ( $type eq 'tree' ) {
      node_show_tree($props);
    }
    else {
      node_show_case($props);
    }
  }

  $node_frozen--;

  node_buttons_sensitive();
}


###########################################################
# Tree Structure Edition
###########################################################

my %tree_buttons = (
  add    => $g->get_widget('tree_add'),
  remove => $g->get_widget('tree_remove'),
  up     => $g->get_widget('tree_up'),
  down   => $g->get_widget('tree_down'),
  parent => $g->get_widget('tree_parent'),
  child  => $g->get_widget('tree_child'),
  edit   => $g->get_widget('tree_edit'),
);


sub tree_next_seq {
  my $iter = shift;

  while ( $iter ) {
    my ($child_key) = $suite_model->get($iter);
    if ( defined $child_key ) {
      my $child_type = $nodes{$child_key}{TYPE};
      if ( (defined $child_type) && ($child_type eq 'tree') ) {
	return $child_key;
      }
    }
    $iter = $suite_model->iter_next($iter);
  }

  return undef;
}


sub tree_buttons_sensitive {
  my $key = shift;
  my $props = shift;

  my %tree_buttons_state = ();
  if ( (defined $key) && (exists $nodes{$key}) ) {
    $tree_buttons_state{add} = 1;

    if ( ! $$props{ROOT} ) {
      my $parent_key = dirname($key);
      my $parent_iter = suite_get_iter($parent_key);

      my $iter1 = $suite_model->iter_children($parent_iter);
      my $path1 = $suite_model->get_path($iter1);

      my $iter = suite_get_iter($key, $parent_iter);
      my $path = $suite_model->get_path($iter);

      # Allow move-up if node is not the first one
      $tree_buttons_state{up} = $path->compare($path1);

      # Allow move-down if node is not the last one
      $iter = $suite_model->iter_next($iter);
      $tree_buttons_state{down} = (defined $iter);

      # Allow move-to-child if a sequence follows at the same level
      $tree_buttons_state{child} = (defined tree_next_seq($iter));

      $tree_buttons_state{remove} = 1;

      $tree_buttons_state{parent} = ($parent_key ne '.');
    }

    my $type = $$props{TYPE} || '';
    $tree_buttons_state{edit} = ($type ne '') && ($type ne 'tree');
  }

  foreach my $button ( keys %tree_buttons ) {
    $tree_buttons{$button}->set_sensitive($tree_buttons_state{$button} || 0);
  }
}


sub tree_rename {
  my $new_id = shift;

  # Update tree name
  my $old_id = $tree_name;
  $tree_name = $new_id;

  # Update tree file
  my $old_file = $tree_file;
  $tree_file = $new_id.'.tree';

  # Update root node in tree hash
  my $props = $nodes{'.'};
  $$props{FILE} = $tree_file;
  $nodes{'.'} = $props;

  # Update root node in tree view
  my $iter = $suite_model->get_iter_first();
  suite_set_node('.', $iter);

  # Rename Tree Root file
  if ( -f $old_file ) {
    _VERBOSE(1, "Renaming tree root file $old_file -> $tree_file\n");
    rename($old_file, $tree_file);
  }
  # Create Tree Root file if not present
  else {
    node_create_file($tree_file);
  }
}


sub tree_unref {
  my $key = shift;
  my $parent_key = dirname($key);
  my $parent = $nodes{$parent_key};
  my $source = branch_load($$parent{FILE}) || return;

  # Stop monitoring the node file
  fam_cancel($key);

  # Unreference node from its parent sequence
  branch_delete($source, basename($key));
  branch_save($source);
}


sub tree_trash {
  my $key = shift;

  my $props = $nodes{$key};
  my $type = $$props{TYPE};
  return if ( $type eq '' );

  # Get name of the file or directory to be trashed
  my $path1 = undef;
  if ( $type eq 'tree' ) {
    $path1 = $key;
    _VERBOSE(1, "Moving directory $path1 to Trash\n");
  }
  else {
    $path1 = $$props{FILE};
    _VERBOSE(1, "Moving file $path1 to Trash\n");
  }

  # In case an Empty Trash was done since starting-up
  mkdir($trash_dir, 0700);

  my $path = $trash_dir.'/'.basename($path1);
  my $path2 = $path;
  my $n = 1;
  while ( -e $path2 ) {
    $path2 = $path.' ('.$n.')';
    $n++;
  }

  rename($path1, $path2);
}


sub tree_add_clicked {
  # Get currently selected node
  return unless defined $node_selected;

  # Create Tree Root file if Test Suite is empty
  if ( ($node_selected eq '.') && (! -f $tree_file) ) {
    node_create_file($tree_file);
  }

  my $props = $nodes{$node_selected};
  return unless defined $props;

  # Get sequence to be modified:
  # - Sequence  : add node at the begining
  # - Test Case : add node just after
  my $parent_key = $node_selected;
  my $id = undef;
  if ( $$props{TYPE} ne 'tree' ) {
    $parent_key = dirname($node_selected);
    $id = basename($node_selected);
  }

  # Expand parent sequence
  suite_expand_key($parent_key);

  # Load parent sequence
  my $parent = $nodes{$parent_key};
  my $source = branch_load($$parent{FILE}) || return;

  # Choose a new id
  my ($new_id) = branch_new_id($source, 'NewNode');

  # Insert new node
  branch_insert_after($source, $id, $new_id);
  branch_save($source);

  # Update node selection
  $node_selected = $parent_key.'/'.$new_id;
  $node_selected =~ s/^\.\/+//;
}


sub tree_remove_clicked {
  # Get currently selected node
  return unless defined $node_selected;
  my $props = $nodes{$node_selected} || return;

  # Retrieve node type. Deleting a non-existing node makes no sense.
  my $type = $$props{TYPE};

  if ( $type eq '' ) {
    tree_unref($node_selected);
    $node_selected = undef;
  }
  else {
    my $name = node_get_name($node_selected);

    if ( $type eq 'tree' ) {
      my $iter = suite_get_iter($node_selected);
      if ( $suite_model->iter_has_child($iter) ) {
	$tree_remove_g = glade_interface('remove_window');
	$tree_remove_g->signal_autoconnect_from_package('main');
	$tree_remove_w = $tree_remove_g->get_widget('remove_window');
	$tree_remove_w->set_icon($icon);

	$tree_remove_g->get_widget('remove_name')->set_text($name);
      }
      else {
	TestFarm::Dialog::question("Move node $name to Trash?\n(directory $node_selected)", \&tree_remove_trash);
      }
    }
    else {
      my $file = $$props{FILE};
      TestFarm::Dialog::question("Move node $name to Trash?\n(file $file)", \&tree_remove_trash);
    }
  }
}


sub tree_remove_parent {
  my $key = $node_selected || return;

  my $id = basename($key);
  my $props = $nodes{$key};
  my $source = branch_load($$props{FILE});

  # Move child nodes to parent
  my $parent_key = dirname($key);
  my $parent = $nodes{$parent_key};
  my $parent_source = branch_load($$parent{FILE});

  my $content = $$source{CONTENT};
  my (@ids) = branch_new_id($parent_source, @$content);
  branch_replace($parent_source, $id, @ids);

  branch_save($parent_source);

  for (my $i = 0; $i <= $#ids; $i++) {
    my $old_id = $$content[$i];
    my $old_key = $key.'/'.$old_id;
    $old_key =~ s/^\.\/+//;

    my $new_id = $ids[$i];
    my $new_key = $parent_key.'/'.$new_id;
    $new_key =~ s/^\.\/+//;

    node_rename_file($old_key, $new_key);
  }

  # Stop monitoring the node file
  fam_cancel($key);

  # Move directory to trash
  tree_trash($key);

  $node_selected = undef;
  tree_remove_cancel();
}


sub tree_remove_trash {
  my $key = $node_selected || return;
  tree_unref($key);
  tree_trash($key);

  $node_selected = undef;
  tree_remove_cancel();
}


sub tree_remove_cancel {
  if ( defined $tree_remove_w ) { $tree_remove_w->destroy() }
  $tree_remove_w = undef;
  $tree_remove_g = undef;
}


sub tree_move($) {
  my $delta = shift;

  # Get currently selected node
  return undef unless defined $node_selected;

  my $id = basename($node_selected);
  my $parent_key = dirname($node_selected);
  my $parent = $nodes{$parent_key};
  my $source = branch_load($$parent{FILE}) || return;

  branch_swap($source, $id, $delta);
  branch_save($source);
}


sub tree_up_clicked {
  tree_move(-1);
}


sub tree_down_clicked {
  tree_move(+1);
}


sub tree_parent_clicked {
  # Get currently selected node
  return unless defined $node_selected;

  # Retrieve old node settings
  my $old_key = $node_selected;
  my $old_id = basename($old_key);
  my $old_parent_key = dirname($old_key);
  my $old_source = branch_load($nodes{$old_parent_key}{FILE}) || return;

  # Retrieve new node settings
  my $new_parent_key = dirname($old_parent_key);
  my $new_source = branch_load($nodes{$new_parent_key}{FILE}) || return;
  my ($new_id) = branch_new_id($new_source, $old_id);
  my $new_key = $new_parent_key.'/'.$new_id;
  $new_key =~ s/^\.\/+//;
  _DEBUG("[Parent] : $old_key -> $new_key\n");

  # Unreference node from its parent sequence
  branch_delete($old_source, $old_id);
  branch_save($old_source);

  # Insert node before its former parent sequence
  branch_insert_before($new_source, basename($old_parent_key), $new_id);
  branch_save($new_source);

  # Physically move node
  $node_selected = node_rename_file($old_key, $new_key);
}


sub tree_child_clicked {
  # Get currently selected node
  return unless defined $node_selected;

  # Retrieve old node settings
  my $old_key = $node_selected;
  my $old_id = basename($old_key);
  my $old_parent_key = dirname($old_key);
  my $old_source = branch_load($nodes{$old_parent_key}{FILE}) || return;

  # Retrieve the child sequence (first sequence following this node)
  my $iter = suite_get_iter($old_key);
  $iter = $suite_model->iter_next($iter);

  # Retrieve new node settings
  my $new_parent_key = tree_next_seq($iter) || return;
  my $new_source = branch_load($nodes{$new_parent_key}{FILE}) || return;
  my ($new_id) = branch_new_id($new_source, $old_id);
  my $new_key = $new_parent_key.'/'.$new_id;
  $new_key =~ s/^\.\/+//;
  _DEBUG("[Child] : $old_key -> $new_key\n");

  # Unreference node from its parent sequence
  branch_delete($old_source, $old_id);
  branch_save($old_source);

  # Insert node at the begining of the child sequence
  branch_insert_before($new_source, undef, $new_id);
  branch_save($new_source);

  # Physically move node
  $node_selected = node_rename_file($old_key, $new_key);
}


###########################################################
# Test Suite Tree View
###########################################################

# Tree view
my $suite_tree = $g->get_widget('suite_treeview');
$suite_tree->set_model($suite_model);

# Column 1
my $suite_column = Gtk2::TreeViewColumn->new();
$suite_column->set_title('Test Suite');

my $cell_renderer = Gtk2::CellRendererPixbuf->new;
$suite_column->pack_start($cell_renderer, FALSE);
$suite_column->add_attribute($cell_renderer, 'pixbuf', SUITE_COL_PIX);

$cell_renderer = Gtk2::CellRendererText->new;
$suite_column->pack_start($cell_renderer, TRUE);
$suite_column->add_attribute($cell_renderer, 'text', SUITE_COL_NAME);
$suite_column->add_attribute($cell_renderer, 'foreground', SUITE_COL_FOREGROUND);

$suite_column->set_sizing(GTK_TREE_VIEW_COLUMN_AUTOSIZE);
$suite_tree->append_column($suite_column);

# Column 2
$cell_renderer = Gtk2::CellRendererPixbuf->new;
$suite_tree->insert_column_with_attributes(-1, "Flg", $cell_renderer,
					   'pixbuf' => SUITE_COL_FLG);

# Column 3
$suite_column = Gtk2::TreeViewColumn->new();
$suite_column->set_title('File');

$cell_renderer = Gtk2::CellRendererText->new;
$cell_renderer->set('scale' => 0.8);
$suite_column->pack_start($cell_renderer, FALSE);
$suite_column->add_attribute($cell_renderer, 'text', SUITE_COL_FILE);
$suite_column->add_attribute($cell_renderer, 'foreground', SUITE_COL_FOREGROUND);

$cell_renderer = Gtk2::CellRendererPixbuf->new;
$suite_column->pack_end($cell_renderer, FALSE);
$suite_column->add_attribute($cell_renderer, 'pixbuf', SUITE_COL_IGNORE);

$suite_tree->append_column($suite_column);

my $suite_selection = $suite_tree->get_selection();
$suite_selection->set_mode('single');
$suite_selection->set_select_function(\&suite_select);

$suite_tree->signal_connect('row-activated', \&suite_activate );


###########################################################
# Test Suite Tree Selection
###########################################################

sub suite_select {
  my $selection = shift;
  my $model = shift;
  my $path = shift;
  my $is_currently_selected = shift;

  if ( $is_currently_selected ) {
    $node_selected = undef;
  }
  else {
    ($node_selected) = $model->get($model->get_iter($path));
    _DEBUG("NODE SELECT $node_selected\n");
  }

  node_show($node_selected);

  return 1;
}


sub suite_activate {
  my ($treeview, $path, $column) = @_;

  my $model = $treeview->get_model();
  my @row = $model->get($model->get_iter($path));

  my $file = $row[SUITE_COL_FILE] || return;

  my $key = $row[SUITE_COL_KEY];
  my $props = $nodes{$key};
  my $type = $$props{TYPE} || '';

  if ( ($type ne '') && ($type ne 'tree') ) {
    edit_file($file);
  }
}


###########################################################
# Test Suite Tree Management
###########################################################

sub suite_get_iter {
  my $key = shift || return undef;
  my $iter = shift || $suite_model->get_iter_first();

  while ( $iter ) {
    my ($key2) = $suite_model->get($iter);

    if ( $key2 ) {
      return $iter if ( $key2 eq $key );

      if ( $suite_model->iter_has_child($iter) ) {
	my $iter2 = suite_get_iter($key, $suite_model->iter_children($iter));
	return $iter2 if $iter2;
      }
    }

    $iter = $suite_model->iter_next($iter);
  }

  return undef;
}


sub suite_set_node($$) {
  my ($key, $iter) = @_;

  my $props = $nodes{$key};
  my $type = $$props{TYPE};

  # Determine which icon to display
  my $pixbuf = node_get_pixbuf($props);

  # Display node name in gray if node not implemented or ignored
  my $ignore = $$props{IGNORE};
  my $foreground = 'black';
  if ( ($type eq '') || $ignore ) {
    $foreground = GRAY;
  }

  # Display BREAK/ABORT_IF_FAILED option icon if enabled
  my $pixbuf_flg = undef;
  if ( defined $$props{ABORT_IF_FAILED} ) {
    $pixbuf_flg = $pixbuf_abort;
  }
  elsif ( defined $$props{BREAK_IF_FAILED} ) {
    $pixbuf_flg = $pixbuf_break;
  }

  # Display node
  $suite_model->set($iter,
		    SUITE_COL_KEY,        $key,
		    SUITE_COL_NAME,       node_get_name($key),
		    SUITE_COL_PIX,        $pixbuf,
		    SUITE_COL_FOREGROUND, $foreground,
		    SUITE_COL_FLG,        $pixbuf_flg,
		    SUITE_COL_FILE,       $$props{FILE},
		    SUITE_COL_IGNORE,     $ignore ? $pixbuf_ignore : undef);
}


sub suite_expand_key($) {
  my $key = shift || return undef;
  my $iter = suite_get_iter($key);
  $suite_tree->expand_to_path($suite_model->get_path($iter));
  return $iter;
}


sub suite_expand_file_comp {
  my ($model, $path, $iter, $args) = @_;

  my @tab = $model->get($iter);
  my $file = $tab[SUITE_COL_FILE];

  if ( $file && ($file eq $$args{FILE}) ) {
    $$args{KEY} = $tab[SUITE_COL_KEY];
    return 1;
  }

  return 0;
}


sub suite_expand_file($) {
  my $file = shift;

  if ( $file !~ /\.wizdef$/ ) {
    my %args = ( FILE => $file );
    $suite_model->foreach(\&suite_expand_file_comp, \%args);

    my $key = $args{KEY};
    my $iter = suite_expand_key($key);
    if ( defined $iter ) {
      $suite_selection->select_iter($iter);
    }
  }
}


sub suite_append($$) {
  my $key = shift;
  my $parent_key = shift;

  my $parent_iter = suite_get_iter($parent_key);
  _DEBUG("NODE APPEND $key to ".($parent_key || '(nil)')."\n");

  # If node already displayed, remove it
  my $iter = suite_get_iter($key, $parent_iter);
  suite_remove_node($iter);

  $iter = $suite_model->append($parent_iter);
  _DEBUG("   + $key at ".$suite_model->get_string_from_iter($iter)."\n");

  suite_set_node($key, $iter);

  return $iter;
}


sub suite_tree_file($) {
  my $key = shift;

  my $filename;
  if ( $key eq '.' ) {
    $filename = $tree_file;
  }
  else {
    $filename = $key.'/'.$tree_name.'.tree';
    unless ( -f $filename ) {
      $filename = $key.'/.tree';
      unless ( -f $filename ) {
	$filename = undef;
      }
    }
  }

  return $filename;
}


sub suite_parse_case($) {
  my $props = shift;

  my $type = $$props{TYPE};
  return unless ( ($type eq 'pm') || ($type eq 'wiz') );

  my $filename = $$props{FILE} || return undef;
  _DEBUG("PARSE CASE $filename\n");

  local *FI;
  unless ( open(FI, $filename) ) {
    _PANIC("Cannot open Test Case file $filename: $!\n");
    return;
  }

  # Delete directives
  foreach ( @directives ) { delete $$props{$_} }

  # Read directives from file
  while ( <FI> ) {
    # Parse attr directive
    if ( s/^\s*#\$(\S+)\s+// ) {
      my $attr = $1;
      s/\s+$//;
      $$props{$attr} = $_;
    }
  }

  close(FI);
}


sub suite_parse_tree($;$) {
  my $key = shift;
  my $filename = shift;

  # Open tree branch file
  unless ( defined $filename ) {
    $filename = suite_tree_file($key) || return undef;
  }
  _DEBUG("PARSE TREE $key ($filename)\n");

  local *FI;
  unless ( open(FI, $filename) ) {
    if ( $key ne '.' ) { _PANIC("Cannot open branch file $filename: $!\n") }
    return undef;
  }

  my $props = $nodes{$key};

  # Delete directives
  foreach ( @directives ) { delete $$props{$_} }

  my $dirname = dirname($filename);
  my @child_nodes = ();

  while ( <FI> ) {
    s/^\s+//;
    s/\s+$//;
    next if /^$/;

    # Parse attr directive
    if ( s/^#\$(\S+)\s+// ) {
      $$props{$1} = $_;
      next;
    }

    # Check if item is ignored
    my $ignore = s/^#~//;

    # Ignore comments
    next if /^#/;

    my %child = ( IGNORE => $ignore );

    s/^(\S+)\s*//;
    my $basename = $1;

    # Set node key
    my $child_key = $dirname.'/'.$basename;
    $child_key =~ s/^\.\/+//;

    # Reject duplicate ids in the same sequence
    my $dups = 0;
    foreach my $child_entry ( @child_nodes ) {
      if ( $$child_entry[0] eq $child_key ) {
	$dups++;
      }
    }

    if ( $dups > 0 ) {
      _WARNING("Ignoring duplicate node id '$basename' in $filename\n");
      next;
    }

    # Set node type and filename
    my $child_type = '';
    my $child_file = undef;

    if ( -d $child_key ) {
      fam_monitor($child_key, \&suite_fam);
      $child_type = 'tree';
      $child_file = suite_tree_file($child_key);
    }
    elsif ( -f $child_key.'.wiz' ) {
      $child_type = 'wiz';
      $child_file = $child_key.'.wiz';
    }
    elsif ( -f $child_key.'.pm' ) {
      $child_type = 'pm';
      $child_file = $child_key.'.pm';
    }

    _DEBUG("   CHILD $basename : key=$child_key type=$child_type\n");
    $child{TYPE} = $child_type;
    $child{FILE} = $child_file;

    suite_parse_case(\%child);

    push @child_nodes, [ $child_key, \%child ];
  }

  close(FI);

  return \@child_nodes;
}


sub suite_list_node {
  my $iter = shift || return;
  my $list = shift;

  my $child_iter = $suite_model->iter_children($iter);
  while ( $child_iter ) {
    suite_list_node($child_iter, $list);
    $child_iter = $suite_model->iter_next($child_iter);
  }

  push @$list, $iter;
}


sub suite_remove_node($;$) {
  my $iter = shift || return;
  my $keep_key = shift;

  # Get all nodes in this branch
  my @list = ();
  suite_list_node($iter, \@list);

  foreach $iter ( @list ) {
    unless ( $keep_key ) {
      my ($key) = $suite_model->get($iter);
      delete $nodes{$key};
      _DEBUG("   - $key at ".$suite_model->get_string_from_iter($iter)."\n");
    }

    $suite_model->remove($iter);
  }
}


sub suite_cleanup_node($$) {
  my $iter = shift;
  my $child_nodes = shift;

  my @removed = ();

  # Collect removed nodes
  my $child_iter = $suite_model->iter_children($iter);
  while ( $child_iter ) {
    my ($child_key) = $suite_model->get($child_iter);

    my $found = 0;
    foreach ( @$child_nodes ) {
      my ($child_key2) = @$_;
      if ( $child_key2 eq $child_key ) {
	$found = 1;
	last;
      }
    }

    if ( ! $found ) {
      _DEBUG("   $child_key not found in the list\n");
      push @removed, $child_iter;
    }

    $child_iter = $suite_model->iter_next($child_iter);
  }

  # Delete removed nodes
  foreach my $child_iter ( @removed ) {
    suite_remove_node($child_iter);
  }
}


sub suite_remove($) {
  my $key = shift;

  my $iter = suite_get_iter($key);
  suite_remove_node($iter);
}


sub suite_scan {
  my $key = shift;

  _DEBUG("SCAN $key\n");

  # Parse Tree Branch file
  my $child_nodes = suite_parse_tree($key);

  foreach ( @$child_nodes ) {
    my ($child_key, $child) = @$_;

    $nodes{$child_key} = $child;
    suite_append($child_key, $key);

    if ( $$child{TYPE} eq 'tree' ) {
      suite_scan($child_key);
    }
  }
}


###########################################################
# Test Suite File events
###########################################################

sub suite_fam_update_tree($) {
  my $key = shift;

  # Parse the new node
  my $child_nodes = suite_parse_tree($key);

  my $iter = suite_get_iter($key);
  my $position = $suite_model->get_string_from_iter($iter);
  _DEBUG("UPDATE TREE $key at $position\n");

  # Cleanup deleted nodes from tree view
  suite_cleanup_node($iter, $child_nodes);

  my $modified = 1;
  while ( $modified ) {
    $modified = 0;

    for (my $i = 0; $i <= $#$child_nodes; $i++) {
      my ($child_key, $child) = @{$$child_nodes[$i]};

      $nodes{$child_key} = $child;

      my $child_iter = suite_get_iter($child_key, $iter);

      if ( defined $child_iter ) {
	my $old_pos = $suite_model->get_string_from_iter($child_iter);
	my $new_pos = "$position:$i";

	if ( $old_pos eq $new_pos ) {
	  _DEBUG("   => EXIST $child_key at $old_pos\n");
	  suite_set_node($child_key, $child_iter);
	}
	else {
	  suite_remove_node($child_iter, 1);
	  $child_iter = $suite_model->insert($iter, $i);
	  suite_set_node($child_key, $child_iter);
	  _DEBUG("   => MOVE $child_key from $old_pos to $new_pos (".$suite_model->get_string_from_iter($child_iter).")\n");
	  suite_scan($child_key);
	  $modified = 1;
	}
      }
      else {
	$child_iter = $suite_model->insert($iter, $i);
	suite_set_node($child_key, $child_iter);
	_DEBUG("   => INSERT $child_key at ".$suite_model->get_string_from_iter($child_iter)."\n");
	suite_scan($child_key);
	$modified = 1;
      }

      last if $modified;
    }
  }

  # Refresh node selection
  if ( defined $node_selected ) {
    if ( exists $nodes{$node_selected} ) {
      my $iter = suite_get_iter($node_selected);

      my $iter0 = $suite_model->iter_parent($iter);
      if ( defined $iter0 ) {
	$suite_tree->expand_to_path($suite_model->get_path($iter0));
      }

      $suite_selection->select_iter($iter);
    }
    else {
      $suite_selection->unselect_all();
    }
  }
}


sub suite_fam_update_case($) {
  my $key = shift;
  _DEBUG("UPDATE CASE $key\n");

  suite_parse_case($nodes{$key});

  my $iter = suite_get_iter($key);
  suite_set_node($key, $iter);
}


sub suite_fam_update($) {
  my $key = shift || return;

  # Check the node has been parsed and referenced
  my $props = $nodes{$key};
  return unless defined $props;
  my $type = $$props{TYPE};

  # Update node with its proper type method
  if ( $type eq 'tree' ) {
    suite_fam_update_tree($key);
  }
  else {
    suite_fam_update_case($key);
  }
}


sub suite_fam_create($) {
  my $key = shift;
  return if ( $key eq '.' );

  # Check the node is referenced
  my $props = $nodes{$key};
  return unless defined $props;

  # Process file creation only if the event is related to a no-type reference
  return unless ( $$props{TYPE} eq '' );

  # Realize file creation as a replacement of the no-type reference
  _VERBOSE(1, "File $key created\n");

  my $parent_key = dirname($key);
  suite_fam_update($parent_key);

  $props = $nodes{$key};
  if ( $$props{TYPE} eq 'tree' ) {
    suite_scan($key);
  }
}


sub suite_fam_delete($) {
  my $key = shift;
  return if ( $key eq '.' );

  suite_remove($key);

  my $parent_key = dirname($key);
  suite_fam_update($parent_key);
}


sub suite_fam {
  my $arg = shift;
  my $events = shift;

  foreach my $ev ( @$events ) {
    my $type = $$ev{type};
    my $monitor = $$ev{monitor};
    my $file = $$ev{file};

    next if $file =~ /~$/;     # Ignore backup files
    next if $file =~ /^\.#/;  # Ignore Emacs temp files

    my ($basename, $dirname, $suffix) = fileparse($file, ('.tree', '.pm', '.wiz'));

    my $key = $monitor;
    $key =~ s/^$tree_dir//;
    $key =~ s/^\.\/*//;
    if ( $suffix ne '.tree' ) {
      $key .= '/' if $key ne '';
      $key .= $basename;
    }
    else {
      if ( $key eq '' ) {
	$key = '.';
      }
    }

    _DEBUG("FAM $type $key\n");

    if ( $type eq 'change' ) {
      suite_fam_update($key);
    }
    elsif ( ($type eq 'exist') || ($type eq 'create') ) {
      suite_fam_create($key);
    }
    elsif ( $type eq 'delete' ) {
      suite_fam_delete($key);
    }

    # Refresh properties display if node is selected
    node_show($node_selected);
  }
}


###########################################################
# Initial Test Suite Creation
###########################################################

sub suite_init() {
  # Init FAM monitoring
  fam_clear();
  fam_monitor('.', \&suite_fam);

  # Clear tree hash
  %nodes = ();

  # Create root node
  my %props = ( TYPE => 'tree', FILE => $tree_file, ROOT => 1 );
  $nodes{'.'} = \%props;

  suite_append('.', undef);
  suite_scan('.');
  suite_expand_key('.');

  node_show(undef);
}


###########################################################
# Editor subprocess management
###########################################################

my %edit_procs = ();

sub edit_file {
  my $file = shift;
  my $lineno = shift || 1;

  my $edit = 'testfarm-edit.sh';

  my $pid = fork();
  if ( $pid ) {
    $edit_procs{$pid} = $file;
    _VERBOSE(1, "Edit Script: [$pid] $edit file='$file' lineno=$lineno\n");
  }
  else {
    exec($edit, $file, $lineno);
  }
}


sub edit_clicked {
  return unless defined $node_selected;
  my $props = $nodes{$node_selected};
  return unless defined $props;
  edit_file($$props{FILE});
}


sub edit_terminate {
  foreach my $pid ( keys %edit_procs ) {
    kill('TERM', $pid);
  }
  %edit_procs = ();
}


sub edit_sigchld($) {
  my $pid = shift;
  if ( exists $edit_procs{$pid} ) {
    delete $edit_procs{$pid};
    _VERBOSE(1, "Edit Script: [$pid] Done\n");
  }
}


sub edit_running {
  my @n = keys %edit_procs;
  return ($#n >= 0);
}


###########################################################
# Build subprocess management
###########################################################

# Console tree model
use constant BUILD_COL_ICON       => 0;
use constant BUILD_COL_TEXT       => 1;
use constant BUILD_COL_FOREGROUND => 2;
use constant BUILD_COL_FILENAME   => 3;
use constant BUILD_COL_LINENO     => 4;

use constant BUILD_SCROLL_LENGTH  => 1000;

my $build_model = Gtk2::TreeStore->new(qw(Gtk2::Gdk::Pixbuf Glib::String Glib::String Glib::String Glib::String));

# Console tree view
my $build_tree = $g->get_widget('build_treeview');
$build_tree->set_model($build_model);

# Column 1
$cell_renderer = Gtk2::CellRendererPixbuf->new();
$build_tree->insert_column_with_attributes(-1, "Lvl", $cell_renderer,
					   'pixbuf' => BUILD_COL_ICON);
$build_tree->get_column(0)->set_sizing(GTK_TREE_VIEW_COLUMN_AUTOSIZE);
$build_tree->get_column(0)->set_resizable(0);

# Column 2
$cell_renderer = Gtk2::CellRendererText->new();
my $n = $build_tree->insert_column_with_attributes(-1, "Message", $cell_renderer,
						   'text'       => BUILD_COL_TEXT,
						   'foreground' => BUILD_COL_FOREGROUND);
$build_tree->set_expander_column($build_tree->get_column($n));

my $build_selection = $build_tree->get_selection();
$build_selection->set_mode('single');
$build_selection->set_select_function(\&build_select);

$build_tree->signal_connect('row-activated', \&build_activate );


my @build_highlights = (
  ' is up to date',
  'Nothing to be done for ',
);

my @build_warnings = (
  ' used only once:',
  'Warning: ',
  'Found = in conditional',
  'masks earlier declaration in same scope',
);


sub last_row_visible {
  my $tree = shift;
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


sub build_dump {
  my $color0 = shift;

  # Get last row visibility
  my $scroll = last_row_visible($build_tree);

  # Show lines on console
  my $iter = undef;
  foreach ( @_ ) {
    my $msg = $_;
    $msg =~ s/\s+$//;

    # Append new message row
    $iter = $build_model->append(undef);

    # Extract location prefix
    my $prefix = '';
    if ( $msg =~ s/^(.+):(\d+): +// ) {
      my $filename = $1;
      my $lineno = $2;
      $prefix = $filename.':'.$lineno.': ';

      # Set file location
      $build_model->set($iter, BUILD_COL_FILENAME, $filename);
      $build_model->set($iter, BUILD_COL_LINENO, $lineno);
    }

    # Determine message enlightment
    my $color = $color0;
    my $pixbuf = undef;

    if ( $msg =~ s/^make(\[\d+\])*:\s*// ) {
      $color = GRAY;
      if ( $msg =~ /^\*\*\*/) {
	$pixbuf = $pixbuf_error;
	$color = 'darkred';
      }
    }
    elsif ( $msg =~ s/^\[(\S+)\] // ) {
      my $tag = $1;
      if ( $tag eq 'INFO' ) {
	$pixbuf = $pixbuf_info;
	$color = undef;
      }
      elsif ( $tag eq 'WARNING' ) {
	$pixbuf = $pixbuf_warning;
      }
      elsif ( $tag eq 'ERROR' ) {
	$pixbuf = $pixbuf_error;
      }
      elsif ( $tag eq 'PANIC' ) {
	$pixbuf = $pixbuf_panic;
	$color = 'darkred';
      }
    }
    elsif ( $msg =~ /^---\s/) {
      $pixbuf = $pixbuf_info;
      $color = undef;
    }
    elsif ( $msg =~ /^\*\*\*/) {
      $pixbuf = $pixbuf_error;
      $color = 'darkred';
    }
    elsif ( $msg =~ / syntax OK$/ ) {
      $pixbuf = $pixbuf_passed;
      $color = 'darkgreen';
    }
    elsif ( $msg =~ / had compilation errors.$/ ) {
      $pixbuf = $pixbuf_error;
      $color = 'darkred';
    }

    # Refine prefixed message enlightment
    if ( $prefix ) {
      unless ( defined $pixbuf ) {
	$pixbuf = $pixbuf_error;
	foreach my $pattern ( @build_warnings ) {
	  if ( $msg =~ /$pattern/ ) {
	    $pixbuf = $pixbuf_warning;
	    last;
	  }
	}
      }
    }

    foreach my $pattern ( @build_highlights ) {
      if ( $msg =~ /$pattern/ ) {
	$color = undef;
	last;
      }
    }

    if ( defined $pixbuf ) {
      $build_model->set($iter, BUILD_COL_ICON, $pixbuf);
    }

    $build_model->set($iter, BUILD_COL_TEXT, $prefix.$msg);

    if ( defined $color ) {
      $build_model->set($iter, BUILD_COL_FOREGROUND, $color);
    }

    # Clamp console length
    my $n = $build_model->get_string_from_iter($iter);
    if ( $n > BUILD_SCROLL_LENGTH ) {
      my $iter0 = $build_model->get_iter_from_string('0');
      $build_model->remove($iter0);
    }
  }

  $build_tree->realize();

  # Scroll down if last row was visible
  if ( (defined $iter) && $scroll ) {
    my $path = $build_model->get_path($iter);
    $build_tree->scroll_to_cell($path, undef, 1, 1.0, 0.0);
  }
}


sub build_mkmf {
  my @msg = `twiz-makefile -v 2>&1`;
  build_dump(undef, @msg);
}


sub build_check {
  my $rev = undef;

  # Check Makefile seniority compared to the Wizard revision
  if ( -f 'Makefile' ) {
    my $tree_found = 0;

    local *FI;
    if ( open(FI, 'Makefile') ) {
      while ( <FI> ) {
	s/\s+$//;
	if ( /^MAKEFILE_REV = (.+)$/ ) {
	  $rev = $1;
	}
	elsif ( /^TARGETS = (.+)$/ ) {
	  my @targets = split /\s+/, $1;
	  foreach ( @targets ) {
	    if ( $_ eq $tree_name ) {
	      $tree_found = 1;
	      last;
	    }
	  }
	}
      }
      close(FI);
    }

    if ( $tree_found ) {
      my $rev0 = `twiz-makefile -rev`;
      if ( $rev0 ) {
	if ( (! defined $rev) || ($rev != $rev0) ) {
	  build_dump(undef, '[WARNING] Makefile version is outdated: Regenerating one');
	  $rev = undef;
	}
      }
      else {
	build_dump(undef, '[PANIC] Cannot retrieve Makefile version number');
	$rev = undef;
      }
    }
    else {
      build_dump(undef, "[WARNING] Cannot find target '$tree_name' in Makefile: Regenerating one");
      $rev = undef;
    }
  }
  else {
    build_dump(undef, '[WARNING] No Makefile found. Generating one');
  }

  # If needed, remake the Makefile
  unless ( defined $rev ) {
    build_mkmf();
  }
}


local *BUILD_STDIN;
local *BUILD_STDOUT;
local *BUILD_STDERR;
my $build_pid = undef;
my $build_stdout_tag = undef;
my $build_stderr_tag = undef;


sub build_stdout {
  my $line;
  while ( defined ($line = BUILD_STDOUT->gets()) ) {
    chomp($line);
    #print STDERR "-- STDOUT : '$line'\n";
    build_dump(GRAY, $line);
  }

  return 1;
}


sub build_stderr {
  my $line;
  while ( defined ($line = BUILD_STDERR->gets()) ) {
    chomp($line);
    #print STDERR "-- STDERR : '$line'\n";
    build_dump(undef, $line);
  }

  return 1;
}


sub build_sigchld($) {
  my $pid = shift || return;
  return if (! defined $build_pid) || ( $pid != $build_pid );
  _VERBOSE(1, "Build Process: [$pid] Done\n");

  Glib::Source->remove($build_stdout_tag);
  $build_stdout_tag = undef;

  Glib::Source->remove($build_stderr_tag);
  $build_stderr_tag = undef;

  build_stderr();
  build_stdout();

  close(BUILD_STDIN);
  close(BUILD_STDOUT);
  close(BUILD_STDERR);

  $build_pid = undef;

  build_buttons_sensitive();
}


sub build_make {
  $ENV{TREENAME} = $tree_name;

  my @cmd = ('make');
  if ( @_ ) {
    push @cmd, @_;
  }

  build_dump(undef, "[INFO] Running '@cmd'");

  # Spawn process w/ redirected i/o
  $build_pid = open3(*BUILD_STDIN, *BUILD_STDOUT, *BUILD_STDERR, @cmd);
  if ( $build_pid ) {
    # Setup proc i/o handling
    BUILD_STDOUT->autoflush(1);
    BUILD_STDOUT->blocking(0);
    BUILD_STDERR->autoflush(1);
    BUILD_STDERR->blocking(0);
    $build_stdout_tag = Glib::IO->add_watch(fileno(BUILD_STDOUT), 'in', \&build_stdout);
    $build_stderr_tag = Glib::IO->add_watch(fileno(BUILD_STDERR), 'in', \&build_stderr);
  }
  else {
    build_dump(undef, "[PANIC] Cannot start process '@cmd': $!");
  }

  build_buttons_sensitive();
}


sub build_compile {
  $build_model->clear();
  build_check();
  build_make(@_);
}


sub build_make_clicked {
  build_compile();
}


sub build_update_clicked {
  $build_model->clear();
  build_mkmf();
}


sub build_clean_clicked {
  $build_model->clear();
  build_make('clean');
}


sub build_select {
  my $selection = shift;
  my $model = shift;
  my $path = shift;
  my $is_currently_selected = shift;

  my @row = $model->get($model->get_iter($path));

  return $row[BUILD_COL_FILENAME] ? 1:0;
}


sub build_activate {
  my ($treeview, $path, $column) = @_;

  my $model = $treeview->get_model();
  my @row = $model->get($model->get_iter($path));

  my $filename = $row[BUILD_COL_FILENAME] || return;
  my $lineno = $row[BUILD_COL_LINENO];

  suite_expand_file($filename);
  edit_file($filename, $lineno);
}


my $build_buttons = $g->get_widget('build_toolbar');

sub build_buttons_sensitive {
  my $st = (! defined $build_pid) && (defined $tree_file) && (-f $tree_file);
  $build_buttons->set_sensitive($st)
}


###########################################################
# Main processing loop
###########################################################

sub sig_chld {
  # Collect pid's of terminated children
  while ( 1 ) {
    my $pid = waitpid(-1, WNOHANG);
    last if ( $pid <= 0 );
    edit_sigchld($pid);
    build_sigchld($pid);
  }
}

sub main_quit {
  $SIG{CHLD} = 'IGNORE';
  $SIG{HUP} = 'IGNORE';
  $SIG{QUIT} = 'IGNORE';
  $SIG{TERM} = 'IGNORE';

  edit_terminate();
  fam_terminate();

  Gtk2->main_quit();

  exit(0);
}

$SIG{CHLD} = \&sig_chld;

fam_init();
suite_init();

build_buttons_sensitive();

Gtk2->main;
