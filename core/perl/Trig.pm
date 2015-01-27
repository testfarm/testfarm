##
## TestFarm
## Test Suite Execution Library
## Triggers Management
##
## (C) Basil Dev 2006
##
## $Revision: 191 $
## $Date: 2006-07-29 16:07:55 +0200 (sam., 29 juil. 2006) $
##

package TestFarm::Trig;

use TestFarm::Engine;

require Exporter;
@ISA = qw(Exporter);

@EXPORT = qw(
  TrigDef
  TrigUndef
  TrigClear
  TrigTimeout
  TrigWait
  TrigInfo
  TrigCount
  TrigWaitInfo
  TrigVarDef
  TrigVarUndef
  TrigVarId
  TrigVarPrefix
);


my $TrigAttr_count = 0;

sub TrigAttr(\$;$) {
  my $obj = shift;
  my $id = shift;
  my $ref = ref $obj;

  #print STDERR '-- [caller='.uc(caller)."] TrigAttr($obj, $id) ref=$ref\n";
  unless ( defined $id ) {
    if ( $ref ne 'SCALAR' ) {
      $id = eval($$obj->trig_id());
    }

    if ( (! defined $id) || ($id =~ /^\s*$/) ) {
      $TrigAttr_count ||= 0;
      $id = "TRIG".$TrigAttr_count++;
    }
  }

  my $periph = ($ref eq 'SCALAR') ? $$obj : $$obj->{ID};

  if ( (! defined $periph) || ($periph =~ /^\s*$/) ) {
    print STDERR "Illegal Test Interface name when declaring trigger '$id'\n";
    return undef;
  }

  return ($id, $periph);
}


#
# Procedural Trigger Management
#

sub TrigDef($$$) {       # Arguments: <interface> <id> <regex>
  my $periph = shift;
  my $id = shift;
  my $regex = shift;

  ($id, $periph) = TrigAttr($periph, $id);
  TestFarm::Engine::trig_def($id, $periph, $regex);

  return $id;
}

sub TrigUndef {       # Arguments: <id> ...
  if ( TestFarm::Engine::started ) {
    TestFarm::Engine::trig_undef(@_);
  }
}

sub TrigClear {       # Arguments: <id> ...
  if ( TestFarm::Engine::started ) {
    TestFarm::Engine::trig_clear(@_);
  }
}

sub TrigTimeout($) {   # Arguments: <time>
  my $value = shift;
  return TestFarm::Engine::timeout($value);
}

sub TrigWait($;$) {
  my $expr = shift;
  my $timeout = shift;
  return TestFarm::Engine::wait($expr, $timeout);
}

sub TrigInfo($) {
  my $id = shift;
  return TestFarm::Engine::trig_info($id);
}

sub TrigCount($) {
  my $id = shift;
  return TestFarm::Engine::trig_count($id);
}

sub TrigWaitInfo($;$) {
  my $trig = shift;
  my $timeout = shift;
  my $str = TestFarm::Engine::wait($trig, $timeout) || return undef;
  my @trigged = split ' ', $str;
  return TestFarm::Engine::trig_info($trigged[0]);
}


#
# Magic Variable Trigger Management
#

sub TIESCALAR {
  my $class = shift;
  my $periph = shift;
  my $id = shift;
  my $regex = shift;

  $regex = undef if ((defined $regex) && ($regex =~ /^\s*$/));
  if ( defined $regex ) {
    TrigDef($periph, $id, $regex);
  }

  my $self = {};
  $self->{periph} = $periph;
  $self->{id} = $id;
  $self->{prefix} = "";
  $self->{regex} = $regex;

  return bless $self, $class;
}

sub FETCH {
  my $self = shift;
  return $self->{id};
}

sub STORE {
  my $self = shift;

  my $regex = shift;
  $regex = undef if ((defined $regex) && ($regex =~ /^\s*$/));

  my $id = $self->{id};

  if ( defined $regex ) {
    TrigDef($self->{periph}, $id, $self->{prefix}.$regex);
  }
  else {
    TrigUndef($id);
  }

  $self->{regex} = $regex;
  return $id;
}

sub DESTROY {
  my $self = shift;
  return unless $self->{regex};
  TrigUndef($self->{id});
}


sub TrigVarDef($\$;$) {   # <interface> <$var> [<regex>]
  my $periph = shift;
  my $var = shift;

  my $id;
  ($id, $periph) = TrigAttr($periph);

  my $regex = shift;

  TrigVarUndef($var);
  tie $$var, TestFarm::Trig, $periph, $id, $regex;

  return $$var;
}

sub TrigVarUndef(\$) {   # Arguments: <$var>
  my $var = shift;
  return if ref tied $$var eq '';
  untie $$var;
  $$var = undef;
}

sub TrigVarId(\$$) {   # Arguments: <$var> <id>
  my $var = shift;
  my $id = $_[0] || "";

  if ( $id ) {

    if ( defined $$var->{regex} ) {
      TrigUndef($$var->{id});
      TrigDef($$var->{periph}, $id, $$var->{prefix}.$$var->{regex});
    }
    $$var->{id} = $id;
  }

  return $$var->{id};
}

sub TrigVarPrefix(\$$) {   # Arguments: <$var> <prefix>
  my $var = shift;
  $$var->{prefix} = $_[0] || "";
  if ( defined $$var->{regex} ) {
    TrigDef($$var->{periph}, $$var->{id}, $$var->{prefix}.$$var->{regex});
  }
}

1;
