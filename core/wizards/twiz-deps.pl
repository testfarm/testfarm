#!/usr/bin/perl -w

##
## TestFarm
## Test Suite Development Wizards
## Makefile Wizard Dependency Generator
##
## $Revision: 1121 $
## $Date: 2010-03-05 15:07:40 +0100 (ven., 05 mars 2010) $
##

use File::Basename;
use Cwd;

use TestFarm::Env;

use TestFarm::Wiz qw(
  :DEFAULT
  @wizlib
  wizlib_init
);


############################################################
# Import WIZLIB path
############################################################

wizlib_init();


############################################################
# Get options and arguments
############################################################

sub usage {
  my $rev = '$Revision: 1121 $ ';
  $rev =~ /^\$Revision: (\S+)/;
  print STDERR "TestFarm Makefile Wizard Dependency Generator - version $1\n" if defined $1;
  print STDERR "Usage: twiz-deps -tree [-wizlist <wizlist-file>] [\@]<file> ...\n";
  print STDERR "Usage: twiz-deps -wiz [\@]<file> ...\n";
  print STDERR "Usage: twiz-deps -wizdef [\@]<file> ...\n";
  exit(1);
}

usage() if ( $#ARGV < 1 );

my $mode = undef;
my $wizlist = undef;

@input_file_list = ();

for (my $i = 0; $i <= $#ARGV; $i++) {
  my $arg = $ARGV[$i];

  if ( $arg =~ /^-/ ) {
    if ( $arg eq "-wizlist" ) {
      usage() if ( $i >= $#ARGV );
      $wizlist = $ARGV[++$i];
    }
    elsif ( defined $mode ) {
      usage();
    }
    else {
      $mode = $arg;
    }
  }
  elsif ( $arg =~ /^\@(.+)$/ ) {
    my $fname = $1;
    local *FI;

    if ( open(FI, "<$fname") ) { 
      while ( <FI> ) {
        chomp;
        push @input_file_list, $_;
      }
      close(FI);
    }
    else {
      print STDERR "Cannot open list file $fname\n";
    }
  }
  else {
    push @input_file_list, $arg;
  }
}

usage() unless ( defined $mode );
exit(0) unless ( @input_file_list );


############################################################
# Process input files
############################################################

@vars = ();
@wizdefs = ();

if ( $mode eq "-tree" ) {
  local *WIZLIST;

  if ( $wizlist ) {
    unless ( open(WIZLIST, ">$wizlist") ) {
      print STDERR "Cannot create WIZ-list file $wizlist: $!\n";
      exit(2);
    }
  }

  foreach ( @input_file_list ) {
    next if -d;

    my $treename = $_;
    $treename =~ s/\.tree$//;

    check_tree("", $treename, $wizlist ? \*WIZLIST : undef);
  }

  if ( $wizlist ) {
    close(WIZLIST);
  }
}
elsif ( $mode eq "-wizdef" ) {
  foreach ( @input_file_list ) {
    my @files = ();
    if ( -d ) {
      s/\/+//;
      @files = ( <$_/*.wizdef>, <$_/*.pm> );
    }
    else {
      @files = ( $_ );
    }

    foreach ( @files ) {
      if ( /\.wizdef$/ ) {
	check_wizcall($_);
      }
      elsif ( /\.pm$/ ) {
	check_wizgen($_);
      }
    }
  }
  print "_WIZDEFS = @wizdefs\n";
}
elsif ( $mode eq "-wiz" ) {
  foreach ( @input_file_list ) {
    next if -d;
    my @deps = ();
    check_wizscript($_, \@deps);
  }
}
else {
  usage();
}


############################################################
# WIZ def dependencies
############################################################

sub add_var {
  my $id = shift;
  my $fname = shift;
  my $deps = shift;

  foreach ( @vars ) {
    return undef if ( $_ eq $id );
  }

  push @vars, $id;

  print "$id = $fname";
  if ( defined $deps ) {
    foreach ( @$deps ) {
      print " \$($_)";
    }
  }
  print "\n";

  return $id;
}


sub seek_file {
  my $basename = shift;

  foreach ( @wizlib ) {
    my $fname = $_."/".$basename;
    return $fname if ( -f $fname );
  }

  return undef;
}


sub file_path($$) {
  my $fname = shift;
  my $suffix = shift;
  my $newsuffix = shift || $suffix;

  my ($id, $dir) = fileparse($fname, ($suffix));
  $dir =~ s/\/$//;

  my $fpath = $id.$newsuffix;
  $fpath = "$dir/$fpath" unless ( ($dir eq ".") || ($dir eq "") );

  return ($id, $fpath);
}


sub check_wizcall {
  my $fname = shift;
  return 1 unless defined $fname;

  local *FI;
  return 1 unless ( open(FI, $fname) );

  my @deps = ();

  while ( <FI> ) {
    chomp;
    s/^\s*//;
    next if /^$/;
    next unless /^\#\$/;

    my $id = undef;

    if ( /^\#\$WIZCALL\s+(\S+)/ ) {
      $id = $1;
      check_wizcall(seek_file("$id.wizdef"));
    }
    elsif ( /^\#\$WIZGEN\s+(\S+)::/ ) {
      $id = $1;
      check_wizgen(seek_file("$id.pm"));
    }

    if ( defined $id ) {
      foreach ( @deps ) {
        if ( $_ eq $id ) {
          $id = undef;
          last;
        }
      }
      push @deps, $id if defined $id;
    }
  }

  close(FI);

  my ($id, $fpath) = file_path($fname, '.wizdef');
  if ( add_var($id, $fpath, \@deps) ) {
    $fname =~ s/^$PWD\///;
    $fname =~ s/^\.\///;
    push @wizdefs, $fname;
  }

  return 0;
}


sub check_wizgen {
  my $fname =  shift;

  return 1 unless defined $fname;

  my ($id, $fpath) = file_path($fname, '.pm');
  add_var($id, $fpath, undef);

  # TODO: also add .pm package dependencies

  return 0;
}


############################################################
# WIZ script dependencies
############################################################

sub add_to_deps {
  my $item = shift;
  my $deps = shift;

  foreach ( @$deps ) { return 0 if ( $_ eq $item ) }
  push @$deps, $item;
  return 1;
}


sub check_wizscript {
  my $fname = shift;
  my $deps = shift;
  my $quiet = shift;
  return 1 unless defined $fname;

  local *FI;
  return 1 unless ( open(FI, $fname) );

  my ($id, $dir, $suf) = fileparse($fname, (".wiz"));
  $dir =~ s/\/$//;
  my $pm = "$id.pm";
  $pm = "$dir/$pm" unless ( ($dir eq ".") || ($dir eq "") );

  while ( <FI> ) {
    chomp;
    s/^\s*//;
    next if /^$/;
    next if /^\#/;

    s/^(\S+)\s*//;
    my $kw = $1;

    if ( ($kw eq "INSERT_FILE") || ($kw eq "INCLUDE") ) {
      foreach ( split ) {
        my $file = "$dir/$_.wiz";
        next unless ( -e $file );
        add_to_deps($fname, $deps);
        check_wizscript($file, $deps, 1);
      }
    }
    else {
      add_to_deps("\$($kw)", $deps);
    }
  }

  close(FI);

  unless ( $quiet ) {
    add_to_deps("\$(PREAMBLE)", $deps);
    add_to_deps("\$(POSTAMBLE)", $deps);
    print "$pm: $fname @$deps\n";
  }

  return 0;
}


############################################################
# Tree dependencies
############################################################

sub check_tree {
  my $dirname = shift;
  my $treename = shift;
  my $wizlist = shift;

  if ( ($dirname ne "") && ($dirname !~ /\/$/ ) ) {
    $dirname .= "/";
  }

  my $fname = $dirname.$treename.".tree";
  unless ( -f $fname ) {
    $fname = $dirname.".tree";
  }

  local *FI;
  return 1 unless ( open(FI, $fname) );

  print $fname."\n";

  while ( <FI> ) {
    chomp;
    s/^\s*//;
    next if /^$/;
    next if /^\#/;

    if ( $wizlist ) {
      my $wizname = $dirname.$_.".wiz";
      print $wizlist $wizname."\n" if ( -f $wizname );
    }

    my $path = $dirname.$_;
    if ( -d $path ) {
      check_tree($path, $treename, $wizlist);
    }
  }

  close(FI);

  return 0;
}
