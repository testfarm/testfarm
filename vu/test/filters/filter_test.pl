#!/usr/bin/perl -w

use IO::Handle;

# Disable output buffering
STDOUT->autoflush();

%CLASSES = (
  'class1' => [],
  'class2' => [],
);

@FILTERS = ();

foreach ( keys %CLASSES ) { print "*$_\n" };

while ( <STDIN> ) {
  s/^\s+//;
  s/\s+$//;
  next if /^$/;

  if ( s/^\+// ) {
    my ($num, $class_id, @options) = split /\s+/;

    if ( exists $CLASSES{$class_id} ) {
      my $class = $CLASSES{$class_id};
      push @$class, $num;

      $FILTERS[$num] = \@options;

      print STDERR "Filter $num added: class='$class_id', options=(@options)\n";
      print "+$num\n";
    }
    else {
      print STDERR "Filter add $num rejected: unknown class '$class_id'\n";
    }
  }
  elsif ( s/^-// ) {
    my ($num) = split /\s+/;

    if ( defined $FILTERS[$num] ) {
      $FILTERS[$num] = undef;
      print STDERR "Filter $num removed\n";
      print "-$num\n";
    }
    else {
      print STDERR "Filter remove $num rejected: unknown filter\n";
    }
  }
  elsif ( s/^\?// ) {
    my ($num) = split /\s+/;

    my $msg;

    if ( defined $FILTERS[$num] ) {
      $msg = "@{$FILTERS[$num]}";
      print STDERR "Filter $num information: '$msg'\n";
    }
    else {
      $msg = '*UNKNOWN FILTER*';
      print STDERR "Filter info $num rejected: unknown filter\n";
    }

    print "?$num $msg\n";
  }
  elsif ( s/^!// ) {
    my ($num, $shmid) = split /\s+/;

    if ( defined $FILTERS[$num] ) {
      print STDERR "Filter $num applied: shmid=$shmid\n";
    }
    else {
      print STDERR "Filter apply $num rejected: unknown filter\n";
    }

    print "!$num\n";
  }
  else {
    print STDERR "Unknown command '$_'\n";
  }
}
