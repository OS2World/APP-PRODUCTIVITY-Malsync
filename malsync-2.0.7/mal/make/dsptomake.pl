#!/usr/bin/perl

#
# The contents of this file are subject to the Mozilla Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is Mobile Application Link.
#
# The Initial Developer of the Original Code is AvantGo, Inc.
# Portions created by AvantGo, Inc. are Copyright (C) 1997-1999
# AvantGo, Inc. All Rights Reserved.
#
# Contributor(s):
#

use strict;
use FileHandle;

my ($m, $o, $mfile, $ofile, $infile);

# Open input file
$infile = new FileHandle(shift @ARGV);
if (!$infile) {
    die "Syntax: perl dsptomake <dspfile> [build.mk file] [ofiles.mk]";
}

# Open outfile for build.mk
if (!($m = shift @ARGV)) {
    $m = 'build.mk';
}
$mfile = new FileHandle("> $m");

# Open outfile of ofiles.mk
if (!($o = shift @ARGV)) {
    $o = 'ofiles.mk';
}
$ofile = new FileHandle("> $o");

# cruise through infile

$ofile->print("OFILES =");

my $count = 0;
while (my $line = <$infile>) {                # Foreach line
    if ($line =~ /SOURCE=.*\.c.*$/) {          # if we see SOURCE=[...].c
        $line =~ s/\\/\//gi;                  # change '\' to '/'
        my ($source, $line) = split ('=', $line);
				    # pick out the right side of SOURCE=[...]
        $line =~ /(.*)\/(\w*)\.c/;  # pick out the path and the source;
        
        # Dump a line to mfile
        $mfile->print("\$(OBJDIR)/$2.o: $1/$2.c\n\t\@echo \"\"\n\t\$(CC) -c \$< \$(CFLAGS) -o \$\@\n\n");
    
        # Dump a line to ofile
        $ofile->print(" \\\n\$(OBJDIR)/$2.o");
        $count++;
    }
}

$mfile->close;
$ofile->close;

print "Processed $count .c files\n";
