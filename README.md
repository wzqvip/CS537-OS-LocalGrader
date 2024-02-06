# CS537-OS-LocalGrader

Just clone things from the linux machine that can run on your machine. (But why I need this?)

Use some unix-style machine like WSL on Windows.

# Generic Grading

```
pregrade P{num}
```

num is the Project number.

e.g.

```
./pregrade P1
```

# P0

XD

# P1

Copy your `MADCounter.c` to the P1 directory. And run the following command in that directory.

```
./test-madcounter.csh
```

If you want to see all the test points, use `-c`

```
./test-madcounter.csh -c
```

To see complete output (and log to file).

```
./test-madcounter.csh -c -vv > log.txt
```

# P2

Copy the *xv6-public* folder to P2

Then go to your folder and execute the tests

```
cd P2/xv6-public
```

```
../tester/runtests -d ../initial-xv6/tests/ -c 
```

You may need to create a new directory called `test-out` in public-xv6 dir. If you encounter problems.

**Note: There are problems with test8. The user program is not included in the Makefile. I tried to include but it's too large for the xv6 system.**

-> I asked TA. The old version does have this problem. Then they reduce it from 12 to 8 tests. You may try to add `test_8.c(_test_8)` to your makefile and manually test it. I won't update this because this contains more test points and it's easy to test by yourself.
