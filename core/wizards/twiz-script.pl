#!/usr/bin/perl -w

##
## TestFarm
## Test Suite Development Wizards - PERL Script Generator
##
## Author: Sylvain Giroudon
## Creation: 21-JUN-2006
##
## This file is part of TestFarm,
## the Test Automation Tool for Embedded Software.
## Please visit http://www.testfarm.org.
##
## TestFarm is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## TestFarm is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
##

use POSIX;
use File::Basename;
use Text::ParseWords;

use TestFarm::Env;

use TestFarm::Wiz qw(
  :DEFAULT
  location_clear
  location_push
  location_pop
  location_line
  $verbose
  $colors
  $errcount
  $warncount
  @wizlib
  wizlib_init
);


############################################################
# Get options
############################################################

$quiet = 0;
$colors = isatty('STDERR');
@input_file_list = ();

my $rev = '$Revision: 1135 $ ';
if ( $rev =~ /^\$Revision: (\S+)/ ) {
  $version = $1;
}
else {
  $version = '(unknown)';
}

sub usage {
  print STDERR "TestFarm Script Wizard - version $version\n";
  print STDERR "Usage: twiz-script [-v] [-q|--quiet] [--colors] [--nocolors] <wiz-script> ...\n";
  exit(1);
}

sub parse_options {
  my $opt = shift;

  if ( $opt eq "-v" ) { $verbose++ }
  elsif ( ($opt eq "-q") || ($opt eq "--quiet") ) { $quiet = 1 }
  elsif ( $opt eq "--colors" ) { $colors = 1 }
  elsif ( $opt eq "--nocolors" ) { $colors = 0 }
  else { return 1 }

  return 0;
}

for (my $i = 0; $i <= $#ARGV; $i++) {
  my $arg = $ARGV[$i];

  if ( $arg =~ /^-/ ) {
    usage() if parse_options($arg);
  }
  elsif ( $arg =~ /^\@(.+)$/ ) {
    my $fname = $1;
    local *FI;

    if ( open(FI, "<$fname") ) { 
      while ( <FI> ) {
        chomp;
        push @input_file_list, $_;
      }
      close(FI);
    }
    else {
      print STDERR "Cannot open list file $fname\n";
    }
  }
  else {
    push @input_file_list, $arg;
  }
}

if ( open(FI, ".twiz") ) {
  while ( <FI> ) {
    chomp;
    parse_options($_);
  }
  close(FI);
}

