#!/bin/bash

$(dirname $0)/runtests fork $@
exit $?
