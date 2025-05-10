# 14 - memcpy

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
memcpy@ubuntu:~$ ls -la
total 36
drwxr-x---   5 root memcpy 4096 May  3 08:18 .
drwxr-xr-x 128 root root   4096 May  3 07:30 ..
d---------   2 root root   4096 Mar  4  2016 .bash_history
-rw-r--r--   1 root memcpy  646 May  3 08:18 Dockerfile
dr-xr-xr-x   2 root root   4096 Jul 13  2016 .irssi
-rw-r--r--   1 root memcpy 3151 May  3 08:09 memcpy.c
drwxr-xr-x   2 root root   4096 Oct 23  2016 .pwntools-cache
-rw-r--r--   1 root root    192 Mar 10  2016 readme
-r--r-----   1 root memcpy  742 May  3 08:18 super.pl
```

We need to read the `readme` file:
```bash
memcpy@ubuntu:~$ cat readme
the compiled binary of "memcpy.c" source code (with real flag) will be executed under memcpy_pwn privilege if you connect to port 9022.
execute the binary by connecting to daemon(nc 0 9022).
```

Let's try to run `nc 0 9022`:
```bash
memcpy@ubuntu:~$ nc 0 9022
Hey, I have a boring assignment for CS class.. :(
The assignment is simple.
-----------------------------------------------------
- What is the best implementation of memcpy?        -
- 1. implement your own slow/fast version of memcpy -
- 2. compare them with various size of data         -
- 3. conclude your experiment and submit report     -
-----------------------------------------------------
This time, just help me out with my experiment and get flag
No fancy hacking, I promise :D
specify the memcpy amount between 8 ~ 16 :
```


We then turn to the `memcpy.c` file:
```c
// gcc -o memcpy memcpy.c -m32 -lm
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>

unsigned long long rdtsc(){
        asm("rdtsc");
}

char* slow_memcpy(char* dest, const char* src, size_t len){
        int i;
        for (i=0; i<len; i++) {
                dest[i] = src[i];
        }
        return dest;
}

char* fast_memcpy(char* dest, const char* src, size_t len){
        size_t i;
        // 64-byte block fast copy
        if(len >= 64){
                i = len / 64;
                len &= (64-1);
                while(i-- > 0){
                        __asm__ __volatile__ (
                        "movdqa (%0), %%xmm0\n"
                        "movdqa 16(%0), %%xmm1\n"
                        "movdqa 32(%0), %%xmm2\n"
                        "movdqa 48(%0), %%xmm3\n"
                        "movntps %%xmm0, (%1)\n"
                        "movntps %%xmm1, 16(%1)\n"
                        "movntps %%xmm2, 32(%1)\n"
                        "movntps %%xmm3, 48(%1)\n"
                        ::"r"(src),"r"(dest):"memory");
                        dest += 64;
                        src += 64;
                }
        }

        // byte-to-byte slow copy
        if(len) slow_memcpy(dest, src, len);
        return dest;
}

int main(void){

        setvbuf(stdout, 0, _IONBF, 0);
        setvbuf(stdin, 0, _IOLBF, 0);

        printf("Hey, I have a boring assignment for CS class.. :(\n");
        printf("The assignment is simple.\n");

        printf("-----------------------------------------------------\n");
        printf("- What is the best implementation of memcpy?        -\n");
        printf("- 1. implement your own slow/fast version of memcpy -\n");
        printf("- 2. compare them with various size of data         -\n");
        printf("- 3. conclude your experiment and submit report     -\n");
        printf("-----------------------------------------------------\n");

        printf("This time, just help me out with my experiment and get flag\n");
        printf("No fancy hacking, I promise :D\n");

        unsigned long long t1, t2;
        int e;
        char* src;
        char* dest;
        unsigned int low, high;
        unsigned int size;
        // allocate memory
        char* cache1 = mmap(0, 0x4000, 7, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        char* cache2 = mmap(0, 0x4000, 7, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        src = mmap(0, 0x2000, 7, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

        size_t sizes[10];
        int i=0;

        // setup experiment parameters
        for(e=4; e<14; e++){    // 2^13 = 8K
                low = pow(2,e-1);
                high = pow(2,e);
                printf("specify the memcpy amount between %d ~ %d : ", low, high);
                scanf("%d", &size);
                if( size < low || size > high ){
                        printf("don't mess with the experiment.\n");
                        exit(0);
                }
                sizes[i++] = size;
        }

        sleep(1);
        printf("ok, lets run the experiment with your configuration\n");
        sleep(1);

        // run experiment
        for(i=0; i<10; i++){
                size = sizes[i];
                printf("experiment %d : memcpy with buffer size %d\n", i+1, size);
                dest = malloc( size );

                memcpy(cache1, cache2, 0x4000);         // to eliminate cache effect
                t1 = rdtsc();
                slow_memcpy(dest, src, size);           // byte-to-byte memcpy
                t2 = rdtsc();
                printf("ellapsed CPU cycles for slow_memcpy : %llu\n", t2-t1);

                memcpy(cache1, cache2, 0x4000);         // to eliminate cache effect
                t1 = rdtsc();
                fast_memcpy(dest, src, size);           // block-to-block memcpy
                t2 = rdtsc();
                printf("ellapsed CPU cycles for fast_memcpy : %llu\n", t2-t1);
                printf("\n");
        }

        printf("thanks for helping my experiment!\n");
        printf("flag : [erased here. get it from server]\n");
        return 0;
}
```


Running the experiment with the minimum number result ends the program after experiment number 5 for some reason:
```bash
memcpy@ubuntu:~$ nc 0 9022
Hey, I have a boring assignment for CS class.. :(
The assignment is simple.
-----------------------------------------------------
- What is the best implementation of memcpy?        -
- 1. implement your own slow/fast version of memcpy -
- 2. compare them with various size of data         -
- 3. conclude your experiment and submit report     -
-----------------------------------------------------
This time, just help me out with my experiment and get flag
No fancy hacking, I promise :D
specify the memcpy amount between 8 ~ 16 : 8
specify the memcpy amount between 16 ~ 32 : 16
specify the memcpy amount between 32 ~ 64 : 32
specify the memcpy amount between 64 ~ 128 : 64
specify the memcpy amount between 128 ~ 256 : 128
specify the memcpy amount between 256 ~ 512 : 256
specify the memcpy amount between 512 ~ 1024 : 512
specify the memcpy amount between 1024 ~ 2048 : 1024
specify the memcpy amount between 2048 ~ 4096 : 2048
specify the memcpy amount between 4096 ~ 8192 : 4096
ok, lets run the experiment with your configuration
experiment 1 : memcpy with buffer size 8
ellapsed CPU cycles for slow_memcpy : 5330
ellapsed CPU cycles for fast_memcpy : 680

experiment 2 : memcpy with buffer size 16
ellapsed CPU cycles for slow_memcpy : 978
ellapsed CPU cycles for fast_memcpy : 1096

experiment 3 : memcpy with buffer size 32
ellapsed CPU cycles for slow_memcpy : 1522
ellapsed CPU cycles for fast_memcpy : 1736

experiment 4 : memcpy with buffer size 64
ellapsed CPU cycles for slow_memcpy : 2698
ellapsed CPU cycles for fast_memcpy : 410

experiment 5 : memcpy with buffer size 128
ellapsed CPU cycles for slow_memcpy : 5368

memcpy@ubuntu:~$
```

After a few more tries where it fails in the 5th experiment, we can be sure the problem is not statistical. The important thing about the 5th experiment, is that it comes after the 4th, where the fast copy in `fast_memcpy` actually runs for the first time. Then, is fails the second time it runs.

According to the documentation of the [MOVDQA](https://mudongliang.github.io/x86/html/file_module_x86_id_183.html) and [MOVNTPS](https://www.felixcloutier.com/x86/movntps) commands, the operands must be aligned to 16 byte, meaning, the address should be a multiplication of 16 (both the addresses that are read from and written to). Otherwise, an exception is generated (general-protection exception).

Even though the `cache1`, `cache2`, `src` are aligned to 16 (according to `mmap(2)` they are even page aligned, which is more strong), `dest` is an address which is the result of `malloc`, which isn't necessarily 16-byte aligned. The reason for that, according to `malloc(3)`, is that the result address of the call is aligned to any builtin type. Because the binary is compiled to 32 bit, it means it is only aligned to 8 byte, whereas in 64 bit it would have been aligned to 16 byte. Running the binary on a 64 bit machine verifies it - the experiment works.

So we need to make sure that running the 32 bit binary on the 64 bit machine will make `malloc` return a 16 byte aligned address. While we can't control the way `malloc` allocated memory, we can try to strongly estimate how it will behave: Allocating sequentially will give us sequential allocations in memory. Allocating using `malloc` allocated us the amount of memory we requested, but usually, depending on the algorithm, it allocates an internal metadata struct in order to manage the allocation algorithm of `malloc`. A small check program verifies what we thought, but also gives us the metadata size of each `malloc` block - 16 bytes.

So to conclude it means each `malloc` returns an 8 byte aligned address, with another 16 bytes of metadata. We need to make sure that the result will always be 16 byte aligned, so we will just add another 8 bytes to each allocation! That way, we will shift the alignment to be 16 byte aligned as well as the guaranteed 7 byte alignment.


So it seems all we need to do is just:

1. Pass for experiments 4 to 10 sizes that satisfy: (size - 8) % 16 == 0

## Solution
We will just pass for experiments 4 to 10 the powers of 2 plus 8:

```bash
memcpy@ubuntu:/tmp$ echo -e "8\n16\n32\n72\n136\n264\n520\n1032\n2056\n4104" | nc 0 9022
Hey, I have a boring assignment for CS class.. :(
The assignment is simple.
-----------------------------------------------------
- What is the best implementation of memcpy?        -
- 1. implement your own slow/fast version of memcpy -
- 2. compare them with various size of data         -
- 3. conclude your experiment and submit report     -
-----------------------------------------------------
This time, just help me out with my experiment and get flag
No fancy hacking, I promise :D
specify the memcpy amount between 8 ~ 16 : specify the memcpy amount between 16 ~ 32 : specify the memcpy amount between 32 ~ 64 : specify the memcpy amount between 64 ~ 128 : specify the memcpy amount between 128 ~ 256 : specify the memcpy amount between 256 ~ 512 : specify the memcpy amount between 512 ~ 1024 : specify the memcpy amount between 1024 ~ 2048 : specify the memcpy amount between 2048 ~ 4096 : specify the memcpy amount between 4096 ~ 8192 : ok, lets run the experiment with your configuration
experiment 1 : memcpy with buffer size 8
ellapsed CPU cycles for slow_memcpy : 5522
ellapsed CPU cycles for fast_memcpy : 800

experiment 2 : memcpy with buffer size 16
ellapsed CPU cycles for slow_memcpy : 906
ellapsed CPU cycles for fast_memcpy : 1106

experiment 3 : memcpy with buffer size 32
ellapsed CPU cycles for slow_memcpy : 1576
ellapsed CPU cycles for fast_memcpy : 1684

experiment 4 : memcpy with buffer size 72
ellapsed CPU cycles for slow_memcpy : 3046
ellapsed CPU cycles for fast_memcpy : 750

experiment 5 : memcpy with buffer size 136
ellapsed CPU cycles for slow_memcpy : 5692
ellapsed CPU cycles for fast_memcpy : 344

experiment 6 : memcpy with buffer size 264
ellapsed CPU cycles for slow_memcpy : 10528
ellapsed CPU cycles for fast_memcpy : 468

experiment 7 : memcpy with buffer size 520
ellapsed CPU cycles for slow_memcpy : 20328
ellapsed CPU cycles for fast_memcpy : 624

experiment 8 : memcpy with buffer size 1032
ellapsed CPU cycles for slow_memcpy : 39936
ellapsed CPU cycles for fast_memcpy : 1076

experiment 9 : memcpy with buffer size 2056
ellapsed CPU cycles for slow_memcpy : 64300
ellapsed CPU cycles for fast_memcpy : 1314

experiment 10 : memcpy with buffer size 4104
ellapsed CPU cycles for slow_memcpy : 119420
ellapsed CPU cycles for fast_memcpy : 2244

thanks for helping my experiment!
flag : b0thers0m3_m3m0ry_4lignment
```