#! /usr/bin/env python3

import argparse
import os
import signal
import subprocess
import sys
import time
import binascii
import re

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

def execute_bash_command(cmd, timeout_length, do_binary):
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, preexec_fn=os.setsid)

    try:
        std_output, std_error = proc.communicate(timeout=timeout_length)
    except subprocess.TimeoutExpired:
        print('WARNING test timed out (i.e., it took too long)')
        # proc.kill()
        os.killpg(os.getpgid(proc.pid), signal.SIGTERM)
        std_output, std_error = proc.communicate()
    error_code = proc.returncode
    #print(error_code)
    if not do_binary:
        # Python3 subprocess exec() sometimes returns bytes not a string -
        # make sure to turn it into ascii before treating as string
        output_actual = std_output.decode('utf-8', 'ignore').replace('…', '...')
        error_actual = std_error.decode('utf-8', 'ignore')
    else:
        # does 
        output_actual = std_output
        error_actual = std_error
    rc_actual = int(error_code)
    #print(rc_actual)
    return output_actual, error_actual, rc_actual

# some code from:
# stackoverflow.com/questions/18404863/i-want-to-get-both-stdout-and-stderr-from-subprocess
def run_command(run, timeout_length, verbosity):
    cmd = f'script -E never -qc \'tty; sleep 0.1; ./{binary_file}\' /dev/null'

    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, preexec_fn=os.setsid, text=True)
    # time.sleep(0.5)
    if verbosity >= 2:
        print('COMMAND', cmd)

    def flushed_write_to_proc(proc, content):
            proc.stdin.write(content)
            proc.stdin.flush()

    status_table = {
        'D': 'uninterruptible sleep (usually IO)',
        'I': 'Idle kernel thread',
        'R': 'running or runnable (on run queue)',
        'S': 'interruptible sleep (waiting for an event to complete)',
        'T': 'stopped by job control signal',
        't': 'stopped by debugger during the tracing',
        'W': 'paging (not valid since the 2.6.xx kernel)',
        'X': 'dead (should never be seen)',
        'Z': 'defunct ("zombie") process, terminated but not reaped by its parent',
    }

    # Sending commands to wsh
    run.replace('\r\n', '\n')
    error_messages = []
    tty = proc.stdout.readline().removesuffix('\n')
    if verbosity >= 3:
        print(f'TTY for script: {tty}')
    time.sleep(0.5)
    for shell_cmd in run.split('\n'):
        if (shell_cmd.startswith('#') and not shell_cmd.startswith('##')) or shell_cmd == '':
            pass
        elif shell_cmd.startswith('##'):
            if shell_cmd.startswith('##assertStat'):
                stripped = shell_cmd.removeprefix('##assertStat ')
                stat = stripped[0]
                target_cmd = stripped[2:]

                if verbosity >= 1:
                    print('Assert status of process ', target_cmd, ' ', stat, ' ', status_table.get(stat))

                ps_cmd = f'ps -t {tty} -o args,s | grep \'{target_cmd}\\b\''
                if verbosity >= 2:
                    print('COMMAND', ps_cmd)
                ps_output, ps_error, ps_rc = execute_bash_command(ps_cmd, timeout_length, False)
                line_num = ps_output.count('\n')
                if line_num > 1:
                    err = f'TESTING SCRIPT: Found {line_num} processes \"{target_cmd}\". Terminate all such instances before testing\n'
                    print(err)
                    if verbosity >= 3:
                        print(f'Output from ps :[{ps_output}]')
                    error_messages.append(err)
                if stat == 'X':
                    if line_num != 0:
                        error_messages.append(f'ERROR: During test, command {target_cmd} is expected to be terminated. '
                                               + (f'Found [{ps_output}]' if verbosity >= 2 else ''))
                else:
                    err_flag = False
                    error_msg = f'ERROR: During test, command {target_cmd} is expected to have status {stat} ({status_table.get(stat)}). '
                    if line_num == 0:
                        error_msg = error_msg + 'However, the command is terminated. ' + (f'Found [{ps_output}]' if verbosity >= 2 else '')
                        err_flag = True
                    else:
                        actual_stat = ps_output.removesuffix('\n')[-1]
                        if actual_stat != stat:
                            error_msg = error_msg + f'However, the command has status {actual_stat} ({status_table.get(actual_stat)})'
                            err_flag = True
                    if err_flag:
                        error_messages.append(error_msg)
            else:
                cmd = shell_cmd.removeprefix('##')
                if verbosity >= 2:
                    print('COMMAND', cmd)
                execute_bash_command(cmd, timeout_length, False)
        elif shell_cmd.startswith('\\'):
            code = eval('\'' + shell_cmd + '\'')
            if verbosity >= 1:
                if code == '\x03':
                    display = 'Ctrl+C'
                elif code == '\x1A':
                    display = 'Ctrl+Z'
                else:
                    display = code
                print('SHELL INPUT ', display)
            try:
                flushed_write_to_proc(proc, code)
            except:
                try:
                    std_output, std_error = proc.communicate(timeout=0.5)
                    error_code = proc.returncode
                    error_messages.append(f'Program terminated while there is still pending input. Output [{std_output}]. Stderr [{std_error}]')
                    return std_output, std_error, int(error_code), error_messages
                except subprocess.TimeoutExpired:
                    print('Failed to write input to shell although the shell does not terminate')
                    exit(-1)
        else:
            if verbosity >= 1:
                print('SHELL INPUT ', shell_cmd)
            try:
                flushed_write_to_proc(proc, f'{shell_cmd}\n')
            except:
                try:
                    std_output, std_error = proc.communicate(timeout=0.5)
                    error_code = proc.returncode
                    error_messages.append(f'Program terminated while there is still pending input. Output [{std_output}]. Stderr [{std_error}]')
                    return std_output, std_error, int(error_code), error_messages
                except subprocess.TimeoutExpired:
                    print('Failed to write input to shell although the shell does not terminate')
                    exit(-1)

    try:
        std_output, std_error = proc.communicate(timeout=timeout_length)
    except subprocess.TimeoutExpired:
        if verbosity >= 0:
            print('WARNING test timed out (i.e., it took too long)')
        # proc.kill()
        os.killpg(os.getpgid(proc.pid), signal.SIGTERM)
        std_output, std_error = proc.communicate()
    error_code = proc.returncode
    #print(error_code)
    ps_output = std_output
    ps_error = std_error
    ps_rc = int(error_code)
    #print(rc_actual)
    return ps_output, ps_error, ps_rc, error_messages

