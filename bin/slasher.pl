#!/usr/bin/perl

while (<>) {
  if (/\\\\.*Dino/i) {
      $_ =~ s/\\\\/\//g;
  }
  print;
}
