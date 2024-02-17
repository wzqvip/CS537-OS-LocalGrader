#! /bin/csh -f
set TEST_HOME = `pwd`
set source_file = wsh.c
set binary_file = wsh
set bin_dir = ${TEST_HOME}/bin
set test_dir = ${TEST_HOME}/variables
set curr_dir = `pwd`

env CURR_DIR=${curr_dir} TEST_HOME=${TEST_HOME} PROMPT='wsh> ' ${bin_dir}/p3-tester.py -s $source_file -b $binary_file -t $test_dir $argv[*]
