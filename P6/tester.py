#!/usr/bin/python3

import argparse
import requests
import subprocess
import sys
import socket
import timeit
import threading
import os
import io
import traceback
import time
import shutil
#sys.path.append('.')

# Constants
SUCCESS = 0

# Args class
class Args:
    def __init__(self, ct, st, w, s, q):
        self.client_threads = ct
        self.server_threads = st
        self.win_size = w
        self.init_table_size = s
        self.qsize = q

# General test
# The tester calls init, run, and clean, in order
class Test():
    def __init__(self, num=1):
        super(Test, self).__init__()
        self.args = Args(1, 1, 1, 1, 1) 
        self.num = num
        self.score = 0
        self.timeout = 30 # Seconds
        self.sync = True # Validates GET results if set - overwrites
        #                   client_threads and win_size with 1

        # Args
        self.win_size = 1
        self.client_threads = 1
        self.server_threads = 1
        self.init_table_size = 1000000
        self.workload_file = 'workload.txt'
        self.expected_file = 'solution.txt'
        
        self.baseline_tput = 0
        self.min_speedup = 2.0

    def run(self):
        self.score = 0
        print(f'Error: Run not implemented')
        return self.score

    def init(self):
        pass

    def run_client(self):
        global test_base
        # Limit concurrency to synchronous execution 
        if self.sync is True:
            self.win_size = 1

        cmd = ['./client',
                f'-w {self.win_size}',
                f'-n {self.client_threads}',
                f'-t {self.server_threads}',
                f'-s {self.init_table_size}',
                f'-f',
                f'-i{workload_base}/{self.workload_file}',
                f'-e{workload_base}/{self.expected_file}',
                ]

        # Set the validate flag
        if self.sync is True:
            cmd.append('-c')

        #print(f'cmd is {cmd}')

        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        #p = subprocess.Popen(cmd)
        std_out, std_err = 0, 0
        try:
            std_out, std_err = p.communicate(timeout=self.timeout)
            if p.returncode != SUCCESS:
                print(f'{std_err}')
                self.score = 0
    
        except subprocess.TimeoutExpired:
            print(f'Error: Test timed out')
        return p.returncode, std_out, std_err

    def get_baseline_tput(self):
        cmd = [f'{test_base}/slow_client',
                f'-w {self.win_size}',
                f'-n {self.client_threads}',
                f'-t {self.server_threads}',
                f'-s {self.init_table_size}',
                f'-f',
                f'-i{workload_base}/{self.workload_file}',
                f'-x{test_base}/slow_server',
                ]
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        std_out, std_err = 0, 0
        try:
            std_out, std_err = p.communicate(timeout=self.timeout)
            if p.returncode != SUCCESS:
                print(f'{std_err}')
                self.score = 0
    
        except subprocess.TimeoutExpired:
            print(f'Error: Test timed out')

        return self.get_tput(std_out.decode('utf-8'))

    def get_tput(self, outstr):
        s = outstr.split(' ')
        return float(s[4])

    def clean(self):
        pass
# ----------------------------------------- Tests ----------------------------------------- #

# Small number of PUTs with unique keys
class Test1(Test):
    def __init__(self):
        super().__init__(1)
        self.win_size = 1
        self.client_threads = 1
        self.server_threads = 1
        self.init_table_size = int(1e6)
        self.desc = 'Small number of PUTs with unique keys'
        self.sync = False 
        self.workload_file = 'workload1.txt'
    
    def run(self):
        self.score = 1

        rc, stdout, stderr = self.run_client()

        if rc is not SUCCESS:
            self.score = 0

        return self.score 


# Small number of PUTs/GETs with unique keys
class Test2(Test):
    def __init__(self):
        super().__init__(2)
        self.win_size = 1
        self.client_threads = 1
        self.server_threads = 1
        self.init_table_size = int(1e6)
        self.desc = 'Small number of PUTs/GETs with unique keys'
        self.sync = True
        self.workload_file = 'workload2.txt'
        self.expected_file = 'solution2.txt'
    
    def run(self):
        self.score = 1

        rc, stdout, stderr = self.run_client()

        if rc is not SUCCESS:
            self.score = 0

        return self.score 


