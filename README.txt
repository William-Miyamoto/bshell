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

