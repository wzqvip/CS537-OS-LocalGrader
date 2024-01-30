#! /bin/csh -f
set TEST_HOME = `pwd`
set source_file = MADCounter.c
set binary_file = MADCounter
set bin_dir = ${TEST_HOME}/bin
set test_dir = ${TEST_HOME}/tests-madcounter

${bin_dir}/generic-tester.py -s $source_file -b $binary_file -t $test_dir $argv[*]
