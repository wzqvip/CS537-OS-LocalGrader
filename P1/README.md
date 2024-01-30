# P1 - Unix Utilities - MADCounter

## <p style="color:red">UPDATES</p>
- We will not be testing the Ascii character error. You should still be tracking only ASCII characters 0-127

## Administrivia 

- **Due Date**: January 30th, at 11:59pm
- Projects may be turned in up to 3 days late but you will receive a penalty of 10 percentage points for every day it is turned in late.
- **Slip Days**: 
  - In case you need extra time on projects, you each will have 2 slip days for individual projects and 3 slip days for group projects (5 total slip days for the semester). After the due date we will make a copy of the handin directory for on time grading. 
  - To use a slip days or turn in your assignment late you will submit your files with an additional file that contains only a single digit number, which is the number of days late your assignment is(e.g 1, 2, 3). Each consecutive day we will make a copy of any directories which contain one of these slipdays.txt files. This file must be present when you submit you final submission, or we won't know to grade your code. 
  - We will track your slip days and late submissions from project to project and begin to deduct percentages after you have used up your slip days.
  - After using up your slip days you can get up to 90% if turned in 1 day late, 80% for 2 days late, and 70% for 3 days late, but for any single assignment we won't accept submissions after the third days without an exception. This means if you use both of your individual slip days on a single assignment you can only submit that assignment one additional day late for a total of 3 days late with a 10% deduction.
  - Any exception will need to be requested from the instructors.

  - Example slipdays.txt
```
1
```

