#!/usr/bin/perl -w

if ( ($#ARGV < 0) || ($#ARGV > 2) ) {
  print STDERR "Usage: mkgif.pl <name> [<t1> [<t2>]]\n";
  exit(1);
}

$FILES = $ARGV[0];
$T1 = $ARGV[1];
$T2 = $ARGV[2];

my @list = glob($FILES);
unless ( @list ) {
  printf STDERR "No files $FILES found\n";
  exit(1);
}

$NAME = $FILES;
unless ( $NAME =~ s/(\..+)*$//g ) {
  $NAME = 'image';
}

my $t0 = 0;
my $tmin = undef;
my $tmax = undef;

my @args = ();
my $count = 0;

foreach ( sort @list ) {
  /\.(\d+)\./;
  my $t = $1;

  next if ((defined $T1) and ($t < $T1));
  next if ((defined $T2) and ($t > $T2));
  $count++;

  $tmin = $t unless defined $tmin;
  $tmax = $t;

  my $dt = sprintf("%.0f", ($t - $t0) / 10);

  if ( $dt > 0 ) {
    push @args, "-delay $dt";
  }

  push @args, $_;

  $t0 = $t;
}

if ( $count > 0 ) {
  my $gif = $NAME.'.gif';
  printf STDERR "$gif: $count frames, %.3f seconds\n", ($tmax-$tmin) / 1000;
  system("convert @args $gif");
  print "$gif\n";
}

exit(0);
