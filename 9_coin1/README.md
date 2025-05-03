# 9 - coin1

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
coin1@ubuntu:~$ ls -la
total 12
drwxr-x---   2 root coin1 4096 Apr  5 09:58 .
drwxr-xr-x 127 root root  4096 May  1 15:07 ..
-rw-r--r--   1 root root    23 Apr  5 09:58 readme
```

We need to read the `readme` file:

```bash
coin1@ubuntu:~$ cat readme
nc 0 9007 to get flag!
```

Let's try to run `nc 0 9007`:
```bash
coin1@ubuntu:~$ nc 0 9007

        ---------------------------------------------------
        -              Shall we play a game?              -
        ---------------------------------------------------

        You have given some gold coins in your hand
        however, there is one counterfeit coin among them
        counterfeit coin looks exactly same as real coin
        however, its weight is different from real one
        real coin weighs 10, counterfeit coin weighes 9
        help me to find the counterfeit coin with a scale
        if you find 100 counterfeit coins, you will get reward :)
        FYI, you have 60 seconds.

        - How to play -
        1. you get a number of coins (N) and number of chances (C)
        2. then you specify a set of index numbers of coins to be weighed
        3. you get the weight information
        4. 2~3 repeats C time, then you give the answer

        - Example -
        [Server] N=4 C=2        # find counterfeit among 4 coins with 2 trial
        [Client] 0 1            # weigh first and second coin
        [Server] 20                     # scale result : 20
        [Client] 3                      # weigh fourth coin
        [Server] 10                     # scale result : 10
        [Client] 2                      # counterfeit coin is third!
        [Server] Correct!

        - Ready? starting in 3 sec... -

N=895 C=10
```

It seems impossible to solve the game if we don't have a smart plan.

Luckily for us, it seems like a classic question in binary search, which takes `log(N)`. So we only need `C` to be more than `log(N)`. It always seems to be the case, so we are good to go.

NOTE: You can interact with the server using SSH port forwarding to fasten the development process:
```bash
ssh -p2222 -L 12345:localhost:9007 coin1@pwnable.kr
```
The problem with that is the communication is really slow, so for production it is best to run the solution inside the pwnable server rather than on your host using SSH port forwarding.


So it seems all we need to do is just:

1. Binary search the special coin and win a game
2. Repeat the previous step for 100 games

## Solution
* Create a script file `solution.py` (see attached file)
* Create a file name `/tmp/solution.py` in the pwnable server, and paste the content of our local `solution.py` to this file
* Run the script: `/tmp/solution.py`

```bash
coin1@ubuntu:~$ python /tmp/solution.py
[+] Opening connection to localhost on port 9007: Done
Congrats! get your flag

b1naRy_S34rch1Ng_1s_3asy_p3asy

[*] Closed connection to localhost port 9007
```