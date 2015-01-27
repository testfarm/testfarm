##
## TestFarm
## System Environment
##
## $Revision: 259 $
## $Date: 2006-10-04 15:56:35 +0200 (mer., 04 oct. 2006) $
##

package TestFarm::Env;

require Exporter;
@ISA = qw(Exporter);

@EXPORT = qw(
  $PWD
  $home_dir
  $conf_dir
  @libs
);

use Cwd;

BEGIN {
  $PWD = getcwd();

  $home_dir = $ENV{TESTFARM_HOME} || "/opt/testfarm";
  $home_dir =~ s{/+$}{};
  push @libs, "$home_dir/lib";

  $conf_dir = $ENV{TESTFARM_CONFIG} || "/var/testfarm";
  $conf_dir =~ s{/+$}{};
  push @libs, "$conf_dir/lib";

  if ( defined $ENV{TESTFARM_LIB} ) {
    foreach ( split ':', $ENV{TESTFARM_LIB} ) {
      s{/+$}{};
      push @libs, $_;
    }
  }

  foreach ( @libs ) { eval("use lib '$_'") }
}

1;
