# 4 - random

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
random@ubuntu:~$ ls -la
total 44
drwxr-x---   5 root random      4096 Apr  5 09:49 .
drwxr-xr-x 126 root root        4096 Apr  5 10:12 ..
d---------   2 root root        4096 Jun 30  2014 .bash_history
-r--r-----   1 root random_pwn    34 Apr  5 09:45 flag
dr-xr-xr-x   2 root root        4096 Aug 20  2014 .irssi
drwxr-xr-x   2 root root        4096 Oct 23  2016 .pwntools-cache
-r-xr-sr-x   1 root random_pwn 16232 Apr  5 09:49 random
-rw-r--r--   1 root root         335 Apr  5 09:49 random.c
```

See first exercise for reason we can't just read `flag` directly.

We then turn to the `random.c` file and its corresponding compiled binary file `random`:
```c
#include <stdio.h>

int main(){
        unsigned int random;
        random = rand();        // random value!

        unsigned int key=0;
        scanf("%d", &key);

        if( (key ^ random) == 0xcafebabe ){
                printf("Good!\n");
                setregid(getegid(), getegid());
                system("/bin/cat flag");
                return 0;
        }

        printf("Wrong, maybe you should try 2^32 cases.\n");
        return 0;
}
```

We see that a random value is calculated each run, which is XORed with an input from stdin (see `scanf(3)`). The result of the XOR should be `0xcafebabe`.

At first glance it seems like the only option it to "guess" what the random value is, XOR with a corresponding key from the input in order to bypass the `if`.

Let's use `ltrace` to see what `rand(3)` returns:

```bash
random@ubuntu:~$ echo -n "1234" | ltrace ./random
rand(1, 0x7ffc78c49e38, 0x7ffc78c49e48, 0x55e53593dd90)                                                                 = 0x6b8b4567
__isoc99_scanf(0x55e53593c008, 0x7ffc78c49d00, 0, 0x7f950fc6c214)                                                       = 1
puts("Wrong, maybe you should try 2^32"...Wrong, maybe you should try 2^32 cases.
)                                                                             = 40
+++ exited (status 0) +++
random@ubuntu:~$ echo -n "5678" | ltrace ./random
rand(1, 0x7ffc37364108, 0x7ffc37364118, 0x5589d591fd90)                                                                 = 0x6b8b4567
__isoc99_scanf(0x5589d591e008, 0x7ffc37363fd0, 0, 0x7f6c584e1214)                                                       = 1
puts("Wrong, maybe you should try 2^32"...Wrong, maybe you should try 2^32 cases.
)                                                                             = 40
+++ exited (status 0) +++
random@ubuntu:~$
```

Interesting! Although we provided a different input, the result of the call to `rand(3)` is always `0x6b8b4567`! It means it is deterministic and not random, and we can calculate the right value for `key`.

So it seems all we need to do is just:

1. Pass a deterministic value of `key` to `scanf(3)`

## Solution
The right value of `key`:
```
rand_result ^ 0xcafebabe = key
0x6b8b4567 ^ 0xcafebabe = 0xa175ffd9
```

Because `scanf(3)` receives `%d` as the format, it expects an integer, so we need to convert the `0xa175ffd9` to decimal: `2,708,864,985`.


```bash
random@ubuntu:~$ echo -n "2708864985" | ./random
Good!
m0mmy_I_can_predict_rand0m_v4lue!
```