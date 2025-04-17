# 1 - fd

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
total 48
drwxr-x---   5 root fd      4096 Apr  1 14:50 .
drwxr-xr-x 126 root root    4096 Apr  5 10:12 ..
d---------   2 root root    4096 Jun 12  2014 .bash_history
-r-xr-sr-x   1 root fd_pwn 15148 Mar 26 13:17 fd
-rw-r--r--   1 root root     452 Mar 26 13:17 fd.c
-r--r-----   1 root fd_pwn    50 Apr  1 06:06 flag
----------   1 root root     128 Oct 26  2016 .gdb_history
dr-xr-xr-x   2 root root    4096 Dec 19  2016 .irssi
drwxr-xr-x   2 root root    4096 Oct 23  2016 .pwntools-cache
```

We obviously want to read the `flag` file, but if we try it we fail with:
```
cat: flag: Permission denied
```
By running `id` command we can see why:
```
uid=1002(fd) gid=1002(fd) groups=1002(fd)
```
We have pid of 1002 (fd) and we want to read the `flag` file which doesn't have read permissions for "other", and the file is owned by user `root` and group `fd_pwn`.

We then turn to the `fd.c` file and its corresponding compiled binary file `fd`:
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char buf[32];
int main(int argc, char* argv[], char* envp[]){
        if(argc<2){
                printf("pass argv[1] a number\n");
                return 0;
        }
        int fd = atoi( argv[1] ) - 0x1234;
        int len = 0;
        len = read(fd, buf, 32);
        if(!strcmp("LETMEWIN\n", buf)){
                printf("good job :)\n");
                setregid(getegid(), getegid());
                system("/bin/cat flag");
                exit(0);
        }
        printf("learn about Linux file IO\n");
        return 0;

}
```

We can see that the program expects an argument which is supposed to be converted to an `int`, using the `atoi` function. Then a fixed number, `0x1234` is subtracted from our argument.

The result is then treated as an fd. Then `read` syscall (technically, its `libc` wrapper) is called with the fd, reading 32 bytes from it. If we read the ascii characters `LETMEWIN\n` from it, we enter the if branch, executing `/bin/cast flag`, which will show up the content of the `flag` file.

Because the binary has an effective group id, `SGID`, it is able to read the `flag` file using it's own group permissions we (uid/gid `1002`) don't have.

So it seems all we need to do is just:

1. Pass the program an argument that if we'd subtract `0x1234` from it, we'd get a valid file descriptor. According to `atoi(2)`, the argument we pass to it is treated as base-10 (decimal).
2. Have a file descriptor we can read from the `LETMEWIN\n` characters.

## Solution
We can easily control a simple fd which can hold whichever data we would like it to hold, and it is the `stdin`, the fd of the standard input!

We can just write `LETMEWIN\n` to `stdin`, and make the argument we pass such that the `fd` in the code would be 0, the open fd of `stdin`. For more information see `stdin(3)`.

The argument we should thus pass is `0x1234 + 0` in decimal, so it is `4660`.

```bash
fd@ubuntu:~$ echo LETMEWIN | ./fd 4660
good job :)
Mama! Now_I_understand_what_file_descriptors_are!
fd@ubuntu:~$   
```
