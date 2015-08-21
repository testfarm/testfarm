##
## TestFarm
## User Configuration Files management
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

package TestFarm::Config;

use XML::DOM;

require Exporter;
@ISA = qw(Exporter);

@EXPORT = qw(
  @VerdictId
  @CriticityId
  @CriticityColor
);


#
# User config files management
#

$user_conf_home = $ENV{HOME}.'/.testfarm';

my $user_conf_tag = 'defaults';
my $user_conf_dir = $user_conf_home;
my $user_conf_file = $user_conf_dir.'/defaults';


my %user_conf_files = (
  'defaults' => [ 'defaults', 'TESTFARM_DEFAULTS' ],
  'report'   => [ 'report/',  'REPORT_CONFIG' ],
  'batch'    => [ 'batch/',   'BATCH' ],
);


sub set_conf($;$) {
  my $conf_id = shift || return undef;
  my $file = shift || 'Default';

  my $conf = $user_conf_files{$conf_id};
  unless ( defined $conf ) {
    print STDERR "*WARNING* Unknown TestFarm Config id '$conf_id'\n";
    return undef;
  }

  $user_conf_dir = $user_conf_home;

  my $conf_file = $$conf[0];
  if ( $conf_file =~ s/\/$// ) {
    $user_conf_dir .= '/'.$conf_file;
    $conf_file = $file;
  }

  $user_conf_tag = $$conf[1];
  $user_conf_file = $user_conf_dir.'/'.$conf_file;

  # Create storage directory
  unless ( -d $user_conf_home ) {
    mkdir($user_conf_home, 0755);
  }
  unless ( -d $user_conf_dir ) {
    mkdir($user_conf_dir, 0755);
  }

  return $user_conf_file;
}


sub get_string($) {
  my $name = shift;

  return undef unless ( -f $user_conf_file );
  my $parser = new XML::DOM::Parser();
  my $doc = $parser->parsefile($user_conf_file);

  my $root = $doc->getDocumentElement();
  return undef unless defined $root;

  my $nodes = $root->getElementsByTagName($name, 0);
  return undef if ( $#$nodes < 0 );

  my $content = '';

  foreach ( $$nodes[0]->getChildNodes() ) {
    next unless ( $_->getNodeType() == TEXT_NODE );
    my $str = $_->getData();
    $str =~ s/^\s+//;
    $str =~ s/\s+$//;
    $content .= $str;
  }

  $doc->dispose();
  $parser = undef;

  return $content;
}


sub set_string($$) {
  my $name = shift;
  my $content = shift;

  unless ( -f $user_conf_file ) {
    local *FOUT;
    if ( open(FOUT, ">$user_conf_file") ) {
      print FOUT "<?xml version=\"1.0\"?>\n";
      print FOUT "<$user_conf_tag/>\n";
      close(FOUT);
    }
    else {
      print STDERR "Cannot create file $user_conf_file: $!\n";
      return undef;
    }
  }

  my $parser = new XML::DOM::Parser();
  my $doc = $parser->parsefile($user_conf_file);

  # Get document data
  my $root = $doc->getDocumentElement()
    or $doc->createElement($user_conf_tag);

  # Prepare for update
  my $nodes = $root->getElementsByTagName($name, 0);
  my $elt = $$nodes[0];
  if ( defined $elt ) {
    foreach ( $elt->getChildNodes() ) {
      $elt->removeChild($_);
    }
  }
  else {
    $elt = $doc->createElement($name);
    $root->appendChild($elt);
  }

  # Insert updated data
  my $elt_data = $doc->createTextNode($content);
  $elt->appendChild($elt_data);

  # Dump XML file
  #$doc->printToFileHandle(\*STDOUT);
  $doc->printToFile($user_conf_file);
}


#
# Library config files management
#

sub get_lib($) {
  my $fname = shift;

  foreach my $dirname ( '/opt/testfarm', '/var/testfarm' ) {
    my $fpath = "$dirname/lib/$fname";
    return $fpath if ( -f $fpath );
  }

  return undef;
}


#
# Verdict and Criticity definitions
#

@VerdictId = ('PASSED', 'FAILED', 'INCONCLUSIVE', 'SKIP');

@CriticityId = ('-');
@CriticityColor = ('black');


sub init_criticity {
  my $fpath = get_lib('criticity.xml') || return 0;

  my $parser = new XML::DOM::Parser();
  my $doc = $parser->parsefile($fpath);

  my $root = $doc->getDocumentElement();
  return undef unless defined $root;

  my @nodes = $root->getElementsByTagName('CRITICITY', 0);
  return undef if ( $#nodes < 0 );

  foreach ( @nodes ) {
    my $level = $_->getAttribute('level');
    if ( $level > 0 ) {
      $CriticityId[$level] = $_->getAttribute('id');
      $CriticityColor[$level] = $_->getAttribute('color');
    }
  }

  $doc->dispose();
  $parser = undef;
}

init_criticity();

1;
