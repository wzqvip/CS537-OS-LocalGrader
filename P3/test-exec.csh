#! /bin/csh -f
set TEST_HOME = /p/course/cs537-oliphant/tests/P3
set source_file = wsh.c
set binary_file = wsh
set bin_dir = ${TEST_HOME}/bin
set test_dir = ${TEST_HOME}/exec

env TEST_HOME=${TEST_HOME} PROMPT='wsh> ' ${bin_dir}/p3-tester.py -s $source_file -b $binary_file -t $test_dir $argv[*]