def replace_env_vars(input_string):
    # Define the regular expression pattern with a capturing group
    pattern = r'\${(.+?)}'

    # Find all matches and extract the captured content
    matches = re.finditer(pattern, input_string)

    # Replace each match with the corresponding environment variable value
    output_string = input_string
    for match in matches:
        var_name = match.group(1)
        if var_name in os.environ:
            var_value = os.environ[var_name]
            output_string = output_string.replace(match.group(0), var_value)
    
    return output_string

def handle_outcomes(stdout_expected, stdout_actual, stderr_expected, stderr_actual, rc_expected, rc_actual,
                    fout_name, fout_actual, fout_expected, verbosity, total_time, extra_err_msg=[]):
    test_passed = True
    if verbosity != -1 and total_time is not None:
        print('Test finished in %.3f seconds' % total_time)
    if rc_actual != rc_expected:
        print_failed('return code does not match expected', rc_expected, rc_actual, test_passed, verbosity)
        test_passed = False
    stdout_actual = stdout_actual.replace('\r\n', '\n')
    
    stdout_expected = replace_env_vars(stdout_expected.replace('\r\n', '\n'))
    stderr_actual = stderr_actual.replace('\r\n', '\n')
    stderr_expected = replace_env_vars(stderr_expected.replace('\r\n', '\n'))
    if stdout_expected != stdout_actual:
        print_failed('standard output does not match expected', stdout_expected, stdout_actual, test_passed, verbosity)
        test_passed = False
    if stderr_actual != stderr_expected:
        print_failed('standard error does not match expected', stderr_expected, stderr_actual, test_passed, verbosity)
        test_passed = False
    if len(extra_err_msg) != 0:
        print('Some error happened during testing: ')
        for err in extra_err_msg:
            print(err)
        test_passed = False
    if fout_name is not None:
        for i in range(fout_name):
            if fout_actual[i] != fout_expected[i]:
                print_failed("output file %s does not match expected" % fout_name[i], fout_expected[i], fout_actual[i],
                             test_passed, verbosity)
                test_passed = False
                break
    if test_passed:
        print('RESULT passed')
        return True
    return False

def test_exists(test_number, input_dir):
    run_file = '%s/%d.run' % (input_dir, test_number)
    return os.path.exists(run_file)

