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
