#!/usr/bin/perl -w

##
## TestFarm
## Test Suite Development Wizards
## System Configuration to Test Feature Library converter
##
## (C) Basil Dev 2006
##
## $Revision: 1129 $
## $Date: 2010-03-31 10:42:33 +0200 (mer., 31 mars 2010) $
##

use File::Basename;
use XML::LibXML;

use TestFarm::Env;
use TestFarm::Config;


###########################################################
# Get & Check arguments
###########################################################

my $filebase = undef;
my $fileout = undef;
my $verbose = 0;
my $checkonly = 0;

sub usage {
  my $rev = '$Revision: 1129 $ ';
  $rev =~ /^\$Revision: (\S+)/;
  print STDERR "TestFarm Configuration Wizard - version $1\n" if defined $1;
  print STDERR "Usage: twiz-conf [-v] [-c] [-o <output-file>] <config-file>\n";
  exit(1);
}

for (my $i = 0; $i <= $#ARGV; $i++) {
  my $arg = $ARGV[$i];

  if ( $arg =~ /^-/ ) {
    if ( $arg eq "-v" ) {
      $verbose++;
    }
    elsif ( $arg eq "-c" ) {
      $checkonly = 1;
    }
    elsif ( $arg eq "-o" ) {
      usage() if ( $i >= $#ARGV );
      $fileout = $ARGV[++$i];
      usage() if ( ($fileout ne "-") && ($fileout =~ /^-/) );
    }
    else {
      usage();
    }
  }
  else {
    usage() if ( defined $filebase );
    $filebase = $arg;
  }
}

usage() unless ( defined $filebase );

my $filedir = $PWD;
$filebase .= ".xml" unless ( $filebase =~ /.xml$/ );
unless ( -f "$filedir/$filebase" ) {
  $filedir = $conf_dir.'/lib';
  unless ( -f $filedir.'/'.$filebase ) {
    print STDERR "[ERROR] System Definition File '$filebase' not found in directories $PWD, $filedir\n";
    exit(1);
  }
}
$filein = "$filedir/$filebase";

my ($conf_name) = fileparse($filein, (".xml"));
$fileout = $conf_name.".pm" unless ( defined $fileout );
my ($packname) = fileparse($fileout, (".pm"));


###########################################################
# Fetch & Parse System Definition File
###########################################################

my $parser = XML::LibXML->new();
my $doc = undef;

my $search_path = $ENV{'SGML_SEARCH_PATH'};
foreach ( @TestFarm::Env::libs ) {
  $search_path .= ":" if $search_path;
  $search_path .= $_;
}
$ENV{'SGML_SEARCH_PATH'} = $search_path;

$parser->validation(1);

eval {
  $doc = $parser->parse_file($filein);
};
if ($@) {
  foreach ( split '\n', $@ ) {
    next if /twiz-conf/;
    print STDERR "[ERROR] $_\n";
  }
  exit(1);
}


###########################################################
# Check root element is present
###########################################################

my @conf_nodes = $doc->getElementsByTagName("CONFIG");
if ( $#conf_nodes < 0 ) {
  print STDERR "[ERROR] $filein: CONFIG element not found\n";
  exit(2);
}

# Exit with success now if checkonly option (-c) is used
exit(0) if $checkonly;

if ( $#conf_nodes > 0 ) {
  print STDERR "[ERROR] $filein: More than one CONFIG element found\n";
  exit(2);
}

my $conf = $conf_nodes[0];


###########################################################
# Check non-redundency of identifiers
###########################################################

my @id_list = ();
my $id_faults = 0;

sub id_add {
  my $id = shift;
  my $usage = shift;

  foreach ( @id_list ) {
    if ( $$_[0] eq $id ) {
      print STDERR "[ERROR] $filein: Identifier '$id' already used";
      print STDERR " in $$_[1] declaration" if ( $#$_ > 0 );
      print STDERR "\n";
      return undef;
    }
  }
  push @id_list, [$id, $usage];
  return $id;
}

foreach my $id ( @VerdictId ) {
  id_add($id, "Main System");
}

for (my $i = 1; $i <= $#CriticityId; $i++ ) {
  my $id = $CriticityId[$i];
  if ( defined $id ) {
    id_add($id, "Main System");
    id_add(uc($id), "Main System");
  }
}

id_add("START", "Main System");
id_add("STOP", "Main System");
id_add("INFO", "Main System");
id_add("%INFO", "Main System");
id_add("%FEATURE", "Main System");
id_add("%ACTION", "Main System");

my $service = $conf->getElementsByTagName("SERVICE");
if ( $verbose && ($#$service >= 0) ) {
  print STDERR "[INFO] $filein: Services:";
  foreach ( @$service ) {
    my $id = $_->getAttribute("id");
    print STDERR " $id";
  }
  print STDERR "\n";
}

my $interface = $conf->getElementsByTagName("INTERFACE");
if ( $#$interface >= 0 ) {
  print STDERR "[INFO] $filein: Interfaces:" if $verbose;
  foreach ( @$interface ) {
    my $id = $_->getAttribute("id");
    print STDERR " $id" if $verbose;
    $id_faults++ unless defined id_add("\$$id", "Interface");
    $id_faults++ unless defined id_add("START_$id", "Interface");
    $id_faults++ unless defined id_add("STOP_$id", "Interface");
  }
  print STDERR "\n" if $verbose;
}
else {
  print STDERR "[ERROR] $filein: No interfaces defined\n";
  $id_faults++;
}

my $feature = $conf->getElementsByTagName("FEATURE");
if ( $#$feature >= 0 ) {
  print STDERR "[INFO] $filein: Features:" if $verbose;
  foreach ( @$feature ) {
    my $id = $_->getAttribute("id");
    print STDERR " $id" if $verbose;
    $id_faults++ unless defined id_add("START_$id", "Action");
    $id_faults++ unless defined id_add("STOP_$id", "Action");
  }
  print STDERR "\n" if $verbose;
}
else {
  print STDERR "[ERROR] $filein: No features defined\n";
  $id_faults++;
}

my $action = $conf->getElementsByTagName("ACTION");
if ( $#$action >= 0 ) {
  print STDERR "[INFO] $filein: Actions:" if $verbose;
  foreach ( @$action ) {
    my $id = $_->getAttribute("id");
    print STDERR " $id" if $verbose;
    $id_faults++ unless defined id_add("$id", "Action");
  }
  print STDERR "\n" if $verbose;
}

exit(3) if ( $id_faults > 0 );


###########################################################
# Open output file
###########################################################

unless ( open(FO, ">$fileout") ) {
  print STDERR "[ERROR] Cannot open output file '$fileout': $!\n";
  exit(2);
}

print STDERR "[INFO] $filein: Writing Test Feature Library '$fileout'\n" if $verbose;

print FO "#\n";
print FO "# This file is generated automatically by the TestFarm Configuration Wizard\n";
print FO "# from the System Definition File $filein,\n";
print FO "#\n";
print FO "# DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT\n";
print FO "#\n\n";
print FO "package $packname;\n\n";
print FO "use TestFarm::Service;\n";
print FO "use TestFarm::Engine;\n";
print FO "use TestFarm::Trig;\n";


###########################################################
# Declared interface types that are used
###########################################################

my $err = 0;
my @uses = ();

foreach ( @$interface ) {
  # Get interface type;
  my $type = $_->getAttribute("type");
  next if $type =~ /^\s*$/;

  # Check for non-redundant declaration
  foreach ( @uses ) {
    if ( $type eq $_ ) {
      $type = undef;
      last;
    }
  }
  next unless defined $type;

  # The module file
  my $file = $type;
  $file =~ s/::/\//g;

  # Check that the interface lib actually exists.
  my $pm = undef;
  foreach ( @INC ) {
    $pm = "$_/$file.pm";
    last if ( -f $pm );
  }

  unless ( -f $pm ) {
    print STDERR "[ERROR] No package found for interface '$type'\n";
    $err++;
  }

  # Dump use declaration
  push @uses, $type;
  print FO "use $type;\n";
}

@uses = undef;
print FO "\n";

if ( $err > 0 ) {
  close(FO);
  unlink($fileout);
  exit(3);
}


###########################################################
# Dump package exports
###########################################################

print FO "require Exporter;\n";
print FO "\@ISA = qw (Exporter);\n";
print FO "\@EXPORT = qw (\n";
foreach my $id ( @VerdictId ) {
  print FO "  $id\n";
}
for (my $i = 1; $i <= $#CriticityId; $i++ ) {
  my $id = $CriticityId[$i];
  if ( defined $id ) {
    print FO '  '.$id."\n";
    print FO '  '.uc($id)."\n";
  }
}
print FO "  START\n";
print FO "  STOP\n";
print FO "  INFO\n";
print FO "  %INFO\n";
print FO "  %FEATURE\n";
print FO "  %ACTION\n";
foreach ( @$interface ) {
  my $id = $_->getAttribute("id");
  my $var = "\$$id";
  print FO "  $var\n";
}
foreach ( @$action ) {
  my $id = $_->getAttribute("id");
  print FO "  $id\n";
}
print FO ");\n\n";

for (my $i = 0; $i <= $#VerdictId; $i++ ) {
  my $id = $VerdictId[$i];
  print FO "use constant $id => $i;\n";
}

for (my $i = 1; $i <= $#CriticityId; $i++ ) {
  my $id = $CriticityId[$i];
  if ( defined $id ) {
    print FO 'use constant '.$id." => $i;\n";
    print FO 'use constant '.uc($id)." => $i;\n";
  }
}


###########################################################
# Dump package BEGIN function
###########################################################

print FO "\n#\n";
print FO "# System information\n";
print FO "#\n\n";

print FO "BEGIN {\n";

print FO "  %INFO = (\n";
print FO "    'System Configuration' => '$conf_name',\n";

my $desc = get_text_by_tag($conf, "DESCRIPTION");
print FO "    'System Description' => '$desc',\n";

foreach ( $conf->getChildrenByTagName("INFO") ) {
  my $id = $_->getAttribute("id");
  my $txt = get_text_line($_);
  $txt .= " " if ( $txt =~ /\$$/ );  # To avoid unwanted PERL interpolation
  print FO "    '$id' => '$txt',\n" if ( $txt ne "" );
}
print FO "  );\n";

print FO "  %FEATURE = (\n";
foreach ( @$feature ) {
  my $id = $_->getAttribute("id");
  my $desc = get_text_by_tag($_, "DESCRIPTION");
  print FO "    '$id' => '$desc',\n";
}
print FO "  );\n";

print FO "  %ACTION = (\n";
foreach ( @$action ) {
  my $id = $_->getAttribute("id");
  my $parent = $_->getParentNode();
  my $parent_id = $parent->getAttribute("id");
  print FO "    '$id' => '$parent_id',\n";
}
print FO "  );\n";

dump_text(\*FO, $conf, "START");

print FO "}\n\n";

print FO "sub INFO {\n";
print FO "  foreach ( keys %INFO ) { print \"IN_HEADER \$_: \", \$INFO{\$_}, \"\\n\" unless /^Interface / }\n";
print FO "  print \"IN_HEADER\\n\";\n";
print FO "  foreach ( sort keys %INFO ) { print \"IN_HEADER \$_: \", \$INFO{\$_}, \"\\n\" if /^Interface / }\n";
print FO "}\n\n";


###########################################################
# Dump main start/stop functions
###########################################################

print FO "\n#\n";
print FO "# System start/stop functions\n";
print FO "#\n\n";

print FO "sub __START_FAILED {\n";
print FO "  print STDERR \"*** START FAILED: \@_\\n\";\n";
print FO "  return 1;\n";
print FO "}\n\n";

print FO "sub START {\n";
print FO "  my \$mode = shift || 'AUTO';\n";
print FO "  TestFarm::Engine::echo('SYSTEM_START: $conf_name');\n";
print FO "  print \"Start System $conf_name in \$mode mode:\\n\";\n";
print FO "  print \"  Features: \@{[keys %FEATURE]}\\n\";\n";
print FO "  print \"  Actions: \@{[keys %ACTION]}\\n\";\n";
dump_text(\*FO, $conf, "START");

foreach ( @$interface ) {
  my $id = $_->getAttribute("id");
  my $mode = $_->getAttribute("mode") || "ANY";
  my $indent = 0;

  next if ( $mode eq 'DISABLE' );

  if ( $mode ne "ANY" ) {
    print FO "  if ( \$mode eq '", $mode, "' ) {\n";
    $indent = 1;
  }

  print FO "  " if $indent;
  print FO "  START_$id() && return __START_FAILED('Interface $id');\n";

  foreach ( $_->getChildrenByTagName("FEATURE") ) {
    my $id = $_->getAttribute("id");
    my $mode = $_->getAttribute("mode") || "ANY";

    next if ( $mode eq 'DISABLE' );

    if ( $mode ne "ANY" ) {
      print FO "  " if $indent;
      print FO "  if ( \$mode eq '", $mode, "' ) {\n  ";
    }

    print FO "  " if $indent;
    print FO "  START_$id() && return __START_FAILED('Feature $id');\n";

    if ( $mode ne "ANY" ) {
      print FO "  " if $indent;
      print FO "  }\n";
    }
  }

  if ( $mode ne "ANY" ) {
    print FO "  }\n";
  }
}

print FO "  return 0;\n";
print FO "}\n\n";

print FO "sub STOP {\n";
foreach ( @$interface ) {
  my $id = $_->getAttribute("id");

  print FO "  if ( defined \$$id ) {\n";

  my $feature = $_->getChildrenByTagName("FEATURE");
  for (my $i = $#$feature; $i >= 0; $i--) {
    my $id = $$feature[$i]->getAttribute("id");
    print FO "    STOP_$id();\n";
  }

  print FO "    STOP_$id();\n";
  print FO "  }\n";
}
dump_text(\*FO, $conf, "STOP");
print FO "}\n\n";


###########################################################
# Dump Interface start/stop functions
###########################################################

print FO "\n#\n";
print FO "# Interface start/stop functions\n";
print FO "#\n\n";

foreach ( @$interface ) {
  my $id = $_->getAttribute("id");
  my $type = $_->getAttribute("type");
  my $addr = $_->getAttribute("addr") || "";
  my $var = "\$$id";

  print FO "sub START_$id {\n";
  print FO "  TestFarm::Engine::echo('INTERFACE_START: $id');\n";
  print FO "  print \"Start Interface $id\\n\";\n";
  print FO "  my \$I = $var = $type->new('$id', \"$addr\");\n";

  # Dump interface info
  foreach ( $_->getChildrenByTagName("INFO") ) {
    my $iid = $_->getAttribute("id");
    my $key = "Interface $id $iid";
    my $txt = get_text_line($_);
    $txt .= " " if ( $txt =~ /\$$/ );  # To avoid unwanted PERL interpolation
    if ( $txt ne "" ) {
      print FO "  \$INFO{'", $key, "'} = '", $txt, "';\n";
    }
    else {
      print FO "  my \$", $iid, " = eval('\$I->", $iid, "()');\n";
      print FO "  \$INFO{'", $key, "'} = \$", $iid, " if defined \$", $iid, ";\n";
    }
  }

  dump_text(\*FO, $_, "START") ||
    print FO "  return 0;\n";

  print FO "}\n\n";

  print FO "sub STOP_$id {\n";
  print FO "  my \$I = $var;\n";
  dump_text(\*FO, $_, "STOP");
  print FO "  \$I = $var = undef;\n";
  print FO "}\n\n";
}


###########################################################
# Dump Features start/stop functions
###########################################################

print FO "\n#\n";
print FO "# Feature start/stop functions\n";
print FO "#\n\n";

foreach ( $conf->getElementsByTagName("FEATURE") ) {
  my $id = $_->getAttribute("id");
  my $parent = $_->getParentNode();
  my $parent_id = $parent->getAttribute("id");

  print FO "sub START_$id {\n";
  print FO "  my \$I = \$$parent_id;\n";
  dump_text(\*FO, $_, "START") ||
    print FO "  return 0;\n";
  print FO "}\n\n";

  print FO "sub STOP_$id {\n";
  print FO "  my \$I = \$$parent_id;\n";
  dump_text(\*FO, $_, "STOP");
  print FO "}\n\n";
}


###########################################################
# Dump action functions
###########################################################

my $titled = 0;

foreach ( @$interface ) {
  my $parent_id = $_->getAttribute("id");

  foreach ( $_->getElementsByTagName("ACTION") ) {
    my $id = $_->getAttribute("id");
    my $proto = $_->getAttribute("proto");

    unless ( $titled ) {
      print FO "\n#\n";
      print FO "# Local Interface Actions\n";
      print FO "#\n\n";
      $titled = 1;
    }

    print FO "sub $id";
    print FO "($proto)" if ( (defined $proto) && ($proto !~ /^\s*$/) );
    print FO " {\n";
    print FO "  my \$I = \$$parent_id;\n";
    dump_text_content(\*FO, $_);
    print FO "}\n\n";
  }
}


###########################################################
# Dump Service start/stop functions
###########################################################

if ( @$service ) {
  print FO "\n#\n";
  print FO "# Services start/stop functions\n";
  print FO "#\n\n";

  print FO "sub START_SERVICE {\n";
  print FO "  my \$mode = shift || 'AUTO';\n";
  foreach ( @$service ) {
    my $id = $_->getAttribute("id");
    my $mode = $_->getAttribute("mode") || "ANY";

    next if ( $mode eq 'DISABLE' );

    print FO "  START_SERVICE_$id()";
    if ( $mode ne "ANY" ) {
      print FO " if ( \$mode eq '", $mode, "' )";
    }
    print FO ";\n";
  }
  print FO "}\n\n";

  print FO "sub STOP_SERVICE {\n";
  foreach ( @$service ) {
    my $id = $_->getAttribute("id");
    print FO "  STOP_SERVICE_$id();\n";
  }
  print FO "}\n\n";

  foreach ( @$service ) {
    my $id = $_->getAttribute("id");
    my $cmd = $_->getAttribute("cmd") || "";

    $cmd =~ s/\"/\\\"/g;

    print FO "sub START_SERVICE_$id {\n";
    print FO "  my \$CMD = \"$cmd\";\n";
    dump_text(\*FO, $_, "START") ||
      print FO "  return TestFarm::Service::start(\$CMD);\n";
    print FO "}\n\n";

    print FO "sub STOP_SERVICE_$id {\n";
    print FO "  my \$CMD = \"$cmd\";\n";
    dump_text(\*FO, $_, "STOP") ||
      print FO "  return TestFarm::Service::stop(\$CMD);\n";
    print FO "}\n\n";

    print FO "sub STATUS_SERVICE_$id {\n";
    print FO "  my \$CMD = \"$cmd\";\n";
    dump_text(\*FO, $_, "STATUS") ||
      print FO "  return TestFarm::Service::pidof(\$CMD);\n";
    print FO "}\n\n";
  }
}


###########################################################
# Dump End of Package
###########################################################

print FO "\n#\n";
print FO "# End of Package\n";
print FO "#\n\n";

print FO "END {\n";
print FO "  STOP();\n";
dump_text(\*FO, $conf, "STOP");
print FO "}\n\n";

print FO "1;\n";
print FO "__END__\n";

close(FO);
exit(0);


###########################################################
# Node text content
###########################################################

sub get_text_line {
  my $node = shift;
  my $txt = "";

  foreach ( $node->getChildNodes() ) {
    next unless ( $_->nodeType() == XML_TEXT_NODE );
    my $str = $_->getData();
    $str =~ s/^\s+//;
    $str =~ s/\s+$//;
    $str =~ s/\s/ /g;
    $txt .= $str;
  }

  return $txt;
}


sub get_text_by_tag {
  my $node = shift;
  my $tag = shift;

  my $txt = "";
  foreach ( $node->getChildrenByTagName($tag) ) {
    $txt .= get_text_line($_);
  }

  return $txt;
}


sub dump_text_content {
  my $fo = shift;
  my $node = shift;
  my $count = 0;

  foreach ( $node->getChildNodes() ) {
    next unless ( $_->nodeType() == XML_TEXT_NODE );

    foreach ( split '\n', $_->getData() ) {
      s/^\s+//;
      s/\s+$//;
      next if /^$/;
      print $fo "  $_\n";
      $count++;
    }
  }

  return $count;
}


sub dump_text {
  my $fo = shift;
  my $node = shift;
  my $tag = shift;
  my $count = 0;

  foreach ( $node->getChildrenByTagName($tag) ) {
    $count += dump_text_content($fo, $_);
  }

  return $count;
}
