import shutil, os, subprocess
from testing import Test, BuildTest, pexpect

class Xv6Test(BuildTest, Test):
   name = "all"
   description = "build xv6 using make"
   timeout = 30
   tester = "tester.c"
   make_qemu_args = ""
   point_value=0
   success_pattern = 'SUCCESS'
   failure_pattern = 'FAILED'

   def __call__(self):
       return run(self)

   def run(self):
      tester_path = self.test_path + "/" + self.tester
      self.log("Running xv6 user progam " + str(tester_path))
      shutil.copy(tester_path, self.project_path + "/tester.c")

      # shawgerj copy Makefile to Makefile.test and gawk tester.c into UPROGS
#      cmd = "gawk '($1 == \"_mkdir\\\") { printf(\"\t_tester\\\n\"); } { print $0 }'"
      cmd = [self.test_path + "/edit-makefile.sh",
             self.project_path + "/Makefile",
             self.project_path + "/Makefile.test"]

      # with open(self.test_path + "/Makefile", "r") as m:
      #    with open(self.project_path + "/Makefile.test", "w+") as mtest:
      #       subprocess.Popen(cmd, stdin=m, stdout=mtest, shell=True)

      subprocess.Popen(cmd)
      is_success = self.make(["xv6.img", "fs.img"])
      if not is_success:
         return # stop test on if make fails

      target = "qemu-nox " + self.make_qemu_args
      if self.use_gdb:
         target = "qemu-gdb " + self.make_qemu_args
      self.log("make " + target)
      child = pexpect.spawn("make -f Makefile.test " + target,
            cwd=self.project_path,
            logfile=self.logfd,
            timeout=self.timeout)
      self.children.append(child)

      if self.use_gdb:
         gdb_child = subprocess.Popen(
               ["xterm", "-title", "\"gdb\"", "-e", "gdb"],
               cwd=self.project_path)
         self.children.append(gdb_child)

      child.expect_exact("init: starting sh")
      child.expect_exact("$ ")
      child.sendline("tester")
      child.expect_exact("tester")
       
      # check if test passed
      # Omid: Define an expected output for each test
      # Yurun: refactor list. failure/success_patterns are lists such as [(pattern, message), ...]
      default_failure_patterns = [
         ('Segmentation Fault', ''), 
         ("FAILED", "tester failed"), 
         ("cpu\d: panic: .*\n", "xv6 kernel panic"), 
         ("[$][ ]", "Tester terminates while no expected pattern is matched"), 
         # (pexpect.TIMEOUT, "Timeout")
         # (self.failure_pattern, "tester failed"), 
      ]
      
      failure_patterns = []
      if type(self.failure_pattern) is list:
         failure_patterns.extend((f, "tester failed") for f in self.failure_pattern)
      else:
         failure_patterns.append((self.failure_pattern, "tester failed"))
      failure_patterns.extend(default_failure_patterns)
      #if pexpect.TIMEOUT not in self.success_pattern:
      #  failure_patterns.append(pexpect.TIMEOUT)
      success_patterns = [ ]
      if type(self.success_pattern) is list:
         success_patterns.extend((s, '') for s in self.success_pattern)
      else:
         success_patterns.append((self.success_pattern, ''))

      # Failure patterns should go after success patterns, as patterns are matched in order
      patterns = list(map(lambda pair: pair[0], success_patterns + failure_patterns))
      index = child.expect(patterns)

      if index >= len(success_patterns):
         self.fail(failure_patterns[index - len(success_patterns)][1])

      if self.use_gdb:
         child.wait()
      else:
         child.close()
      self.done()

class Xv6Build(BuildTest):
   name = "build"
   description = "build xv6 using make"
   timeout = 60
   targets = ["xv6.img", "fs.img"]


   def __call__(self):
       return self.done()
