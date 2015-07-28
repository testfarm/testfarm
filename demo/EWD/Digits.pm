package Digits;

use TestFarm::Trig;
use EWDsystem;

%PATTERNS = ();
%LINKS = ();


sub Setup {
  my $name = shift;

  Destroy($name);

  my @ids = ();
  foreach my $digit ( @_ ) {
    push @ids, $$digit{ID};
  }

  # Launch digit value post-processing link
  $VISU->link($name, "@ids");

  my $ret = $VISU->connect($name, "./Digits.pl @ids");
  if ( $ret == 0 ) {
    $LINKS{$name} = 1;
  }

  # Start OCR recognition
  foreach my $digit ( @_ ) {
    my $id = $$digit{ID};
    $PATTERNS{$id} = $name;
    $VISU->match_add($id, $$digit{SOURCE}.' dump');
  }
  $VISU->sync();

  return $ret;
}


sub Destroy($) {
  my $name = shift;

  if ( exists $LINKS{$name} ) {
    $VISU->kill($name);
    $VISU->unlink($name);
    delete $LINKS{$name};
  }

  foreach my $id ( keys %PATTERNS ) {
    if ( $PATTERNS{$id} eq $name ) {
      $VISU->match_remove($id);
      delete $PATTERNS{$id};
    }
  }

  $VISU->sync();
}


sub DestroyAll() {
  foreach my $name ( keys %LINKS ) {
    $VISU->kill($name);
    $VISU->unlink($name);
  }

  foreach my $id ( keys %PATTERNS ) {
    $VISU->match_remove($id);
  }

  %LINKS = ();
  %PATTERNS = ();

  $VISU->sync();
}


sub SendCommand($$) {
  my $name = shift;
  my $msg = shift;

  $VISU->send($name, '++ '.$msg);
}

sub Clear($) {
  my $name = shift;
  SendCommand($name, 'clear');
}

sub Show($) {
  my $name = shift;

  my $regex = '^\d+\s+'.$name.'\s+VALUE=';
  my $trig = TrigDef($VISU, $name, $regex);
  SendCommand($name, 'show');

  my $ret = TrigWaitInfo($name, '500ms');
  if ( $ret ) {
    $ret =~ s/^\d+\s+\S+\s+//;
  }

  TrigUndef($name);

  return $ret;
}

1;
