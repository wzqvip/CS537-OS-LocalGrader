#! /bin/csh -f
set TEST_HOME = `pwd`
set source_file = wsh.c
set binary_file = wsh
set bin_dir = ${TEST_HOME}/bin
set test_dir = ${TEST_HOME}/batch

env TEST_HOME=${TEST_HOME} test_dir=${TEST_HOME}/batch PROMPT='wsh> ' ${bin_dir}/batch-tester.py -s $source_file -b $binary_file -t $test_dir $argv[*]
