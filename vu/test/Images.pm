package Images;

$dirname = '.';


sub SearchFile {
  my $basename = shift;

  $dirname =~ s/\/+$//;

  my $file = 'objects/';
  if ( (defined $dirname) && ($dirname ne '.') ) {
    $file .= $dirname.'/';
  }
  $file .= $basename;

  unless ( -f $file ) {
    $file = 'objects/'.$basename;
  }
  unless ( -f $file ) {
    $file = '';
  }

  return $file;
}


sub RetrieveImage {
  my $object = shift;

  my $basename = $object.'.ppm.gz';
  my $file = SearchFile($basename);
  if ( $file eq '' ) {
    print "Cannot find image file \"$basename\"\n";
  }

  return $file;
}


sub RetrieveMask {
  my $object = shift;
  return SearchFile($object.'-mask.ppm.gz');
}

sub RetrieveOptions {
  my $object = shift;
  my $file = SearchFile($object.'-options');

  my $options = '';

  if ( $file ) {
    local *OPTS;
    if ( open(OPTS, $file) ) {
      while ( <OPTS> ) {
	s/^\s+//;
	s/\s+$//;
	next if /^$/;
	next if /^#/;
	$options .= $_.' ';
      }
      close(OPTS);
    }
    else {
      print STDERR "Cannot open Image options file $options: $!\n";
      return undef;
    }
  }

  return $options;
}

1;