usage() if ( $#input_file_list < 0 );

$verbose = 0 if $quiet;


############################################################
# Import WIZLIB path
############################################################

wizlib_init();
INFO2(1, "WIZ definition files path: @wizlib");

# Add WIZLIB items to @INC list
foreach my $dir ( @wizlib ) {
  eval("use lib \"$dir\"");
}


############################################################
# Process input files given as arguments
############################################################

$input_file = "";
$input_line = 0;

$give_up = "";         # GIVE_UP wiz keyword
@$give_up_args = ();   # Parameters to be given when expanding GIVE_UP wiz

@wizscript_stack = (); # Stack of included wiz-script
@wizdef_stack = ();    # Stack of included wiz-def

$wizgen_ret = "";      # Value returned by last WIZGEN function call
$exit_ret = 0;         # Value to be returned by the script wizard command


# Process all input files
foreach ( @input_file_list ) {
  # Skip options
  if ( /^-/ ) { next }

  # Get files properties
  my $filein = $_;
  ($packname, $dirname, my $suffix) = fileparse($filein, (".wiz"));
  $dirname =~ s/\/+$//;
  $filename = "$dirname/$packname.pm";

  $packname =~ s/\W/_/g;
  if ( $dirname ne '.' ) {
    $packname = $dirname.'/'.$packname;
  }
  $packname =~ s/\/+/::/g;

  # Protect package name from not beginning with a letter
  if ( $packname =~ /^\d/ ) {
    $packname = "_$packname";
  }

  # Reset error & warning counters
  $errcount = 0;
  $warncount = 0;

  # Open output file
  unless ( open(STDOUT, ">$filename") ) {
    ERROR2("Unable to open output file $filename");
    next;
  }
  INFO2(1, "$filein: Output file is \"$filename\"");
  INFO2(2, "Package name is \"$packname\"");

  # Dump some information comments
  print STDOUT "#\n";
  print STDOUT "# This file was generated automatically using the TestFarm Script Wizard\n";
  print STDOUT "#\n";
  print STDOUT "# DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT\n";
  print STDOUT "#\n\n";

  # Clear give-up flag
  $give_up = "";
  @give_up_args = ();

  # Clear location info
  location_clear();

  # Expand the PREAMBLE wiz
  @wizdef_stack = ();
  dump_WIZ(0, "PREAMBLE");

  # process wiz-script file
  @wizscript_stack = ();
  my $ret = process_input($filein);

  # Put the POSTAMBLE label;
  print STDOUT "POSTAMBLE:\n";

  # Expand the POSTAMBLE wiz
  @wizdef_stack = ();
  dump_WIZ(0, "POSTAMBLE");

  # Close output file
  close(STDOUT);

  # Report WIZ processing error */
  if ( $ret ) {
    ERROR2("Unable to open input file $filein");
    unlink $filename;
    next;
  }

  # Display warning/error count summary
  REPORT_ERR_WARN($filein);

  if ( $errcount > 0 ) {
    # If some errors were encountered, remove output file
    print STDERR "*** No output produced from $filein\n" unless $quiet;
    unlink $filename;
    $exit_ret = 1;
  }
  else {
    if ( $give_up ne "" ) {
      # If a #$GIVE_UP wiz was encountered,
      # overwrite the whole output script with this wiz
      unless ( open(STDOUT, ">$filename") ) {
        ERROR2("Unable to open output file \"$filename\"");
        next;
      }
      dump_WIZ(0, $give_up, @give_up_args);
      close(STDOUT);
    }
    print STDERR "*** Target file \"$filename\" completed\n" unless $quiet;
  }
}

exit($exit_ret);


############################################################
# Process input wiz-script
############################################################

sub process_input {
  # Get arguments
  my $filein = shift;
  local *FI;

  # Check for circular insertion
  foreach ( @wizscript_stack ) {
    if ( /$filein/ ) {
      ERROR("Circular insertion of wiz-script $filein");
      return 1;
    }
  }

  # Open input file
  unless ( open(FI, "<$filein") ) { return 1 }

  # We entered file successfuly !
  INFO(2, "Entering input file \"$filein\"");
  push @wizscript_stack, $filein;
  location_push($filein, 0);
  $input_line = 0;
  $input_file = $filein;

  # Expand the called wiz
  while( <FI> ) {
    chomp;

    $input_line++;
    location_line($input_line);

    # Remove leading & trailing blanks
    s/^\s+//;
    s/\s+$//;

    # Skip blank and comment lines
    if ( "$_" eq "" ) { next }
    if ( /^\#/ ) { next }

    # Get wiz keyword and its arguments
    my @args = quotewords("\\s+", 1, $_);
    my $keyword = shift @args;

    # Ensure quotewords operation succeeded
    if ( ! defined $keyword ) {
      ERROR("Syntax error");
      next;
    }

    if ( ($keyword eq "INSERT_FILE") || ($keyword eq "INCLUDE") ) {
      foreach ( @args ) {
        my $insertname = "$dirname/$_.wiz";

        if ( ! -e $insertname ) {
          ERROR("Couldn't find input file $insertname")
        }
        else {
          INFO(2, "INCLUDE $insertname");

          if ( process_input($insertname) ) {
            ERROR("Unable to open input file $insertname");
          }
        }
      }
    }
    else {
      # Expand wiz
      @wizdef_stack = ();
      dump_WIZ(1, $keyword, @args);
    }
  }

  # Close input file
  close(FI);
  pop @wizscript_stack;
  ($input_file, $input_line) = location_pop();

  INFO(2, "Leaving input file \"$filein\"");

  return 0;
}


############################################################
# WIZ dump
############################################################

sub dump_NEWLINE {
  print STDOUT "\n";
}

sub dump_LINELOC {
  my $filename = shift;
  my $linecount = shift;

  $filename =~ s/^\.\///;
  $filename =~ s/^$PWD\///;

  print STDOUT "#\$LINE $filename $linecount\n";
}

sub dump_WIZ {
  my $show_headfoot = shift;
  my $keyword = shift;
  my @args = @_;
  my $linecount = 0;
  my $param_count = 0;
  my @param_name;
  local *WIZ;

  # Check for circular insertion
  foreach ( @wizdef_stack ) {
    if ( /$keyword/ ) {
      # If a circular file insertion was detected, produce an error
      ERROR("Circular call of wiz \"$keyword\"");
      return 1;
    }
  }

  # Retrieve definition file for the wiz
  my $wizname = "";
  foreach ( @wizlib ) {
    $wizname = "$_/$keyword.wizdef";
    if ( -f $wizname ) { last }
  }
  if ( $wizname eq "" ) {
    ERROR("Cannot find wiz \"$keyword\". Please set environment variable TESTFARM_WIZLIB properly");
    return 1;
  }

  # Open wiz definition file
  unless ( open(WIZ, "<$wizname") ) {
    ERROR("Cannot open wiz definition file \"$wizname\"");
    return 2;
  }

  # We entered file successfuly !
  push @wizdef_stack, $wizname;
  INFO(3, "Expanding wiz \"$keyword\" from definition file \"$wizname\"");

  # Clear condition stack
  my @if_stack = ();        # Conditional input stack
  my @if_weight = ();       # Conditional nesting weight

  # Dump wiz expansion header
  print STDOUT "  #####\n";
  print STDOUT "  ##### $keyword";
  print STDOUT " @args" if ( $#args >= 0 );
  print STDOUT "\n";
  print STDOUT "  #####\n";

  if ( $show_headfoot != 0 ) {
    print STDOUT "  TestFarm::Engine::echo('-STEP BEGIN $keyword";
    print STDOUT " @args" if ( $#args >= 0 );
    print STDOUT "');\n";
  }

  dump_LINELOC($wizname, $linecount);

  while ( <WIZ> ) {
    chomp;
    $linecount++;

    # Skip blank lines
    if ( "$_" eq "" ) {
      dump_NEWLINE();
      next;
    }

    if ( /^=/ ) {
      dump_NEWLINE();

      # Skip POD sections
      INFO2(3, "$wizname:$linecount: POD section begins here");
      while ( <WIZ> ) {
        chomp;
        $linecount++;
        dump_NEWLINE();
        if ( /^=cut/ ) { last }
      }
      INFO2(3, "$wizname:$linecount: POD section ends here");
    }
    elsif ( /^\s*\#/ ) {
      s/^\s*//;

      if ( /^\#\#/ ) {
        s/^\#//;

        # Dump comment line
        print STDOUT "$_\n";
      }
      else {
        dump_NEWLINE();

        if ( /^\#\$PARAM/ ) {
          # A parameter definition...
          # Skip it if current condition if FALSE
          if ( dump_WIZ_ok(\@if_stack) ) {
            my ($trash, $name, $values, @description) = split;

            INFO2(3, "$wizname:$linecount: Parameter <$name>: $values \"@description\"");

            # Determine whether the param is optional
            my $optional = 0;
            if ( $name =~ s/^\[// ) {
              $name =~ s/\]$//;
              $optional = 1;
            }

            # Build the list of possible parameter values
            my @values_list =  split /,/, $values;

            if ( $param_count >= ($#args+1) ) {
              # If the argument is missing...
              if ( $optional ) {
                # If the argument is missing and optional, take the default value
                if ( $values_list[0] =~ /^%d/ ) {
                  $args[$param_count] = "0";
                  INFO(2, "Ommited parameter <$name> set with default value \"$args[$param_count]\"");
                }
                elsif ( ($#values_list >= 0) &&
                        ($values_list[0] ne "%s") &&
                        ($values_list[0] ne "...") &&
                        ($values_list[0] !~ /^\// ) ) {
                  $args[$param_count] = $values_list[0];
                  INFO(2, "Ommited parameter <$name> set with default value \"$args[$param_count]\"");
                }
                else {
                  ERROR2("$wizname:$linecount: No default value found for parameter <$name>");
                }
              }
              else {
                # If the argument is missing and mandatory,
                # put a dummy argument value and produce an error
                $args[$param_count] = "";
                ERROR("Wiz \"$keyword\" requires parameter <$name>");
              }
            }
            else {
              # If the argument is passed to the wiz call...
              # Check parameter against the value-list
              my $faulty = 1;

              foreach ( @values_list ) {
                if ( /^%d/ ) {
                  # An integer value
                  my $value = $args[$param_count];

                  if ( $value =~ /\d+/ ) {
                    $faulty = 0;

                    if ( s/^%d:// ) {
                      my ($min, $max) = split "-";
                      if ( ($value < $min) || ($value > $max) ) {
                        ERROR("Integer value out of range [$min..$max] for parameter <$name> of wiz \"$keyword\"");
                      }
                    }
                  }
                }
                elsif ( $_ eq "%s" ) {
                  # A string value
                  $faulty = 0;
                }
                elsif ( $_ eq "..." ) {
                  # A string value with all trailing parameters
                  my @trailing = ();
                  for (my $i = $param_count; $i <= $#args; $i++) {
                    push @trailing, $args[$i];
                  }

                  $#args -= $#trailing;
                  $args[$#args] = "@trailing";
                  INFO(2, "Collecting trailing arguments passed to wiz \"$keyword\" into parameter <$name>: \"@trailing\"");

                  $faulty = 0;
                }
                elsif ( s/^\/// ) {
                  # A regex-specified value
                  s/\/$//;
                  if ( $args[$param_count] =~ /$_/ ) {
                    $faulty = 0;
                  }
                }
                elsif ( $_ eq $args[$param_count] ) {
                  # A literal value
                  $faulty = 0;
                }
              }

              if ( $faulty ) {
                ERROR("Bad value for parameter <$name> of wiz \"$keyword\"");
              }
            }

            $param_name[$param_count] = $name;
            $param_count++;
          }
        }
        elsif ( /^\#\$WIZCALL/ ) {
          # A sub-wiz call...
          # Skip it if current condition if FALSE
          if ( dump_WIZ_ok(\@if_stack) ) {
            my ($command, $sub_keyword, @sub_args) = split;

            # Replace parameter tags with actual value
            my $count = 0;
            foreach ( @sub_args ) {
              my $token = replace_parameters($_, \@param_name, \@args);
              $sub_args[$count++] = $token;
            }

            location_push($wizname, $linecount);

            if ( defined $sub_keyword ) {
              # Expand wiz
              dump_WIZ(0, $sub_keyword, @sub_args);

              dump_LINELOC($wizname, $linecount);
            }
            else {
              ERROR("Missing wiz keyword in \"$command\" statement");
            }

            location_pop();
          }
        }
        elsif ( /^\#\$WIZGEN/ ) {
          # A wiz code generator call...
          # Skip it if current condition if FALSE
          if ( dump_WIZ_ok(\@if_stack) ) {
            my ($command, @sub_args) = split;

            # Replace parameter tags with actual value
            my $count = 0;
            foreach ( @sub_args ) {
              my $token = replace_parameters($_, \@param_name, \@args);
              $sub_args[$count++] = $token;
            }

            location_push($wizname, $linecount);

            if ( @sub_args ) {
              # Call wiz generator function
              my $modname = gen_WIZ("@sub_args");

              dump_LINELOC($wizname, $linecount);
            }
            else {
              ERROR("Missing generator function call in \"$command\" statement");
            }

            location_pop();
          }
        }
        elsif ( /^\#\$GIVE_UP/ ) {
          # A Give-Up flag...
          # Skip it if current condition if FALSE
          if ( dump_WIZ_ok(\@if_stack) ) {
            INFO(3, "GIVE_UP flag encountered, so I give up...");
            $give_up = $keyword;
            @give_up_args = @args;
          }
        }
        elsif ( /^\#\$IF/ ) {
          my ($command, @condition) = split;

          # Evaluate condition
          my $ret = dump_WIZ_cond("@condition", \@param_name, \@args);

          # Set condition flag
          push @if_stack, $ret;
          push @if_weight, 1;
        }
        elsif ( /^\#\$ELSIF/ ) {
          my ($command, @condition) = split;

          if ( $#if_stack == -1 ) {
            ERROR2("$wizname:$linecount: Unbalanced #\$ELSIF statement");
          }
          else {
            # Revert IF->ELSE condition
            $if_stack[$#if_stack] = (1 - $if_stack[$#if_stack]);

            # Evaluate IF condition
            my $ret = dump_WIZ_cond("@condition", \@param_name, \@args);

            # Set condition flag
            push @if_stack, $ret;
            push @if_weight, $if_weight[$#if_weight] + 1;
          }
        }
        elsif ( /^\#\$ELSE/ ) {
          if ( $#if_stack == -1 ) {
            ERROR2("$wizname:$linecount: Unbalanced #\$ELSE statement");
          }
          else {
            # Revert IF->ELSE condition
            $if_stack[$#if_stack] = (1 - $if_stack[$#if_stack]);
          }
        }
        elsif ( /^\#\$ENDIF/ ) {
          if ( $#if_stack == -1 ) {
            ERROR2("$wizname:$linecount: Unbalanced #\$ENDIF statement");
          }
          else {
            # Pop if condition stack with the proper weight,
            # depending how many ELSIF statements were put.
            my $count = $if_weight[$#if_weight];

            while ( $count > 0 ) {
              pop @if_stack;
              pop @if_weight;
              $count--;
            }
          }
        }
      }
    }
    else {
      # Other lines are code lines to be dumped
      # Skip them if current condition if FALSE
      if ( dump_WIZ_ok(\@if_stack) ) {
        # Check the number of arguments
        if ( $#param_name < $#args ) {
          ERROR("Too many arguments passed to wiz \"$keyword\"");
          last;
        }

        # Replace parameter tags with actual value
        my $line = replace_parameters($_, \@param_name, \@args);

        # Dump line
        print STDOUT "$line\n";
      }
    }
  }

  close(WIZ);
  pop @wizdef_stack;

  # Dump wiz expansion footer
  if ( $show_headfoot != 0 ) {
    print STDOUT "  TestFarm::Engine::echo('-STEP END $keyword @args');\n";
    print STDOUT "  if (\$verdict != 0) { print \"*** TERMINATED AT STEP '$keyword', LINE $linecount\\n\"; goto POSTAMBLE; }\n";
  }
  print STDOUT "\n";

  # Check condition stack
  if ( $#if_stack != -1 ) {
    ERROR2("$wizname:$linecount: Unbalanced #\$IF/ENDIF statements");
  }

  return 0;
}


sub dump_WIZ_cond {
  my ($condition, $param_name, $args) = @_;

  # Replace wiz parameter in condition
  my $expression = replace_parameters($condition, $param_name, $args);

  # Catch warning messages while evaluating
  local $SIG{__WARN__} = sub { WARN("@_") };

  # Evaluate condition
  my $ret = eval($expression) ? 1:0;
  if ( $@ ) {
    chomp $@;
    ERROR("Failed to evaluate condition '$condition' (evaluated as '$expression'):");
    ERROR0("$@");
    $ret = 0;
  }

  # Restore initial warning messages handler
  local $SIG{__WARN__} = 'DEFAULT';

  return $ret;
}


sub dump_WIZ_ok {
  my $stack = shift;
  my $result = 1;

  foreach ( @$stack ) {
    if ( $_ == 0 ) {
      $result = 0;
    }
  }

  return ($result != 0);
}


sub replace_parameters {
  my ($line, $param_name, $args) = @_;
  my $count = 0;

  # Replace explicit parameters
  foreach ( @$param_name ) {
    if ( $count > $#$args ) { last };
    $line =~ s/<$_>/$args->[$count]/g;
    $count++;
  }

  # Replace implicit parameters
  $line =~ s/<__packname__>/$packname/g;
  $line =~ s/<__filename__>/$filename/g;
  $line =~ s/<__dirname__>/$dirname/g;
  $line =~ s/<__ret__>/$wizgen_ret/g;

  return $line;
}


sub gen_WIZ {
  my $call = shift;
  my $modname = "";

  # Extract package name from call statement
  my ($package, $function) = split '::', $call;

  # Check a function call is actually present
  if ( ! defined $function ) {
    ERROR("Badly formatted PERL function call in \"#\$WIZGEN\" statement");
    return "";
  }

  # Retrieve definition file for the wiz
  foreach ( @INC ) {
    $modname = "$_/$package.pm";
    if ( -f $modname ) { last }
  }

  if ( $modname eq "" ) {
    ERROR("Cannot find wiz code generator module \"$package\". Please set environment variable TESTFARM_WIZLIB properly");
    return "";
  }

  INFO(3, "Loading wiz code generator module \"$modname\"");
  dump_LINELOC($modname, 0);

  # Catch warning messages while evaluating
  local $SIG{__WARN__} = sub { WARN("@_") };

  # Load generator module
  eval("require $package");
  if ( $@ ) {
    chomp $@;
    ERROR("Failed to load code generator module \"$modname\":");
    ERROR0("$@");
    return $modname;
  }

  # Import and Execute generator function
  location_push($input_file, $input_line);
  $wizgen_ret = eval("$call");
  location_pop();

  # Restore initial warning messages handler
  local $SIG{__WARN__} = 'DEFAULT';

  return $modname;
}


__END__

#
# DOCUMENTATION
#

=head1 NAME

twiz-script - TestFarm Script Wizard

=head1 SYNOSPIS

twiz-script [-v] [-q|--quiet] [--colors] [--nocolors] I<wiz-script> ...

=head1 DESCRIPTION

This tool processes TestFarm wiz-scripts and produces target modules that can be directly used by the TestFarm runtime environment.
Typically, the target module are PERL module files (*.pm).

=over

=item I<wiz-script> ...

The source wiz-scripts to process.
If the name begins with '@', the file is interpreted as a list of input files, containing
one wiz-script file name per line.

=item B<-v>

Be verbose, i.e. display some information messages while processing.
This option can be given several times, allowing many levels of verbosity:

1. "B<-v>" displays information about what is produced in the target file.

2. "B<-v -v>" in addition to 1., displays information about what is read in the source wiz-script.

3. "B<-v -v -v>" in addition to 2., displays information about what is read in the wiz definition files.

=item B<-q>, B<--quiet>

Be quiet, i.e. do not display a status message after a script is processed.
This also implies verbosity level to 0.

=item B<--colors>, B<--nocolors>

Enable or disable colored error and warning messages.
When enabled, error message are displayed in red and warning messages are displayed in blue.

=back

=head1 WIZ SCRIPT COMMANDS

=head2 #$DESCRIPTION I<text>

=head2 #$REFERENCE I<text>

=head2 #$PRECONDITION I<expression>

=head2 #$CRITICITY I<criticity-id>

=head2 INCLUDE I<wiz-script> ...

=head2 I<wiz-keyword> [I<param> ...]

=head2 # I<comment>

=head1 WIZ DEFINITION COMMANDS

=head2 #$GIVE_UP

=head2 #$PARAM I<identifier> I<format> I<description>

=head2 #$WIZCALL I<wiz-keyword> [I<param> ...]

=head2 #$WIZGEN I<module>::I<function>(I<parameters>)

=head2 #$IF I<condition>

=head2 #$ELSIF I<condition>

=head2 #$ELSE

=head2 #$ENDIF

=head2 ## I<inline-comment>

=head2 # I<comment>

=cut
