#! /usr/bin/env python3

import argparse
import os
import signal
import subprocess
import sys
import time
import binascii

#
# helper routines
#

def read_file(f, do_binary):
    read_string = 'r'
    if do_binary:
        read_string = 'rb'
    with open(f, read_string) as content_file:
        content = content_file.read()
    return content

def dump_hex(binary, group=2):
    return b' '.join(binascii.hexlify(binary[i:i+group]) for i in range(0, len(binary), group))

def print_failed(msg, expected, actual, test_passed, verbosity):
    if test_passed: # up til now, no one said that the test failed
        print('RESULT failed')
    if verbosity >= 0:
        print(msg)
        if verbosity < 2 and hasattr(expected, '__len__') and len(expected) > 1000:
            print('Showing first 500 bytes; use -vv to show full output')
            expected = expected[:500]
            actual = actual[:500]
        if hasattr(expected, 'decode') or hasattr(actual, 'decode'): # binary, dump to hex
            expected = dump_hex(expected)
            actual = dump_hex(actual)
        print('expected: [%s]' % expected)
        print('got:      [%s]\n' % actual)
    return

# some code from:
# stackoverflow.com/questions/18404863/i-want-to-get-both-stdout-and-stderr-from-subprocess
def run_command(cmd, timeout_length, verbosity, do_binary):
    if verbosity >= 1:
        print('COMMAND', cmd)
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, preexec_fn=os.setsid)

    try:
        std_output, std_error = proc.communicate(timeout=timeout_length)
    except subprocess.TimeoutExpired:
        if verbosity >= 0:
            print('WARNING test timed out (i.e., it took too long)')
        # proc.kill()
        os.killpg(os.getpgid(proc.pid), signal.SIGTERM)
        std_output, std_error = proc.communicate()
    error_code = proc.returncode

    if not do_binary:
        # Python3 subprocess exec() sometimes returns bytes not a string -
        # make sure to turn it into ascii before treating as string
        output_actual = std_output.decode('ascii', 'ignore')
        error_actual = std_error.decode('ascii', 'ignore')
    else:
        # does 
        output_actual = std_output
        error_actual = std_error
    rc_actual = int(error_code)

    return output_actual, error_actual, rc_actual

def handle_outcomes(stdout_expected, stdout_actual, stderr_expected, stderr_actual, rc_expected, rc_actual, verbosity, total_time):
    test_passed = True
    if verbosity != -1 and total_time is not None:
        print('Test finished in %.3f seconds' % total_time)
    if rc_actual != rc_expected:
        print_failed('return code does not match expected', rc_expected, rc_actual, test_passed, verbosity)
        test_passed = False
    if stdout_expected != stdout_actual:
        print_failed('standard output does not match expected', stdout_expected, stdout_actual, test_passed, verbosity)
        test_passed = False
    if stderr_actual != stderr_expected:
        print_failed('standard error does not match expected', stderr_expected, stderr_actual, test_passed, verbosity)
        test_passed = False
    if test_passed:
        print('RESULT passed')
        return True
    return False

def test_exists(test_number, input_dir):
    run_file = '%s/%d.run' % (input_dir, test_number)
    return os.path.exists(run_file)