- Questions: We will be using Piazza for all questions.
- **Before beginning:** Read this [lab tutorial](http://pages.cs.wisc.edu/~remzi/OSTEP/lab-tutorial.pdf); it has some useful tips for programming in the C environment, including working with makefiles, using the gdb debugger, and using the linux manual and info systems.
- Collaboration: The assignment has to be done by yourself. Copying code (from others) is considered cheating. [Read this](http://pages.cs.wisc.edu/~remzi/Classes/537/Spring2018/dontcheat.html) for more info on what is OK and what is not. Please help us all have a good semester by not doing this.
- This project is to be done on the [Linux lab machines](https://csl.cs.wisc.edu/docs/csl/2012-08-16-instructional-facilities/), so you can learn more about programming in C on a typical UNIX-based platform (Linux).  Your solution will be tested on these machines.
- Some tests will be provided at *~cs537-1/tests/P1*. Read more about the tests, including how to run them, by executing the command `cat ~cs537-1/tests/P1/README` on any lab machine. Note these test cases are not complete, and you are encouraged to create more on your own.
- **Handing it in**: Copy your files to *~cs537-1/handin/login/P1* where login is your CS login.

## Unix Utilities

In this project, you'll building a simple user level program, much like the various UNIX command line utilities (e.g. man, zip, grep, ...). You'll be implementing a tool that will report some various statistics about the text content of a file (e.g number of unique characters, frequency of characters, frequency of words, etc.) We will call this program MADCounter.c (i.e. Madison Counter)

**Objectives:**

* Re-familiarize yourself with the C programming language, especially:
  * Working with strings
  * Reading and Writing files
  * Working with structs
  * Simple dynamic memory allocation
* Re-familiarize yourself with the shell / terminal / command-line of UNIX

While the project focuses upon writing simple C programs, you can see from the above that even that requires a bunch of other previous knowledge, including a basic idea of what a shell is and how to use the command line on some UNIX-based systems (e.g., Linux or macOS), and of course a basic understanding of C programming. If you **do not** have these skills already, this is not the right place to start.

**Summary of what gets turned in:**

* One **.c** file: **MADCounter.c**.
* Each file should compile successfully when compiled with the `-Wall` and `-Werror` flags.
* They should (hopefully) pass the tests we supply.
* Include a single **README.md** describing the implementation. This file should include your name, your cs login, you wisc ID and email, and the status of your implementation. If it all works then just say that. If there are things you know don't work let me know.
* If applicable, a **document describing online resources used** called **resources.txt**. You are welcome to use resources online you help you with your assignment. **We don't recommend you use Large-Language Models such as ChatGPT.** For this course in particular we have seen these tools give close, but not quite right examples or explanations, that leave students more confused if they don't already know what the right answer is. Be aware that when you seek help from the instructional staff, we will not assist with working with these LLMs and we will expect you to be able to walk the instructional staff member through your code and logic. Online resources (e.g. stack overflow) and generative tools are transforming many industries including computer science and education.  *However*, if you use online sources, you are required to turn in a document describing your uses of these sources. Indicate in this document what percentage of your solution was done strictly by you and what was done utilizing these tools. Be specific, indicating sources used and how you interacted with those sources. Not giving credit to outside sources is a form of plagiarism. It can be good practice to make comments of sources in your code where that source was used. You will not be penalized for using LLMs or reading posts, but you should not create posts in online forums about the projects in the course. The majority of your code should also be written from your own efforts and you should be able to explain all the code you submit.

## MADCounter

The program **MADCounter.c** is a command-line text tool which gathers statistics about characters, words, and file lines from an input file and either prints those statistics to the command line or to an output file. Your tool should also accept a batch file containing a series of statistics requests, each operating on different text files. 

### Single Run
Let's first talk about a single run of this utility, beginning with an example.

$ __welcome.txt__ contains:
```
Welcome to CS537! We are looking forward to having a wonderful semester.
```
```
prompt> ./MADCounter -f welcome.txt -o output.txt -c -w -l -Lw -Ll
prompt> cat output.txt
Total Number of Chars = 73
Total Unique Chars = 29

Ascii Value: 10, Char: 
, Count: 1, Initial Position: 72
Ascii Value: 32, Char:  , Count: 11, Initial Position: 7
Ascii Value: 33, Char: !, Count: 1, Initial Position: 16
Ascii Value: 46, Char: ., Count: 1, Initial Position: 71
Ascii Value: 51, Char: 3, Count: 1, Initial Position: 14
Ascii Value: 53, Char: 5, Count: 1, Initial Position: 13
Ascii Value: 55, Char: 7, Count: 1, Initial Position: 15
Ascii Value: 67, Char: C, Count: 1, Initial Position: 11
Ascii Value: 83, Char: S, Count: 1, Initial Position: 12
Ascii Value: 87, Char: W, Count: 2, Initial Position: 0
Ascii Value: 97, Char: a, Count: 4, Initial Position: 21
Ascii Value: 99, Char: c, Count: 1, Initial Position: 3
Ascii Value: 100, Char: d, Count: 2, Initial Position: 39
Ascii Value: 101, Char: e, Count: 8, Initial Position: 1
Ascii Value: 102, Char: f, Count: 2, Initial Position: 33
Ascii Value: 103, Char: g, Count: 2, Initial Position: 31
Ascii Value: 104, Char: h, Count: 1, Initial Position: 44
Ascii Value: 105, Char: i, Count: 2, Initial Position: 29
Ascii Value: 107, Char: k, Count: 1, Initial Position: 28
Ascii Value: 108, Char: l, Count: 3, Initial Position: 2
Ascii Value: 109, Char: m, Count: 2, Initial Position: 5
Ascii Value: 110, Char: n, Count: 3, Initial Position: 30
Ascii Value: 111, Char: o, Count: 7, Initial Position: 4
Ascii Value: 114, Char: r, Count: 5, Initial Position: 22
Ascii Value: 115, Char: s, Count: 2, Initial Position: 63
Ascii Value: 116, Char: t, Count: 3, Initial Position: 8
Ascii Value: 117, Char: u, Count: 1, Initial Position: 60
Ascii Value: 118, Char: v, Count: 1, Initial Position: 46
Ascii Value: 119, Char: w, Count: 2, Initial Position: 36

Total Number of Words: 12
Total Unique Words: 11

Word: CS537!, Freq: 1, Initial Position: 2
Word: We, Freq: 1, Initial Position: 3
Word: Welcome, Freq: 1, Initial Position: 0
Word: a, Freq: 1, Initial Position: 9
Word: are, Freq: 1, Initial Position: 4
Word: forward, Freq: 1, Initial Position: 6
Word: having, Freq: 1, Initial Position: 8
Word: looking, Freq: 1, Initial Position: 5
Word: semester., Freq: 1, Initial Position: 11
Word: to, Freq: 2, Initial Position: 1
Word: wonderful, Freq: 1, Initial Position: 10

Total Number of Lines: 1
Total Unique Lines: 1

Line: Welcome to CS537! We are looking forward to having a wonderful semester., Freq: 1, Initial Position: 0

Longest Word is 9 characters long:
  semester.
  wonderful

Longest Line is 72 characters long:
  Welcome to CS537! We are looking forward to having a wonderful semester.
```

We will describe each of the flags down below, but for now the full possible usage is:

```
./MADCounter -f <input file> -o <output file> -c -w -l -Lw -Ll
```

  OR
```
./MADCounter -B <batch file>
```
See the batch mode section below for those details, but lets talk about the flags first.

* __-f__ : This flag is placed before the name of the file we are analyzing.
* __-o__ : This flag is used to specify an output file. If this isn't specified than all the output should go to stdout.

__Statistic Flags__: These will determine which kinds of analysis you should do. You will print the output in the order these flags occur. We want the output to always be Ascii alphabetically sorted order. When printing multiple statistics, print an additional newline between the statistics output.

* __-c__ : This flag means you should track each Ascii character 0-127. You should track how many times each character is used and the initial zero based position the character occurred in. You should also track the total number of chars and number of unique chars. When you print the output it should be in the following format, with a line for each character, and should only include characters you actually encountered in the document.
```
Total Number of Chars = <number of chars>
Total Unique Chars = <number unique chars>

Ascii Value: <ascii int>, Char: <character>, Count: <frequency>, Initial Position: <zero based initial position>\n
```

* __-w__ : This flag means you should track each whitespace separated string. You should track how many times each string is used and the initial zero-based position the string occurred in. This will be the string position, not the character position. You should also track the total number of words and number of unique words. When you print the output it should be in the following format for each string.
```
Total Number of Words: <number of words>
Total Unique Words: <number unique words>

Word: <string>, Freq: <freq int>, Initial Position: <position int>\n
```
* __-l__ : This flag means you should track each newline separated line. You should track how many times each line is used and the initial zero-based position the line occurred in. This will be the line number of the first occurrence of the line, not the character position. You should also track the total number of lines and number of unique lines. When you print the output it should be in the following format, with a line for each string. When you print the lines, you should remove any trailing newline characters. 
```
Total Number of Lines: <number of lines>
Total Unique Lines: <number unique lines>

Line: <line>, Freq: <freq int>, Initial Position: <postion int>\n
```
* __-Lw__ : This flag means you should keep track of the longest word(s) and how long the string(s) is. You will print the output in the following format. If there is more than one word with the same length than print them in an Ascii alphabetically sorted list.
```
Longest Word is <length> characters long:\n
\t<word 1>\n
\t<word 2>\n
...
```
* __-Ll__ : This flag means you should keep track of the longest line(s) and how many characters long the line(s) is. You will print the output in the following format. If there is more than one line with the same length than print them in an Ascii alphabetically-sorted list.
```
Longest Line is <length> characters long:\n
\t<Line 1>\n
\t<Line 2>\n
...
```

### Batch Mode
One of the modes of your utility should take a file with a list of different text analysis requests. Each request will be on a line by itself and will take the same form as a single run above minus the executable names. 

$batch.txt
```
-f welcome.txt -o output.txt -c -w -l -Lw -Ll
-f hollow.txt -o output.txt -c -w -l -Lw -Ll
-f welcome.txt -o output.txt -l -w -c
-f movies.txt -o output.txt -c
-f welcome.txt -o output.txt -Lw -Ll
```

These commands are all subject to the errors specified below. If you encounter an error in one of the commands you should print the error message and then move on to handling the next command rather than completely exiting.

### Errors
This section describes the kinds of errors we want you to handle and in what order to handle the errors in. Any errors not on this list we won't expect you to handle. Unless indicated otherwise all error cases you program should exit immediately and return code 1. On success your program should use return code 0. All error messages should be printed to STDOUT with a newline at the end. HINT: To get error message order correct, process all the arguments before doing analysis.
  
* __Usage Error__ - Print when less than 3 arguments are provided.
  * "USAGE:\n\t./MADCounter -f \<input file> -o \<output file> -c -w -l -Lw -Ll\n\t\tOR\n\t./MADCounter -B \<batch file>"
```
USAGE:
  ./MADCounter -f <input file> -o <output file> -c -w -l -Lw -Ll
    OR
  ./MADCounter -B <batch file>
```

* __Invalid Flag__ - All flags should begin with a -, and only flags should begin with this. If there is a flag that we did NOT specify elsewhere in this doc then print this error.
  * "ERROR: Invalid Flag Types"
```
ERROR: Invalid Flag Types
```
* __Can't Open Batch File__ - If the is an error in opening the batch file.
  * "ERROR: Can't open batch file"
```
ERROR: Can't open batch file
```
* __Batch File Empty__ - If there's nothing in the batch file. 
  * "ERROR: Batch File Empty"
```
ERROR: Batch File Empty
```
* __NO Input File Provided__ - If the -f flag wasn't specified or it the -f flag is immediately followed by another flag. 
  * "ERROR: No Input File Provided"
```
ERROR: No Input File Provided
```
* __Can't Open Input File__ - If the is an error in opening the input file.
  * "ERROR: Can't open input file"
```
ERROR: Can't open input file
```
* __No Output File__ - If the -o flag was specified but is immediately followed by another flag. 
  * "ERROR: No Output File Provided"
```
ERROR: No Output File Provided
```
* __Input File Empty__- If there nothing in the input file. 
  * "ERROR: Input File Empty"
```
ERROR: Input File Empty
```
~~* Ascii character above 127 present - While doing character analysis, if you run into an ascii character that isn't on of the 0-127 ascii character set than you can print the following error, but don't exit. You should complete the analysis and if it is success return 0 at the end.~~
  * "ERROR: Detecting Ascii Character \<ascii number detected> at position \<zero based character position>"
```
ERROR: Detecting Ascii Character <ascii number detected> at postion <zero based character position>
```


## P1 Hints & Links


Ascii - https://www.ascii-code.com

Helpful Functions:
* fscanf
* fgetc
* fgets
* fprintf
* printf
* malloc
* strdup
* strcmp
* strtok
* fseek
* fopen
* fclose
* feof

### Recommedations
We recommend you make a struct similar to this one below and maintain an alphabetically sorted list of words. This makes it easier when it comes time to print your results. The same struct can be used for lines as well.
```
typedef struct word{
  char * contents;
  int numChars;
  int frequency;
  int orderAppeared;
  struct word * nextWord;
  struct word * prevWord;
} WORD;
```

__Starting Steps__

1. Start by getting total counts of chars, words, and lines. This is easier to double check than individual frequency of positions and doesn't require comparing values, but rather just grabbing the correctly sized chunks from the input file. Although this can be done with a single pass of the files, I'd recommend doing multiple passes of the file using different file reading functions. Take a look at fgetc, fgets, fscanf, and getline to see their differences and see how they may help you parse the files differently. To get back to the top of the file without opening and closing it multiple times, take a look at fseek.
2. Doing the character analysis will be the next easiest task. For this all you need is a statically sized two dimensional array (frequency & initial position) and a few ints for the total count and total unique count. Grab one character at a time until you reach the end of file or EOF, tracking frequencies, etc as you go.
3. After that start building your struct for words. I recommend keeping a list of structs sorted alphabetically by word. This will make searching for the next string in the list faster and printing alphabetically easier later. Plan out when you are going to add new nodes to the list, and what values need to be changed in existing nodes when adding a node. Writing simple examples with nodes on paper is good practice to double-check your understanding. Practice your dynamic allocation (malloc) and remember to free data at the end of your program or even between requests in the batch mode.
4. For parsing the words I'd take a look at fscanf and for lines I'd take a look at fgets. There is one that one way to parse the file correctly though.
5. Parsing the arguments and the batch file can be worked on a bit independently of the other tasks, especially if you make a function for each of the stat options and for printing each of the stats. You can do this before or after the other logic depending on your work style.
  

### Using man Pages

You'll need to learn how to use a few library routines from the C standard library (often called **libc**) to implement the source code for this assignment. All C code is automatically linked with the C library, which is full of useful functions you can call to implement your program. Learn more about the C library [here](https://en.wikipedia.org/wiki/C_standard_library) and perhaps [here](https://www-s.acm.illinois.edu/webmonkeys/book/c_guide/).  

For this project, we recommend using the following routines to do file input and output: **fopen()** and **fclose()**. Whenever you use a new function like this, the first thing you should do is read about it -- how else will you learn to use it properly?

On UNIX systems, the best way to read about such functions is to use what are called the **man** pages (short for **manual**). In our HTML/web-driven world, the man pages feel a bit antiquated, but they are useful and informative and generally quite easy to use.

To access the man page for **fopen()**, for example, just type the following at your UNIX shell prompt: ``` prompt> man fopen ```

Then, read! Reading man pages effectively takes practice; why not start learning now?

We will also give a simple overview here. The **fopen()** function "opens" a file, which is a common way in UNIX systems to begin the process of file access. In this case, opening a file just gives you back a pointer to a structure of type **FILE**, which can then be passed to other routines to read, write, etc. 

Here is a typical usage of **fopen()**:

```c 
FILE *fp = fopen("main.c", "r"); 
if (fp == NULL) { 
	printf("cannot open file\n"); 
  exit(1); 
} 
```

A couple of points here. First, note that **fopen()** takes two arguments: the *name* of the file and the *mode*. The latter just indicates what we plan to do with the file. In this case, because we wish to read the file, we pass "r" as the second argument. Read the man pages to see what other options are available. 

Second, note the *critical* checking of whether the **fopen()** actually
succeeded. This is not Java where an exception will be thrown when things goes wrong; rather, it is C, and it is expected (in good programs, i.e., the only kind you'd want to write) that you always will check if the call succeeded. Reading the man page tells you the details of what is returned when an error is encountered; in this case, the macOS man page says:

``` 
Upon successful completion fopen(), fdopen(), freopen() and fmemopen()
return a FILE pointer.  Otherwise, NULL is returned and the global variable errno is set to indicate the error.  
```

Thus, as the code above does, please check that **fopen()** does not return NULL before trying to use the FILE pointer it returns.

Third, note that when the error case occurs, the program prints a message and then exits with error status of 1. In UNIX systems, it is traditional to return 0 upon success, and non-zero upon failure. Here, we will use 1 to indicate failure.

Side note: if **fopen()** does fail, there are many reasons possible as to why. You can use the functions **perror()** or **strerror()** to print out more about *why* the error occurred; learn about those on your own (using ... you guessed it ... the man pages!).

Finally, whenever you open a file remember you need to close it! You can use the `fclose()` function to close files. 

### Reading from a file
Once a file is open, there are many different ways to read from it. There are two functions that might be useful for you in the context of this assignment. 
* Reading/Writing a line: A number of UNIX utilities (e.g., grep, cat) read input files one line at time. There are two useful functions to read lines: `fgets` which can read a fixed number of characters but stops when it reaches the end of a line, and `getline` which can read entire lines into a buffer (be careful with very long lines!). You can learn more about these functions in, you guessed it, the man pages! To write a string to a file you can similarly use `fputs` or the more powerful formatting capabilities in `fprintf`.

* Reading/Writing bytes: Another common way to read files is to read them a few bytes at a time. The relevant functions to lookup for this are `fread`  and `fwrite`.

### Strings in C
Here is a quick reminder that there are no strings as you might be used to in Java, python, c++, etc. To store or use strings in C you have to you char arrays or char pointers (char*). For this project you'll need to use the __malloc__ and __free__ to dynamically allocate and unallocate arrays or data structs. To operate on these strings there are useful functions in the __string.h__ library. The below resources should give you a pretty good starting overview.
* String Library - https://www.geeksforgeeks.org/strings-in-c/, https://www.tutorialspoint.com/c_standard_library/string_h.htm
* Dynamic Memory Allocation - https://www.geeksforgeeks.org/dynamic-memory-allocation-in-c-using-malloc-calloc-free-and-realloc/?ref=leftbar-rightbar
