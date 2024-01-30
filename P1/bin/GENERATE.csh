#! /bin/csh -f

set CWD = `pwd`
cd ..
set TEST_HOME = `pwd`
cd $CWD

foreach prog (my-cat my-grep my-zip my-unzip)
    set test_script = "./test-${prog}.csh"
    echo "CREATING $test_script"

    echo "#! /bin/csh -f" > $test_script
    echo "set TEST_HOME = $TEST_HOME" >> $test_script
    echo "set source_file = ${prog}.c" >> $test_script
    echo "set binary_file = ${prog}" >> $test_script
    echo "set bin_dir = ${TEST_HOME}/bin" >> $test_script
    echo "set test_dir = ${TEST_HOME}/tests-${prog}" >> $test_script
    echo '${bin_dir}/generic-tester.py -s $source_file -b $binary_file -t $test_dir $argv[*]' >> $test_script
    chmod +x $test_script
end
