package Object;

use TestFarm::Wiz;


sub SearchFile {
  my $basename = shift;

  my $file = $dirname.'/'.$basename;
  return $file if ( -f $file );

  $file = 'objects/'.$basename;
  return $file if ( -f $file );

  return undef;
}


sub RetrieveImage {
  my $object = shift;

  my $basename = $object.'.png';
  my $file = SearchFile($basename);
  if ( defined $file ) {
    $file = 'image='.$file;
  }
  else {
    $basename = $object.'.ppm.gz';
    $file = SearchFile($basename);
    if ( defined $file ) {
      $file = 'image='.$file;
    }
    else {
      ERROR("Cannot find pattern image file \"$basename\"");
      $file = '';
    }
  }

  return $file;
}


sub RetrieveOptions {
  my $object = shift;

  my $options = '';

  my $basename = $object.'-options';
  my $file = SearchFile($basename);
  if ( defined $file ) {
    local *FI;
    if ( open(FI, $file) ) {
      while ( <FI> ) {
	s/^\s+//;
	next if /^$/;
	s/\s+$//;
	$options .= $_.' ';
      }
      close(FI);
    }
    else {
      ERROR("Cannot open pattern option file \"$basename\": $!");
    }
  }

  return $options;
}


1;
