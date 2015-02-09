#!/usr/bin/perl -w

use GD;

# Create a new image
$im = new GD::Image(800,600);

dump_rectangle('RED',      0, 1,0,0);
dump_rectangle('YELLOW', 100, 1,1,0);
dump_rectangle('GREEN',  200, 0,1,0);
dump_rectangle('CYAN',   300, 0,1,1);
dump_rectangle('BLUE',   400, 0,0,1);
dump_rectangle('WHITE',  500, 1,1,1);


# Make sure we are writing to a binary stream
binmode STDOUT;

# Convert the image to PNG and print it on standard output
print $im->png;


sub dump_rectangle {
  my $id = shift;
  my $y = shift;
  my ($R,$G,$B) = @_;

  for (my $i = 0; $i < 8; $i++) {
    my $level = 255 - (32*$i);
    my $R = $_[0] * $level;
    my $G = $_[1] * $level;
    my $B = $_[2] * $level;

    my $color = $im->colorAllocate($R, $G, $B);

    my $x = $i * 100;
    $im->filledRectangle($x, $y, $x+99, $y+99, $color);

    local *OBJECT;
    if ( open(OBJECT, ">$id".$i."-options") ) {
      printf OBJECT "image=#%02X%02X%02X:100x100\n", $R, $G, $B;
      print OBJECT 'window=100x100+'.$x.'+'.$y."\n";
      close(OBJECT);
    }
  }
}
