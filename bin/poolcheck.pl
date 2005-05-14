#!/usr/bin/perl

$dir = "/lot/s800/TAMANANA/users/renderbin/";
$sceneLineNum = 8;

for (;1;) {
  @files = glob ($dir."*");
  for $file (@files) {
    print "$file\n";
    if ($file =~ /\.translated/) {
      if (system ("sendjob $file") == 0) {
	system ("rm $file");
      }
    } else {
      if (translate ($file) == 0) {
	print "Translate successful\n";
	system ("rm $file");
      }
    }
  }
  sleep (5);
}

sub translate ($)
{
  print "Translating... ";
  $file = $_[0];
  open (FILE,$file);
  @lines = <FILE>;
  close (FILE);
  $old_filename = $lines[$sceneLineNum];
  chomp ($old_filename);
  $old_filename =~ s/\/\//\//ig;
  $old_filename =~ s/\/dino/\/lot\/s800/ig;
  $new_filename = $old_filename;
  $new_filename =~ s/\.ma$/\.trn.ma/;
  print "$old_filename -> $new_filename\n";
  # The next lines substitute the backslashes from syflex
  if (system ("slasher.pl $old_filename > $new_filename")) {
    print "Slasher.pl failure\n";
    return 1;
  }
  open (FILE,">".$file.".translated") || return 1;
  $i = 0;
  for $line (@lines) {
    if ($i == $sceneLineNum) {
      print FILE "$new_filename\n";
    } else {
      $line =~ s/\/\/dino/\/dino/i ;
      print FILE $line;
    }
    $i++;
  }
  close (FILE);
  print "Done !\n";
  return 0;
}
