# 10 - blackjack

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
blackjack@ubuntu:~$ ls -la
total 64
drwxr-x---   2 root blackjack  4096 Apr  1 14:40 .
drwxr-xr-x 127 root root       4096 May  1 15:07 ..
-r-xr-x---   1 root root      22153 Apr  1 14:40 blackjack
-rw-r--r--   1 root root      26095 Apr  1 14:40 blackjack.c
-rw-r--r--   1 root root        106 Apr  1 14:40 readme
```

We need to read the `readme` file:
```bash
blackjack@ubuntu:~$ cat readme
once you connect to port 9009, the "blackjack" binary will be executed under asm_pwn privilege. get flag.
```

Let's try to run `nc 0 9009`:
```bash
blackjack@ubuntu:~$ nc 0 9009




              222                111
            222 222            11111
           222   222          11 111
                222              111
               222               111

CCCCC     SS            DD         HHHHH    C    C
C    C    SS           D  D       H     H   C   C
C    C    SS          D    D     H          C  C
CCCCC     SS          D DD D     H          C C
C    C    SS         D DDDD D    H          CC C
C     C   SS         D      D    H          C   C
C     C   SS        D        D    H     H   C    C
CCCCCC    SSSSSSS   D        D     HHHHH    C     C

                        21
     DDDDDDDD      HH         CCCCC    S    S
        DD        H  H       C     C   S   S
        DD       H    H     C          S  S
        DD       H HH H     C          S S
        DD      H HHHH H    C          SS S
        DD      H      H    C          S   S
     D  DD     H        H    C     S   S    C
      DDD      H        H     CCCCC    S     S

         222                     111
        222                      111
       222                       111
      222222222222222      111111111111111
      2222222222222222    11111111111111111


                 Are You Ready?
                ----------------
                      (Y/N)

