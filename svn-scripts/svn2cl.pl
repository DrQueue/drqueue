#!/usr/bin/perl -w

##
#
# Hacked version of some script I found whilst Googling.
# I modified it a bit to have a nicer ChangeLog.
#
# Example:
# --------
#
# svn log http://svn.freenode.net/newircd | perl svn2cl.pl > ChangeLog-newircd
#
##

require 5.0;
use strict;

my %hackers = (
    "jorge"     => "Jorge Daza <jorge\@drqueue.org>",
    "caedes"    => "Pablo Martin <caedes\@sindominio.net>",
    "kraken"	=> "Vincent Dedun <kraken+spam\@smousseland.com>",
    "ender"	=> "David Martínez Moreno <ender\@debian.org>",
		"cgTobi" => "Tobias Sauerwein <cgTobi@gmail.com>"
);

my $parse_next_line = 0;
my $last_line_empty = 0;
my $last_rev = "";

while (my $entry = <>) {

  # Axe windows style line endings, since we should try to be consistent, and
  # the repos has both styles in its log entries
  $entry =~ s/\r\n$/\n/;

  # Remove trailing whitespace
  $entry =~ s/\s+$/\n/;

  my $this_line_empty = $entry eq "\n";

  # Avoid duplicate empty lines
  next if $this_line_empty and $last_line_empty;

  # Don't fail on valid dash-only lines
  if ($entry =~ /^-+$/ and length($entry) >= 72) {

    # We're at the start of a log entry, so we need to parse the next line
    $parse_next_line = 1;

    # Check to see if the final line of the commit message was blank,
    # if not insert one
    print "\n" if $last_rev ne "" and !$last_line_empty;

  } elsif ($parse_next_line) {

    # Transform from svn style to GNU style
    $parse_next_line = 0;

    my @parts = split (/ /, $entry);
    $last_rev  = $parts[0];
    my $hacker = $parts[2];
    my $tstamp = $parts[4];
    my $time   = $parts[5];

    # Use alias if we can't resolve to name, email
    $hacker = $hackers{$hacker} if defined $hackers{$hacker};

    printf "%s %s  %s\n\n", $tstamp, $time, $hacker;

  } elsif ($this_line_empty) {

    print "\n";

  } else {

    if (($entry !~ /^-/) && ($entry !~ /^ /)) {
       print "\t* $entry";
    } elsif ($entry =~ /^-/) {
       print "\t   $entry";
    } else {
       print "\t$entry";
    }
  }

  $last_line_empty = $this_line_empty;
}

1;
