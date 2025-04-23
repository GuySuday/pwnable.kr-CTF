# 4 - random

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
input2@ubuntu:~$ ls -la
total 48
drwxr-x---   5 root input2      4096 Apr  2 09:02 .
drwxr-xr-x 126 root root        4096 Apr  5 10:12 ..
d---------   2 root root        4096 Jun 30  2014 .bash_history
-r--r-----   1 root input2_pwn    45 Apr  2 09:02 flag
-r-xr-sr-x   1 root input2_pwn 16720 Apr  1 13:27 input2
-rw-r--r--   1 root root        1787 Apr  1 13:27 input2.c
dr-xr-xr-x   2 root root        4096 Aug 20  2014 .irssi
drwxr-xr-x   2 root root        4096 Oct 23  2016 .pwntools-cache
```

See first exercise for reason we can't just read `flag` directly.

We then turn to the `input2.c` file and its corresponding compiled binary file `input2`:
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char* argv[], char* envp[]){
        printf("Welcome to pwnable.kr\n");
        printf("Let's see if you know how to give input to program\n");
        printf("Just give me correct inputs then you will get the flag :)\n");

        // argv
        if(argc != 100) return 0;
        if(strcmp(argv['A'],"\x00")) return 0;
        if(strcmp(argv['B'],"\x20\x0a\x0d")) return 0;
        printf("Stage 1 clear!\n");

        // stdio
        char buf[4];
        read(0, buf, 4);
        if(memcmp(buf, "\x00\x0a\x00\xff", 4)) return 0;
        read(2, buf, 4);
        if(memcmp(buf, "\x00\x0a\x02\xff", 4)) return 0;
        printf("Stage 2 clear!\n");

        // env
        if(strcmp("\xca\xfe\xba\xbe", getenv("\xde\xad\xbe\xef"))) return 0;
        printf("Stage 3 clear!\n");

        // file
        FILE* fp = fopen("\x0a", "r");
        if(!fp) return 0;
        if( fread(buf, 4, 1, fp)!=1 ) return 0;
        if( memcmp(buf, "\x00\x00\x00\x00", 4) ) return 0;
        fclose(fp);
        printf("Stage 4 clear!\n");

        // network
        int sd, cd;
        struct sockaddr_in saddr, caddr;
        sd = socket(AF_INET, SOCK_STREAM, 0);
        if(sd == -1){
                printf("socket error, tell admin\n");
                return 0;
        }
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = INADDR_ANY;
        saddr.sin_port = htons( atoi(argv['C']) );
        if(bind(sd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0){
                printf("bind error, use another port\n");
                return 1;
        }
        listen(sd, 1);
        int c = sizeof(struct sockaddr_in);
        cd = accept(sd, (struct sockaddr *)&caddr, (socklen_t*)&c);
        if(cd < 0){
                printf("accept error, tell admin\n");
                return 0;
        }
        if( recv(cd, buf, 4, 0) != 4 ) return 0;
        if(memcmp(buf, "\xde\xad\xbe\xef", 4)) return 0;
        printf("Stage 5 clear!\n");

        // here's your flag
        setregid(getegid(), getegid());
        system("/bin/cat flag");
        return 0;
}
```

We need to pass 5 stages in a row. Failing in one results in the main returning.

### Stage 1
We need to run the binary with `argv` (argument vector) that meets the following conditions:
1. `argv` needs to include exactly 100 `char*` (`argc` should be 100)
2. Element number 65 (decimal value of character 'A') should be: `"\x00"`
3. Element number 66 (decimal value of character 'B') should be: `"\x20\x0a\x0d"`


### Stage 2
We need the binary to read from the 2 fds the following:
1. Read `"\x00\x0a\x00\xff"` from stdin (fd 0)
2. Read `"\x00\x0a\x02\xff"` from stderr (fd 2)

The problem is we can't just fill these fds with the data, since we don't have access to them. We need some sort of an IPC (Inter Process Communication) to do that.

The solution is to communicate with the process which will run the binary, and that is possible using pipes (See `pipe(3)` and `pipe(7)`).

We will create a process, which open two pipes, one for the stdin and one for the stderr of the `input2` process. It will then fork and execute the `input2` binary. Writing to the pipes from the parent would result in the data in the child's pipes. But this is still not enough. We need the child to read the data from fd=0 and fd=2, not new fds the pipe creates. The solution is to use the `dup2(2)` system call, which will duplicate the pipes' fds to fd=0 and fd=2. That way writing data to the pipes from the parent will result in the data arriving to the child's pipe, which would read the data from fd=0 and fd=2.


### Stage 3
We need to run the binary with `envp` (environment pointer) that meets the following conditions:
1. Environment variable `"\xde\xad\xbe\xef"` should have the value `"\xca\xfe\xba\xbe"`


### Stage 4
We need to to read 4 bytes `"\x00\x00\x00\x00"` from a file named `"\x0a"`.


### Stage 5
We need to read 4 bytes `"\xde\xad\xbe\xef"` from a socket on any interface, with port `argv['C']`, which is element number 67 (decimal value of character 'C') in the `argv` array.



So it seems all we need to do is just:

1. Pass the binary `argv` such that it will pass stage 1, and help pass stage 5
2. Create pipes, execute the `input2` binary, duplicate the pipes' fds to 0 and 2, in order to read the data to pass stage 2
3. Pass the binary `envp` such that it will pass stage 3
4. Create a file with the correct name and correct data to pass stage 4
5. Connect to a socket on localhost with a port from `argv['C']` and send there data that will pass stage 5

## Solution
* Create a binary file `solution.c` (see attached file)
* Create a file name `/tmp/solution.c` in the pwnable server, and paste the content of our local `solution.c` to this file
* Compile it using `gcc`: `gcc /tmp/solution.c -o /tmp/solution`
* Run the binary: `/tmp/solution`

```bash
input2@ubuntu:/tmp$ ./solution
Welcome to pwnable.kr
Let's see if you know how to give input to program
Just give me correct inputs then you will get the flag :)
Stage 1 clear!
Stage 2 clear!
Stage 3 clear!
Stage 4 clear!
Stage 5 clear!
Mommy_now_I_know_how_to_pa5s_inputs_in_Linux
```