# return outcome of test AND whether test exists
def test_one(test_number, input_dir, timeout_length, verbosity, show_timing):
    run = read_file('%s/%d.run' % (input_dir, test_number), False).strip().replace('INPUT_DIR', input_dir)
    test_desc = read_file('%s/%d.desc' % (input_dir, test_number), False).strip()
    pattern = r'Score: (\d+)'
    match = re.search(pattern, test_desc)
    if match:
        # Extract the digit using group(1)
        score = int(match.group(1))
    else:
        score = 1

    stdout_expected = read_file('%s/%d.out' % (input_dir, test_number), False)
    stderr_expected = read_file('%s/%d.err' % (input_dir, test_number), False)
    rc_expected = int(read_file('%s/%d.rc' % (input_dir, test_number), False))

    cmp_fout = os.path.exists('%s/%d.fout' % (input_dir, test_number))
    fout_expected = []
    fout_actual = []
    fout_name = None
    if cmp_fout:
        fout_name = read_file('%s/%d.fout' % (input_dir, test_number), False).splitlines()
        for f in fout_name:
            fout_expected.append(read_file('%s/%s' % (input_dir, f), False))

    print('TEST %d - %s' % (int(test_number), test_desc))
    if os.path.exists('%s/%d.prep' % (input_dir, test_number)): # do some prepare work
        prep_desc = read_file('%s/%d.prep' % (input_dir, test_number), False).strip().replace('INPUT_DIR', input_dir)
        try:
            subprocess.check_call(prep_desc, timeout=20, shell=True)
        except:
            print('WARN preparation work failed, may need to try again')
    if os.path.exists('%s/%d.timeout' % (input_dir, test_number)):
        timeout_length = int(read_file('%s/%d.timeout' % (input_dir, test_number), False))
    if show_timing:
        start_time = time.time()
    
    stdout_actual, stderr_actual, rc_actual, extra_error_messages = run_command(run, timeout_length, verbosity)
    if show_timing:
        total_time = time.time() - start_time
    else:
        total_time = None

    if cmp_fout:
        for f in fout_name:
            fout_actual.append(read_file('%s' % f, False))
    if verbosity >= 3:
        print('Actual stdout: ')
        for char in stdout_actual:
            ascii_value = ord(char)
            print(f'{ascii_value}: {char}, ', end='')
    return handle_outcomes(stdout_expected, stdout_actual, stderr_expected, stderr_actual, rc_expected, rc_actual,
                           fout_name, fout_actual, fout_expected, verbosity, total_time, extra_error_messages), score

#
# main test code
#
parser = argparse.ArgumentParser()
parser.add_argument('-c', '--continue', dest='continue_testing', help='continue testing even when a test fails', action='store_true')
parser.add_argument('-S', '--start_test', dest='start_test', help='start with this test number', type=int, default=1)
parser.add_argument('-E', '--end_test', dest='end_test', help='end with this test number; -1 means go until done', type=int, default=-1)
parser.add_argument('-s', '--source_file', dest='source_file', help='name of source file to test', type=str, default='')
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

input_file = args.source_file
binary_file = args.binary_file
input_dir = args.test_directory

print(binary_file.upper() + '...')
print('TEST 0 - clean build (program should compile without errors or warnings)')
if args.show_timing:
    start_time = time.time()
compile_cmd = 'gcc %s -o %s -Wall -Werror %s' % (input_file, binary_file, args.build_flags)
if args.verbosity >= 1:
    print(f'Compile cmd: {compile_cmd}')
stdout_actual, stderr_actual, rc_actual = execute_bash_command(compile_cmd, args.timeout_length, False)
if args.show_timing:
    total_time = time.time() - start_time
else:
    total_time = None
if handle_outcomes('', stdout_actual, '', stderr_actual, 0, rc_actual, None, [], [], args.verbosity, total_time) == False:
    exit(1)
print('')


test_score = 0
total_score = 0
test_number = args.start_test
while True:
    # in this case, we are all done
    if not test_exists(test_number, input_dir):
        if args.end_test != -1:
            print('ERROR cannot run test %d; it does not exist' % test_number)
        break

    test_result, score = test_one(test_number, input_dir, args.timeout_length, args.verbosity, args.show_timing)
    print('')

    if not args.continue_testing and not test_result:
        print('TESTING HALTED (use -c or --continue to keep going if you like)')
        exit(1)

    total_score += score
    if test_result:
        test_score += score

    test_number += 1
    if args.end_test != -1 and test_number > args.end_test:
        break

print(f'TEST SCORE: {test_score} / {total_score}\n\n')
exit(0)


