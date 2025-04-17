# 2 - collision

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
total 44
drwxr-x---   5 root col      4096 Apr  2 08:58 .
drwxr-xr-x 126 root root     4096 Apr  5 10:12 ..
d---------   2 root root     4096 Jun 12  2014 .bash_history
-r-xr-sr-x   1 root col_pwn 15164 Mar 26 13:13 col
-rw-r--r--   1 root root      589 Mar 26 13:13 col.c
-r--r-----   1 root col_pwn    26 Apr  2 08:58 flag
dr-xr-xr-x   2 root root     4096 Aug 20  2014 .irssi
drwxr-xr-x   2 root root     4096 Oct 23  2016 .pwntools-cache

```
See first exercise for reason we can't just read `flag` directly.

We then turn to the `col.c` file and its corresponding compiled binary file `col`:
```c
#include <stdio.h>
#include <string.h>
unsigned long hashcode = 0x21DD09EC;
unsigned long check_password(const char* p){
        int* ip = (int*)p;
        int i;
        int res=0;
        for(i=0; i<5; i++){
                res += ip[i];
        }
        return res;
}

int main(int argc, char* argv[]){
        if(argc<2){
                printf("usage : %s [passcode]\n", argv[0]);
                return 0;
        }
        if(strlen(argv[1]) != 20){
                printf("passcode length should be 20 bytes\n");
                return 0;
        }

        if(hashcode == check_password( argv[1] )){
                setregid(getegid(), getegid());
                system("/bin/cat flag");
                return 0;
        }
        else
                printf("wrong passcode.\n");
        return 0;
}

```

We can see that the program expects an argument which is treated as a `char*`, because it is passed to `strlen(3)`, and later it is passed to a function which expects a `char*`.

The argument should be 20 bytes long, and it is passed to a `check_password` function. This function calculates something using the argument it receives, which is our case is the `argv[1]` of the program. The calculation is treating the argument as an array of 5 `int` elements (math checks out, as `int` is 4 bytes long, so `4*5=20`), calculating their sum and returning it. The result is then compared to a fixed value called `hashcode` of value `0x21DD09EC`.

So it seems all we need to do is just:

1. Pass the program a 20 bytes long argument that is treated as five 4-byte integers.
2. The sum of the 5 integers should be summed to `0x21DD09EC`

### Trial and error
The value `0x21DD09EC` is indivisible by 5 without a remainder, so the naive solution of constructing the input of 5 identical 4-byte integers won't work.

Another naive idea is to provide the first integer as the whole value, and then make the rest zero also won't work - `strlen(3)` expects a string. so when it gets a null terminator (`\x00`), it would stop counting, so it mean we can't use the null terminator at all.

## Solution
We would be able to make the first 4 bytes be the whole value, but because we can't use null terminators (as mentioned above), we need to use at least `\x01` as the value, so that is what we'll do - We will make the last 16 bytes just the `\x01` byte, while we subtract 4 of each byte in the target value:

* Byte #1: 0x21 - 0x4 = 0x1d
* Byte #2: 0xdd - 0x4 = 0xd9
* Byte #3: 0x09 - 0x4 = 0x05
* Byte #4: 0xec - 0x4 = 0xe8

Note that are are on little-endian, so if we want to construct a 4-byte integer we need to turn the order of the bytes. E.g: If we want to construct `0x1dd905e8`, we should write the bytes in the reverse order: `\xe8\x05\xd9\x1d`


```bash
col@ubuntu:~$ ./col $'\xe8\x05\xd9\x1d\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01'
Two_hash_collision_Nicely
col@ubuntu:~$

```
