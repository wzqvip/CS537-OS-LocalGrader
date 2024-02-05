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
