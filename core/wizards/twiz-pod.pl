#!/usr/bin/perl -w

##
## TestFarm
## Test Suite Development Wizards
## WIZdef Document Generator
##
## $Revision: 250 $
## $Date: 2006-10-03 16:27:18 +0200 (mar., 03 oct. 2006) $
##

use File::Basename;
use Cwd;

use TestFarm::Env;

use TestFarm::Wiz qw(
  :DEFAULT
  @wizlib
  wizlib_init
  $verbose
);


############################################################
# Get options and arguments
############################################################

my $html = 0;

sub usage {
  my $rev = '$Revision: 250 $ ';
  $rev =~ /^\$Revision: (\S+)/;
  print STDERR "TestFarm Wizards Document Generator - version $1\n" if defined $1;
  print STDERR "Usage: twiz-doc [-v] [-html]\n";
  exit(1);
}

foreach my $arg ( @ARGV ) {
  if ( $arg eq "-v" ) {
    $verbose++;
  }
  if ( $arg eq "-html" ) {
    $html = 1;
  }
  else {
    usage
  }
}


############################################################
# Import WIZLIB path
############################################################

wizlib_init();
INFO2(1, "WIZ definition files path: @wizlib");


############################################################
# Process wizdefs
############################################################

foreach my $dir ( @wizlib ) {
  next unless -d $dir;
  $dir =~ s/^$PWD\///;
  $dir =~ s/^\.\///;
  INFO2(1, "Scanning directory: $dir");

  my @files = ();
  local *FD;
  opendir(FD, $dir);
  while ( defined (my $file = readdir(FD)) ) {
    next if $file !~ /^(.+)\.wizdef$/;
    next if ( ($1 eq "PREAMBLE") || ($1 eq "POSTAMBLE") );
    $file = "$dir/$file";
    next unless -e $file;
    push @files, $file;
  }
  closedir(FD);

  foreach ( sort @files ) {
    gen_pod($_, $html);
  }
}


############################################################
# Generate POD documentation
############################################################

sub get_wizdef {
  my $filein = shift;

  local *FI;
  unless ( open(FI, $filein) ) {
    ERROR("Couldn't open input file '$filein': $!");
    return ();
  }

  my $summary = "";
  my @params = ();
  my @pods = ();
  my $pod_in_progress = 0;

  while ( <FI> ) {
    unless ( $pod_in_progress ) {
      if ( s/^\s*\#\$// ) {
        chomp;
        if ( /^PARAM\s+(\S+)\s+(\S+)\s+(.+)$/ ) {
          push @params, [$1, $2, $3] unless ( $1 eq "..." );
        }
        elsif ( /^DESCRIPTION\s+(.+)$/ ) {
          $summary = $1;
        }
      }
      elsif ( /^=/ ) {
        $pod_in_progress = 1;
      }
    }

    if ( $pod_in_progress ) {
      push @pods, $_;
      $pod_in_progress = 0 if /^=cut\s*$/;
    }
  }

  close(FI);

  return ($summary, \@params, \@pods);
}


sub gen_pod {
  my $filein = shift;
  my $html = (shift) ? 1:0;

  my ($summary, $params, $pods) = get_wizdef($filein);
  return -1 unless defined $summary;

  my $id = fileparse($filein, (".wizdef"));

  local *FO = \*STDOUT;

  print FO "=head1 ";
  print FO "NAME\n\n" unless $html;
  print FO $id;
  if ( $summary ) {
    if ( $html ) { print FO "\n\n" }
    else { print FO " - " }
    print FO $summary;
  }
  print FO "\n\n";

  print FO "=head", $html+1, " SYNOPSIS\n\n";
  print FO "=for html <TT>\n\n";
  print FO "$id";
  foreach ( @$params ) {
    my ($id, $values) = @$_;
    if ( $id =~ /^\[(.+)\]$/ ) {
      print FO " [I<$1>]";
    }
    else {
      print FO " I<$id>";
    }
    if ( $values eq "..." ) {
      print FO " ...";
      last;
    }
  }
  print FO "\n\n";
  print FO "=for html </TT>\n\n";

  if ( @$params ) {
    print FO "=head", $html+1, " PARAMETERS\n\n";
    print FO "=over\n\n";
    foreach ( @$params ) {
      my ($id, $values, $descr) = @$_;
      my @values_list =  split /,/, $values;

      my $optional = 0;
      if ( $id =~ /^\[(.+)\]$/ ) {
        $optional = 1;
        $id = $1;
      }

      print FO "=item I<$id>";
      print FO " (optional)" if $optional;
      print FO "\n\n";
      print FO "$descr\n\n";

      my $text = "Accepted value";
      $text .= "s" if ( $#values_list > 0 );
      $text .= ": ";

      for (my $i = 0; $i <= $#values_list; $i++ ) {
        my $v = $values_list[$i];
        $text .= ", " if ( $i > 0 );
        if ( $v =~ /^%d/ ) {
          $text .= "integer";
          if ( $v =~ s/^%d:// ) {
            my ($min, $max) = split "-", $v;
            $text .= "[$min-$max]";
          }
        }
        elsif ( ($v eq "%s") || ($v eq "...") ) {
          $text .= "string";
        }
        elsif ( $v =~ /^\// ) {
          $text .= "regex=$v";
        }
        else {
          $text .= "\"B<$v>\"";
          if ( $optional && ($i == 0) ) {
            $text .= " (default)";
          }
        }
      }
      print FO $text, "\n\n";
    }
    print FO "=back\n\n";
  }

  foreach ( @$pods ) {
    if ( $html && s/^=head(\d+)// ) {
      print FO "=head", $1+$html, $_;
    }
    else {
      print FO $_;
    }
  }

  print FO "\n";

  unless ( $html ) {
    print FO "=pod\n\n";
    print FO "=head1 -----------------------\n\n";
    print FO "=head1\n\n";
    print FO "=cut\n\n";
  }

  return 0;
}
