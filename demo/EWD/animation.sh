#!/bin/sh -f
perl -e 'print "V1=0\nV2=0\n"; $| = 1; $v=0; $inc = 1; while (1) { print "V1=$v\n"; $v += $inc; if ($v > 105) {$inc = -1}; if ($v < -5) {$inc = +1}; select undef, undef, undef, 0.2}' | perl EWDdisplay.pl >/dev/null
