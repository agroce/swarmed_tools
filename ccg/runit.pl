#!/usr/bin/perl -w

use strict;
use Proc::Background;
use BSD::Resource;

my @compilers = (
    "gcc-4.7",
    "gcc-4.8",
    "/home/regehr/z/compiler-install/gcc-r205234-install/bin/gcc",
    "clang",
    "/home/regehr/z/compiler-install/llvm-r195376-install/bin/clang",
    );

my $success = setrlimit(0,30,30);
die unless ($success);

my $i=0;
while (1) {
    if ($i%1==0) { print "$i\n"; }
    my $done;
    do {
	$done = 1;
	my $res = system "./ccg --swarm > foo.c";
	if ($res == 0) {
	    my $size = -s "foo.c";
	    print "ccg succeeded with $size bytes of output\n";
	    if ($size < 2000) {
		print "too small\n";
		$done = 0;
	    }	    
	} else {
	    print "ccg timed out\n";
	    $done = 0;
	}
    } while (!$done);
    my @procs = ();
    my $x = 0;
    foreach my $comp (@compilers) {
	my $cmd = "$comp -w -O3 foo.c -o exe_${x}";
	#print "$cmd\n";
	my $proc = Proc::Background->new($cmd);
	push @procs, $proc;
	$x++;
    }
    foreach my $proc (@procs) {
	$proc->wait;
    }
    $i++;
}
