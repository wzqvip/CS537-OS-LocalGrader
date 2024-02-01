#! /bin/bash

if ! [[ -d src ]]; then
    echo "The src/ dir does not exist."
    echo "Your xv6 code should be in the src/ directory"
    echo "to enable the automatic tester to work."
    exit 1
fi

set PATH=`pwd`

# ~cs537-1/tests/P2/tester/run-tests.sh -c 
${PATH}/../tester/run-tests.sh -c