# Large number of PUTs/GETs with unique keys
class Test3(Test):
    def __init__(self):
        super().__init__(3)
        self.win_size = 1
        self.client_threads = 1
        self.server_threads = 1
        self.init_table_size = int(1e6)
        self.desc = 'Large number of PUTs/GETs with unique keys'
        self.sync = True
        self.workload_file = 'workload3.txt'
        self.expected_file = 'solution3.txt'
    
    def run(self):
        self.score = 1

        rc, stdout, stderr = self.run_client()

        if rc is not SUCCESS:
            self.score = 0

        return self.score 

# Large number of PUTs/GETs with unique keys - multithreaded
class Test4(Test):
    def __init__(self):
        super().__init__(4)
        self.win_size = 1
        self.client_threads = 1
        self.server_threads = 6
        self.init_table_size = int(1e6)
        self.desc = 'Large number of PUTs/GETs with unique keys - multithreaded'
        self.sync = True
        self.workload_file = 'workload4.txt'
        self.expected_file = 'solution4.txt'
    
    def run(self):
        self.score = 1

        rc, stdout, stderr = self.run_client()

        if rc is not SUCCESS:
            self.score = 0

        return self.score 
    

# Large number of PUTs - Highly skewed - Performance
class Test5(Test):
    def __init__(self):
        super().__init__(5)
        self.win_size = 1
        self.client_threads = 10
        self.server_threads = 10
        self.timeout = 60
        self.init_table_size = int(1e5)
        self.desc = 'Multi-threaded PUT/GET correctness test'
        self.sync = True
        self.workload_file = 'workload5.txt'
        self.expected_file = 'solution5.txt'
    
    def run(self):
        self.score = 1

        rc, stdout, stderr = self.run_client()

        if rc is not SUCCESS:
            self.score = 0

        return self.score 


# Large number of PUTs - Uniform - Performance
class Test6(Test):
    def __init__(self):
        super().__init__(6)
        self.win_size = 100
        self.client_threads = 2
        self.server_threads = 4
        self.init_table_size = int(1e6)
        self.desc = 'Uniform workload perfromance test'
        self.num_runs = 5 # Average over 5 runs
        self.min_speedup = 3.0
        self.baseline_tput = 100 # K requests / S
        self.sync = False
        self.workload_file = 'workload6.txt'
    
    def run(self):
        self.score = 1

        agg_tput = 0
        bl_agg_tput = 0
        for i in range(self.num_runs):
            rc, stdout, stderr = self.run_client()

            if rc is SUCCESS:
                agg_tput += self.get_tput(stdout.decode('utf-8'))
                bl_agg_tput += self.get_baseline_tput()
            else:
                self.score = 0
                return self.score

        avg_tput = agg_tput / self.num_runs

        bl_avg_tput = bl_agg_tput / self.num_runs

        print(f'baseline: {bl_avg_tput} your solution: {avg_tput} \
                    \n(speedup = {avg_tput/bl_avg_tput})')
        if avg_tput < self.min_speedup * bl_avg_tput:
            self.score = 0
            print(f'Your solution should at least be {self.min_speedup} times faster \
than the baseline')

        return self.score 


# Runs all the tests by calling init, run, clean, respectively 
# Returns total score
def run_tests(tests):
    total_score = 0
    for test in tests:
        try:
            print(f'Running test {test.num} ({test.desc})...')
            test.init()
            total_score += test.run()
            if test.score:
                print(f'Test {test.num} passed')
            else:
                print(f'Test {test.num} failed')
            test.clean()
        except:
            test.clean()
            print(traceback.format_exc())

    return total_score


# Terminate both target and proxy processes 
def clean():
    pass

# Compiles using the make in cwd
def compile():
    try:
        cmd = ['make', 'all']
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = p.communicate(timeout=20)
        
        if p.returncode != 0:
            print(f'Error: making failed with rc={p.returncode}')
            print(err)
    except subprocess.TimeoutExpired:
        print(f'Error: compilation timed out')


test_path = os.getcwd() + '/testdir'
src_path = os.getcwd()
test_base = '/home/cs537-1/tests/P6'
workload_base = test_base + '/workloads/'

# Copy files and prepare testing enviornment
if os.path.exists(test_path):
    shutil.rmtree(test_path)
shutil.copytree(src_path, test_path)

# Compile the source for proxyserver
os.chdir(test_path)
compile()

# Tests to run
tests = [Test1(), Test2(), Test3(), Test4(), Test5(), Test6(),
         ]

ts = run_tests(tests)
print(f'Total score: {ts}')


clean()
