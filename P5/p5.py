#! /bin/env python

import toolspath
from testing import Xv6Build, Xv6Test, pexpect

class test1(Xv6Test):
   name = "test_1"
   description = "Lock: Shared increment of counter by fn1 and fn2"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [r'2.*0.*0.*0.*0.*0.*0']

class test2(Xv6Test):
   name = "test_2"
   description = "Deadlock"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 5
   failure_pattern = ["[$][ ]"]
   success_pattern = [pexpect.TIMEOUT]

class test3(Xv6Test):
   name = "test_3"
   description = "Lock: fn1, fn2, main all incrementing shared counter"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [r'3.*0.*0.*0.*0.*0.*0']                    


class test4(Xv6Test):
   name = "test_4"
   description = "Scheduler: Priority Order: main > fn1"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [r'(#tester){5,}.*(#tester1){5,}']

class test5(Xv6Test):
   name = "test_5"
   description = "Scheduler: Pririty Order: fn1 > main"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [r'(#tester1){5,}.*(#tester){5,}']

class test6(Xv6Test):
   name = "test_6"
   description = "Scheduler: Pririty Order: main > fn1"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [r'(#tester){5,}.*(#tester1){5,}']

class test7(Xv6Test):
   name = "test_7"
   description = "Scheduler: Priority order: fn2 > main > fn1"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [r'(#tester2){5,}.*(#tester){5,}.*(#tester1){5,}']

class test8(Xv6Test):
   name = "test_8"
   description = "Scheduler: Priority Order: fn2 > fn1 > main"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [r'(#tester2){5,}.*(#tester1){5,}.*(#tester){5,}']    


class test9(Xv6Test):
   name = "test_9"
   description = "Scheduler: fn1 thread priority > main thread"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [r'(#tester1){5,}.*(#tester){5,}']


class test10(Xv6Test):
   name = "test_10"
   description = "Priority inheritance"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [
      r'(#tester1){5,}.*(#tester2){5,}.*(#tester){5,}'
   ]

class test11(Xv6Test):
   name = "test_11"
   description = "Priority Restoration"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [
      #r'(#tester1){5,}.*(#tester2){5,}.*(#tester){5,}.*(#tester1){5,}'
      r'(#tester1){5,}.*(#tester2){5,}.*(#tester1){5,}'
   ]

class test12(Xv6Test):
   name = "test_12"
   description = "Multiple contention"
   tester = "ctests/" + name + ".c"
   make_qemu_args = "CPUS=1"
   point_value = 1
   timeout = 15
   failure_pattern = []
   success_pattern = [
      r'(#tester3){5,}.*(#tester){5,}'
   ]




import toolspath
from testing.runtests import main
main(Xv6Build, all_tests=[test1, test2, test3, test4, test5, test6, test7, test8, test9, test10, test11, test12])
