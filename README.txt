################################################################################
BOSTON SHELL PROJECT
WILLIAM MIYAMOTO
CS444
NOVEMBER 2024
################################################################################

bsh.c represents a miniature shell program supporting several basic built-in operations as well as indexing and execution
of Linux executables included in PATH. 

The following are supported built in commands:
exit | exits the program
env | lists environment variables
setenv | set new environment variable or overwrite existing ones, syntax of "setenv {envar} {value}".
unsetenv | delete environment variable, syntax of "unsetenv {envar}".
cd | change directory, syntax of "cd {dir}"; can handle relative paths and navigates to home directory when called with no args or ~
history | displays last X commands issued, where X is defined as HISTSIZE within bsh.c

Other basic commands such as ls, pwd, and dir can be used as well as long as their executables are included in PATH. 
The program also supports I/O redirection using symbols > and < representing standard output and standard input
respectively. Syntax takes the form of "{command} < {filename}". 

Parsing user commands without spaces and piping are currently not supported and should not be attempted in case of 
unpredictable behavior. 

################################################################################

ADDITIONAL NOTES:

I struggled a lot with memory leaks in this project. If there's a small memory leak in any part of the program it may not
cause an error until dozens of commands have been issued, or a specific command sequence. Also a lot of problems with 
segmentation faults. As far as I can tell I have fixed all of the memory leaks and indexing errors and the program 
should be able to handle any sequence of given commands in any order, though to any users of this program I would like
to know if you find a way to break it while only using supported commands. 

env uses a two dimensional array to store environment variables and their values separately, as I found this to be much
more workable compared to a one dimensional array and constant string formatting. Integers such as pdex and hdex keep track of 
the indexes of the HOME and PWD environment variables. This makes a very obtuse implementation involving modifying these indexes
on the env operations before cd will eventually call them. Technically this isn't a performance issue but does present clarity issues. 

setenv will look for an existing envar to overwrite but will add to the end of the list again using a counter integer if it 
does not find a match. It will NOT overwrite a variable if the maximum environment variable count has been reached, you would
have to use unsetenv to open space first.

unsetenv will search for a matching existing envar and shift everything above it down by one slot, overwriting it. This is
a naive implementation that would not be best used in actual practice as we may have an ordering system for which environment
variables to overwrite.

cd creates a buffer for the new working directory, then calls chdir either on the home directory stored in envar or a 
relative directory supplied by the user, after which it will call getcwd and store it in the buffer before copying its
value back into the environment variable array. 

history keeps a 500 element array outside of the main loop which it updates once per iteration by appending each arg in 
a command line input into a slot of the cmd array. Once the maximum array size is reached it will shift everything down 
by one index and update the highest index with the most recent command, creating a chronological history buffer 
which can be called with this command. 

Linux calls are handled by using a pointer to a duplicated PATH variable and then separating it into individual paths
before testing each one for executability. The implementation is somewhat inefficient and hard to understand, but ptr
holds the current PATH being tested provided by strsep(), ptr1 is then used to format that string to add the executable
to the end of it before calling access() on ptr1 to check for executability. We free ptr1 directly at the end of every loop. 
oindx is a roundabout way of solving a memory leak on pathtemp, as after calling strsep we can no longer free the memory by
calling free() on pathtemp. oindx holds the original index of pathtemp which we can call free() on instead. 
If a program is found to be executable the parent process will create a fork and then wait for its child to complete, while
the child process will look for I/O symbols and open stdin or stdout files if it finds them before executing the given
executable. 