```

We turn to the `blackjack.c` file and its corresponding compiled binary file `blackjack` - See attached file as it is too long.

We see that to get the `flag`, the function `cash_test()` must be called with `cast > 1000000`:
```c
void cash_test() //Test for if user has cash remaining in purse
{
     if (cash <= 0) //Once user has zero remaining cash, game ends and prompts user to play again
     {
        printf("You Are Bankrupt. Game Over");
        cash = 500;
        askover();
     }
     if (cash > 1000000){
        FILE* fp=fopen("flag", "r");
        char buf[100];
        memset(buf, 0, 100);
        fread(buf, 1, 100, fp);
        printf("%s\n", buf);
        fclose(fp);
     }
} // End Function
```

For those who know the rules of blackjack, reaching million dollars from 500 dollars is quite challenging. We must find another way to earn cash.

This function is called only once in the beginning of the `play()` function:
```c
void play() //Plays game
{

     int p=0; // holds value of player_total
     int i=1; // counter for asking user to hold or stay (aka game turns)
     char choice3;

     cash = cash;
     cash_test();
     ...
```

So we need to make sure that when the `play()` function is called, `cash` global variable is already high enough to pass `cash_test`'s check.

The `randcard()` function only randomizes a card type (value between 1 and 4), and chooses to display an ascii image according to the result. Each card type is handled in its own function: `clubcard()`, `diamondcard()`, `heartcard()`, `spadecard()`. Inside each function, the card value is randomized (value between 1 and 13), and the return value of each function will be assign to global variables `k` and `l` according to the following rules:
* If the random card value is `x <= 10`: `l=k=x`
* If the random card value is `x == 11`:
    * If `player_total <= 10`: `l=k=11`
    * If `player_total > 10`: `l=k=1`
* If the random card value is `12 <= x <= 13`: `l=k=10`

After that, the `player_total` is calculated using the currently accumulate player's total `p`, and the recently generated value from the `randcard()`, `l`.

The `dealer()` function is called, which checks the `dealer_total`:
* If `dealer_total >= 17`: Nothing happens
* If `dealer_total >= 17`: A random number is generated according to the following rules (similar to those of the player)
    * If the random valus is `x <= 10`: `d=x`
    * If the random valus is `x == 11`:
        * If `dealer_total <= 10`: `d=11`
        * If `dealer_total > 10`: `d=1`
    * If the random valus is `12 <= x <= 13`: `d=10`

After the turns of the player and the dealer, it is `betting()` time. The player enters an integer bet which isn't supposed to be more than `cash`. If it is, it is asked again, but this time there is no check, so any `bet` can be entered, interesting! Finally the bet is returned:
```c
int betting() //Asks user amount to bet
{
 printf("\n\nEnter Bet: $");
 scanf("%d", &bet);

 if (bet > cash) //If player tries to bet more money than player has
 {
        printf("\nYou cannot bet more money than you have.");
        printf("\nEnter Bet: ");
        scanf("%d", &bet);
        return bet;
 }
 else return bet;
} // End Function
```

Now there's a while loop that runs at most 21 times. In each loop the `p` (player's total) is checked:
* If `p == 21`: The player wins, and the value of `bet` is added to `cash`
* If `p > 21`: The player losses, and the value of `bet` is subtracted from `cash`
* If `p < 21`: The player needs to choose whether to hit or stay.

We see that `bet` is not changed at this game until the next game. So it means we can control its value thanks to the bug in function `betting()`. 

So it seems all we need to do is just:

1. Play a game
2. Bet more than `500` in the first bet prompt
3. Bet more than `1000000` on the second bet prompt
3. Win the game

NOTE: If you lose, and `cash` is negative, 

## Solution
```bash
blackjack@ubuntu:~$ nc 0 9009




              222                111
            222 222            11111
           222   222          11 111
                222              111
               222               111

CCCCC     SS            DD         HHHHH    C    C
C    C    SS           D  D       H     H   C   C
C    C    SS          D    D     H          C  C
CCCCC     SS          D DD D     H          C C
C    C    SS         D DDDD D    H          CC C
C     C   SS         D      D    H          C   C
C     C   SS        D        D    H     H   C    C
CCCCCC    SSSSSSS   D        D     HHHHH    C     C

                        21
     DDDDDDDD      HH         CCCCC    S    S
        DD        H  H       C     C   S   S
        DD       H    H     C          S  S
        DD       H HH H     C          S S
        DD      H HHHH H    C          SS S
        DD      H      H    C          S   S
     D  DD     H        H    C     S   S    C
      DDD      H        H     CCCCC    S     S

         222                     111
        222                      111
       222                       111
      222222222222222      111111111111111
      2222222222222222    11111111111111111


                 Are You Ready?
                ----------------
                      (Y/N)
                        y

Enter 1 to Begin the Greatest Game Ever Played.
Enter 2 to See a Complete Listing of Rules.
Enter 3 to Exit Game. (Not Recommended)
Choice: 1

Cash: $500
-------
|C    |
|  4  |
|    C|
-------

Your Total is 4

The Dealer Has a Total of 9

Enter Bet: $1000000

You cannot bet more money than you have.
Enter Bet: 1000000


Would You Like to Hit or Stay?
Please Enter H to Hit or S to Stay.
h
-------
|H    |
|  Q  |
|    H|
-------

Your Total is 14

The Dealer Has a Total of 16

Would You Like to Hit or Stay?
Please Enter H to Hit or S to Stay.
h
-------
|D    |
|  9  |
|    D|
-------

Your Total is 23

The Dealer Has a Total of 24
Dealer Has Went Over!. You Win!

You have 1 Wins and 0 Losses. Awesome!

Would You Like To Play Again?
Please Enter Y for Yes or N for No
y
Woohoo_I_am_now_a_MILL10NAIRE!


Cash: $1000500
-------
|C    |
|  K  |
|    C|
-------

Your Total is 10

The Dealer Has a Total of 2

Enter Bet: $

```