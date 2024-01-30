#! /bin/csh -f

set bindir = `pwd`
cd ..
set homedir = `pwd`
cd $bindir

# foreach f (test-my-cat.csh test-my-grep.csh test-my-zip.csh test-my-unzip.csh)
foreach f (test-my-cat.csh)
    echo "updating $f with root testing directory of ${homedir}"
    set sed_string = "s/TESTING_ROOT_DIRECTORY/${homedir}/g"
    echo $sed_string
end
