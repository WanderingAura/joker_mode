# Building on Windows
```
build.bat <debug|release>
```
# Building on Linux
```
build.sh <debug|release>
```
# Hot reloading
As long as the data structures have not changed, hot reloading should work by executing the build commands while the
program is running. Hot reloading does not work when there are changes to data structures or there are changes to
main.c.

# Coding style reminder

When a function has some output and an error, put the output in the parameter list and return the error code.