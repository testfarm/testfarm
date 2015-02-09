package TestFarm::Version;

require Exporter;
@ISA = qw(Exporter);

@EXPORT = qw(
  $VERSION
);

BEGIN {
    $VERSION = "@VERSION@";
}

1;