# return outcome of test AND whether test exists
def test_one(test_number, input_dir, timeout_length, verbosity, show_timing):
    do_binary = os.path.exists('%s/%d.binary' % (input_dir, test_number))
    run_desc = read_file('%s/%d.run' % (input_dir, test_number), False).strip().replace('INPUT_DIR', input_dir)
    test_desc = read_file('%s/%d.desc' % (input_dir, test_number), False).strip()

    stdout_expected = read_file('%s/%d.out' % (input_dir, test_number), do_binary)
    stderr_expected = read_file('%s/%d.err' % (input_dir, test_number), do_binary)
    rc_expected = int(read_file('%s/%d.rc' % (input_dir, test_number), False))

    print('TEST %d - %s' % (int(test_number), test_desc))
    if os.path.exists('%s/%d.prep' % (input_dir, test_number)): # do some prepare work
        prep_desc = read_file('%s/%d.prep' % (input_dir, test_number), False).strip().replace('INPUT_DIR', input_dir)
        try:
            subprocess.check_call(prep_desc, timeout=20, shell=True)
        except:
            print('WARN preparation work failed, may need to try again')
    if do_binary and verbosity >= 0:
        print('NOTE output in hex not ascii text')
    if os.path.exists('%s/%d.timeout' % (input_dir, test_number)):
        timeout_length = int(read_file('%s/%d.timeout' % (input_dir, test_number), False))
    if show_timing:
        start_time = time.time()
    stdout_actual, stderr_actual, rc_actual = run_command(run_desc, timeout_length, verbosity, do_binary)
    if show_timing:
        total_time = time.time() - start_time
    else:
        total_time = None

    return handle_outcomes(stdout_expected, stdout_actual, stderr_expected, stderr_actual, rc_expected, rc_actual, verbosity, total_time)

#
# main test code
#
parser = argparse.ArgumentParser()
parser.add_argument('-c', '--continue', dest='continue_testing', help='continue testing even when a test fails', action='store_true') 
parser.add_argument('-S', '--start_test', dest='start_test', help='start with this test number', type=int, default=1)
parser.add_argument('-E', '--end_test', dest='end_test', help='end with this test number; -1 means go until done', type=int, default=-1)
parser.add_argument('-s', '--source_files', dest='source_file', help='comma-sep list of names of source file to test', type=str, default='')
parser.add_argument('-b', '--binary_file', dest='binary_file', help='name of binary to produce', type=str, default='a.out')
parser.add_argument('-t', '--test_dir', dest='test_directory', help='path to location of tests', type=str, default='')
parser.add_argument('-T', '--timeout', dest='timeout_length', help='length of timeout in seconds', type=int, default=30)
parser.add_argument('--timed', dest='show_timing', help='show time taken by each test in seconds', action='store_true')
parser.add_argument('-f', '--build_flags', help='extra build flags for gcc', type=str, default='')
parser.set_defaults(verbosity=0)
group = parser.add_mutually_exclusive_group()
group.add_argument('-q', '--quiet', dest='verbosity', help='only display test number and result', action='store_const', const=-1)
group.add_argument('-v', '--verbose', dest='verbosity', help='show command line so you can replicate for debugging, and also some other extra information', action='count')
args = parser.parse_args()

input_file_list = args.source_file
binary_file = args.binary_file
input_dir = args.test_directory

input_file = ''
for f in input_file_list.split(','):
    if input_file == '':
        input_file = f
    else:
        input_file += ' %s' % f


print('TEST 0 - clean build (program should compile without errors or warnings)')
if args.show_timing:
    start_time = time.time()
stdout_actual, stderr_actual, rc_actual = run_command('gcc %s -o %s -Wall -Werror %s' % (input_file, binary_file, args.build_flags), 
                                                      args.timeout_length, args.verbosity, do_binary=False)
if args.show_timing:
    total_time = time.time() - start_time
else:
    total_time = None
if handle_outcomes('', stdout_actual, '', stderr_actual, 0, rc_actual, args.verbosity, total_time) == False:
    exit(1)
print('')

test_number = args.start_test
while True:
    # in this case, we are all done
    if not test_exists(test_number, input_dir):
        if args.end_test != -1:
            print('ERROR cannot run test %d; it does not exist' % test_number)
        break

    test_result = test_one(test_number, input_dir, args.timeout_length, args.verbosity, args.show_timing)
    print('')

    if not args.continue_testing and not test_result:
        print('TESTING HALTED (use -c or --continue to keep going if you like)')
        exit(1)

    test_number += 1
    if args.end_test != -1 and test_number > args.end_test:
        break

exit(0)


