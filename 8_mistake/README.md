# 8 - mistake

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
mistake@ubuntu:~$ ls -la
total 52
drwxr-x---   5 root mistake      4096 Apr  2 09:08 .
drwxr-xr-x 127 root root         4096 May  1 15:07 ..
d---------   2 root root         4096 Jul 29  2014 .bash_history
-r--r-----   1 root mistake_pwn    40 Apr  2 09:08 flag
dr-xr-xr-x   2 root root         4096 Aug 20  2014 .irssi
-r-xr-sr-x   1 root mistake_pwn 16520 Mar 28 14:49 mistake
-rw-r--r--   1 root root          826 Mar 28 14:49 mistake.c
-r--r-----   1 root mistake_pwn    10 Jul 29  2014 password
drwxr-xr-x   2 root root         4096 Oct 23  2016 .pwntools-cache
```

See first exercise for reason we can't just read `flag` directly.

We then turn to the `mistake.c` file and its corresponding compiled binary file `mistake`:
```c
#include <stdio.h>
#include <fcntl.h>

#define PW_LEN 10
#define XORKEY 1

void xor(char* s, int len){
        int i;
        for(i=0; i<len; i++){
                s[i] ^= XORKEY;
        }
}

int main(int argc, char* argv[]){

        int fd;
        if(fd=open("/home/mistake/password",O_RDONLY,0400) < 0){
                printf("can't open password %d\n", fd);
                return 0;
        }

        printf("do not bruteforce...\n");
        sleep(time(0)%20);

        char pw_buf[PW_LEN+1];
        int len;
        if(!(len=read(fd,pw_buf,PW_LEN) > 0)){
                printf("read error\n");
                close(fd);
                return 0;
        }

        char pw_buf2[PW_LEN+1];
        printf("input password : ");
        scanf("%10s", pw_buf2);

        // xor your input
        xor(pw_buf2, 10);

        if(!strncmp(pw_buf, pw_buf2, PW_LEN)){
                printf("Password OK\n");
                setregid(getegid(), getegid());
                system("/bin/cat flag\n");
        }
        else{
                printf("Wrong Password\n");
        }

        close(fd);
        return 0;
}
```

At first glance, it seems that the file `/home/mistake/password` is read into a buffer named `pw_buf`. Like the `flag` file, we can't read the `password` file, as we don't have the permissions. Because the binary has an effective group id, `SGID`, it is able to read the `flag` file using it's own group permissions we (uid/gid `1017`) don't have.

We are requested to enter an input password using `scanf(3)`, which reads 10 characters at most and puts them in a buffer named `pw_buf2`. 

Then our input is XORed, byte by byte, with 1.

Lastly, the read `password` (from buffer `pw_buf`) is compared to the XORed input (from buffer `pw_buf2`). The compare is done by the `strncmp(3)` function, which compares at most `PW_LEN=10` elements from both buffers.

### Trial and error
A naive approach is to read the value of the second argument sent to `strncmp` using `ltrace`, but:

```bash
mistake@ubuntu:~$ ltrace ./mistake
open("/home/mistake/password", 0, 0400)                                                                                 = -1
printf("can't open password %d\n", 1can't open password 1
)                                                                                   = 22
+++ exited (status 0) +++
```

The reason it doesn't work, is that `ltrace` internally uses `ptrace`, which is restricted to use the `SGID` or `SUID` of the binary, in order to prevent vulnerabilities of reading a `SUID` binary. For the same reason, neither `gdb` nor `ptrace` would be able to read the file.

Another weak lead is a buffer overflow. Only `PW_LEN=10` bytes are read from the `password` file, and only `10` bytes at most are read from the input. It means that we can't use a buffer overflow to control these value

Another lost cause is brute forcing the input until we find the right value. There is a `sleep(3)` which is intended to delay us from benefiting from brute forcing the input.

The last trial is making `strncmp(3)` pass before the input password we pass will be null terminated. We can make the first character be "1" which, XORed with "1", would be 0. That way, `pw_buf2` would be "empty". The problem is that `strncmp(3)` would return 0 only if the other buffer, `pw_buf` is empty. Otherwise, it would report it being "smaller" (return value from `strncmp(3)` is positive) than `pw_buf`.

### The bug
We turn to the `password` file to see what can we do with it. Luckily for us, there is a bug in the [order of operators precedence](https://en.cppreference.com/w/c/language/operator_precedence):
```c
        int fd;
        if(fd=open("/home/mistake/password",O_RDONLY,0400) < 0){
                printf("can't open password %d\n", fd);
                return 0;
        }
```

According to the C reference, the order of evaluation of the expression in the `if` statements is:
1. Function call
2. The comparison operator "<"
3. The assignment operator "="

It means that the result of the call to `open` is an fd which is probably 3 (the lowest available one). Then, the fd is compared to 0, and since it is not less than 0, the result of the "<" operator is 0. Lastly, `fd` is assigned with the value 0. This is amazing! It means that `fd` is not read from a file like we thought, but from the standard input!

So it seems all we need to do is just:

1. Pass as the first input (`read`) 10 characters
2. Pass as the second input (`scanf`) 10 characters, such that when XORed byte-by-byte with 1, they would be equal to the first input

## Solution
We can pass countless options, but we will go with the most standard one:

```bash
mistake@ubuntu:~$ echo -e "0000000000\n1111111111" | ./mistake
do not bruteforce...
input password : Password OK
Mommy_the_0perator_priority_confuses_me
```