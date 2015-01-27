##
## TestFarm
## FAM (File Alteration Monitor) Library
## (C) Basil Dev 2006
##
## $Revision: 371 $
## $Date: 2007-02-26 18:58:01 +0100 (lun., 26 févr. 2007) $
##

package TestFarm::FAM;
#use strict;

use File::Basename;
use SGI::FAM;

require Exporter;
@ISA = qw(Exporter);

@EXPORT = qw(
  fam_init
  fam_terminate
  fam_clear
  fam_monitor
  fam_cancel
  fam_rename
  fam_touch
  fam_verbose
);


my %fams = ();
my $fam_io_tag = undef;
my $fam_timeout_tag = undef;
my $fam_idle_tag = undef;
my @fam_touched = ();
my $fam_verbose = 0;
my $fam_lock = 0;

my $FAM = undef;


sub _VERBOSE {
  if ( $fam_verbose >= 2 ) {
    print STDERR "FAM    : @_\n";
  }
}


sub fam_verbose(;$) {
  my $st = shift;
  if ( defined $st ) {
    $fam_verbose = $st;
  }
  return $fam_verbose;
}


sub fam_init {
  $FAM = eval('new SGI::FAM');
  if ( $@ ) {
    chomp(my $message = $@);
    $message =~ s/\s+at .+ line 1$//;
    _PANIC("Cannot start FAM engine: $message\n");
    exit(2);
  }
  else {
    $fam_io_tag = Glib::IO->add_watch($FAM->{conn}->fd, 'in', \&fam_check);
    $fam_timeout_tag = Glib::Timeout->add(1000, \&fam_check);
  }
}


sub fam_add_list {
  my ($list, $monitor, $file, $type) = @_;

  # Check for non-redundancy of FAM event
  foreach my $ev ( @$list ) {
    if ( ($$ev{monitor} eq $monitor) && ($$ev{file} eq $file) && ($$ev{type} eq $type) ) {
      return 0;
    }
  }

  # Add event to list
  _VERBOSE("EVENT monitor='$monitor' type='$type' file='$file'");
  my %ev = ( type => $type, monitor => $monitor, file => $file );
  push @$list, \%ev;

  return 1;
}


sub fam_touch {
  my $filename = shift;

  foreach ( @fam_touched ) {
    if ( $_ eq $filename ) {
      $filename = undef;
      last;
    }
  }
  if ( defined $filename ) {
    fam_idle_start();
    push @fam_touched, $filename;
  }
}


sub fam_idle_start {
  unless ( defined $fam_idle_tag ) {
    $fam_idle_tag = Glib::Idle->add(\&fam_idle);
  }
  return 1;
}


sub fam_idle {
  fam_check();
  $fam_idle_tag = undef;
  return 0;
}


sub fam_check {
  return 1 unless defined $FAM;

  return 1 if $fam_lock;
  $fam_lock = 1;

  # Collect FAM events
  my @list = ();
  while ( $FAM->pending ) {
    my $event = $FAM->next_event;
    my $type = $event->type;
    if ( ($type eq 'create') ||
	 ($type eq 'exist') ||
         ($type eq 'delete') ||
	 ($type eq 'change') ) {
      my $monitor = $FAM->which($event);
      my $file = $event->filename;

      my $fam = $fams{$monitor};
      next unless ( defined $fam );

      fam_add_list(\@list, $monitor, $file, $type);
    }
  }

  # Collect possibly lost events
  foreach my $file ( @fam_touched ) {
    my $monitor = dirname($file);
    fam_add_list(\@list, $monitor, basename($file), 'change');
  }
  @fam_touched = ();

  # Process FAM events
  foreach my $ev ( @list ) {
    my $monitor = $$ev{monitor};
    my $fam = $fams{$monitor};
    &{$$fam{handler}}($$fam{arg}, [$ev]);
  }

  $fam_lock = 0;
  return 1;
}


sub fam_monitor($$;$) {
  return unless defined $FAM;

  my $monitor = shift;
  my $handler = shift;
  my $arg = shift;

  unless ( exists $fams{$monitor} ) {
    _VERBOSE("MONITOR monitor='$monitor'");
    $FAM->monitor($monitor);
  }
  my %fam = ( 'handler' => $handler, 'arg' => $arg );
  $fams{$monitor} = \%fam;
}


sub fam_cancel {
  return 0 unless defined $FAM;

  my $monitor = shift;

  if ( exists $fams{$monitor} ) {
    _VERBOSE("CANCEL monitor='$monitor'");
    $FAM->cancel($monitor);
    delete $fams{$monitor};
    return 1;
  }

  return 0;
}


sub fam_rename($$) {
  return unless defined $FAM;

  my ($old_dir, $new_dir) = @_;

  my %renamed = ();

  # Get directory and its children
  foreach my $key ( keys %fams ) {
    my $key2 = $key;

    if ( $key2 eq $old_dir ) {
      $key2 = $new_dir;
    }
    else {
      next unless $key2 =~ s/^$old_dir\//$new_dir\//;
    }

    _VERBOSE("RENAME $key1 -> $key2");

    $FAM->cancel($key);
    $renamed{$key2} = $fams{$key};
    delete $fams{$key};
  }

  # Rename directory
  rename($old_dir, $new_dir);

  # Monitor renamed directory and children
  foreach my $key ( keys %renamed ) {
    $FAM->monitor($key);
    $fams{$key} = $renamed{$key};
  }
}


sub fam_clear {
  return unless defined $FAM;

  foreach ( keys %fams ) {
    $FAM->cancel($_);
  }

  %fams = ();
}


sub fam_terminate {
  Glib::Source->remove($fam_timeout_tag) if defined $fam_timeout_tag;
  $fam_timeout_tag = undef;

  Glib::Source->remove($fam_io_tag) if defined $fam_io_tag;
  $fam_io_tag = undef;

  fam_clear();

  $FAM = undef;
}

1;
