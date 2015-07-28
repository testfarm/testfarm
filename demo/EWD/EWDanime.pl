#!/usr/bin/perl -w

use EWDdisplay;

EWDdisplay::init();
EWDdisplay::send("PWR1=0");
EWDdisplay::send("PWR2=0");

my $v1 = 0;
my $inc1 = 1;

while (1) {
  EWDdisplay::send("PWR1=$v1");

  $v1 += $inc1 * 0.1;

  if ($v1 > 105) {$inc1 = -1};
  if ($v1 < -5) {$inc1 = +1};

  select undef, undef, undef, 0.1;
}
