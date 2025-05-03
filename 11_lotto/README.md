# 11 - lotto

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
lotto@ubuntu:~$ ls -la
total 48
drwxr-x---   5 root lotto      4096 Apr  2 09:05 .
drwxr-xr-x 127 root root       4096 May  1 15:07 ..
d---------   2 root root       4096 Feb 18  2015 .bash_history
-r--r-----   1 root lotto_pwn    39 Apr  2 09:05 flag
dr-xr-xr-x   2 root root       4096 Feb 18  2015 .irssi
-r-xr-sr-x   1 root lotto_pwn 16576 Apr  1 13:32 lotto
-rw-rw-r--   1 root root       1747 Apr  1 13:32 lotto.c
drwxr-xr-x   2 root root       4096 Oct 23  2016 .pwntools-cache
```

See first exercise for reason we can't just read `flag` directly.

We then turn to the `lotto.c` file and its corresponding compiled binary file `lotto`:
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

unsigned char submit[6];

void play(){

        int i;
        printf("Submit your 6 lotto bytes : ");
        fflush(stdout);

        int r;
        r = read(0, submit, 6);

        printf("Lotto Start!\n");
        //sleep(1);

        // generate lotto numbers
        int fd = open("/dev/urandom", O_RDONLY);
        if(fd==-1){
                printf("error. tell admin\n");
                exit(-1);
        }
        unsigned char lotto[6];
        if(read(fd, lotto, 6) != 6){
                printf("error2. tell admin\n");
                exit(-1);
        }
        for(i=0; i<6; i++){
                lotto[i] = (lotto[i] % 45) + 1;         // 1 ~ 45
        }
        close(fd);

        // calculate lotto score
        int match = 0, j = 0;
        for(i=0; i<6; i++){
                for(j=0; j<6; j++){
                        if(lotto[i] == submit[j]){
                                match++;
                        }
                }
        }

        // win!
        if(match == 6){
                setregid(getegid(), getegid());
                system("/bin/cat flag");
        }
        else{
                printf("bad luck...\n");
        }

}

void help(){
        printf("- nLotto Rule -\n");
        printf("nlotto is consisted with 6 random natural numbers less than 46\n");
        printf("your goal is to match lotto numbers as many as you can\n");
        printf("if you win lottery for *1st place*, you will get reward\n");
        printf("for more details, follow the link below\n");
        printf("http://www.nlotto.co.kr/counsel.do?method=playerGuide#buying_guide01\n\n");
        printf("mathematical chance to win this game is known to be 1/8145060.\n");
}

int main(int argc, char* argv[]){

        // menu
        unsigned int menu;

        while(1){

                printf("- Select Menu -\n");
                printf("1. Play Lotto\n");
                printf("2. Help\n");
                printf("3. Exit\n");

                scanf("%d", &menu);

                switch(menu){
                        case 1:
                                play();
                                break;
                        case 2:
                                help();
                                break;
                        case 3:
                                printf("bye\n");
                                return 0;
                        default:
                                printf("invalid menu\n");
                                break;
                }
        }
        return 0;
}
```

In this game we play a lotto guess game, where we need to guess correctly 6 numbers ranging from 1 to 45 correctly.

The random numbers are generated using `/dev/urandom`, which is random and can not be guessed.

Naive solutions of using a bind mount over the file, or soft linking it to a deterministic file fail, because we don't have the permissions for it (our user is not in the sudoers file).

Let's see what needs to be done in order to get the `flag`:
```c
        // calculate lotto score
        int match = 0, j = 0;
        for(i=0; i<6; i++){
                for(j=0; j<6; j++){
                        if(lotto[i] == submit[j]){
                                match++;
                        }
                }
        }

        // win!
        if(match == 6){
                setregid(getegid(), getegid());
                system("/bin/cat flag");
        }
        else{
                printf("bad luck...\n");
        }
```

We need `match` to be exactly 6. There are two loops which iterate over `lotto` and `submit`, incrementing `match` every time a matching byte was found in `lotto` and `submit`. There seems to be a problem regarding the check - we would expect the check to be as follows:
```c
        for(i=0; i<6; i++){
                if(lotto[i] == submit[i]){
                        match++;
                }
        }
```

And not using two loops. The loops are counting the number of times **each number from `lotto` appears in `submit`**! This is great, because the original lotto game expects al 6 numbers to be the same, which has a probability of $ \frac{1}{\binom{45}{6}} = \frac{1}{8,145,060} $, which is a really small chance.

Because each number is compared to all the numbers, it seems we only need to provide numbers that appear in `lotto`, **regardless of order**. We can choose any 6 numbers, but it is most convenient to provide the same number 6 times and wait for it to appear only once in the lotto generated number, that way it will be counted 6 times.

The probability of the number we provide 6 times to appear only once in the lotto numbers, causing `match` to be 6 at the end of the for-loops:
$$
\frac{\binom{44}{5}}{\binom{45}{6}}=\frac{1,086,008}{8,145,060}=0.13
$$

Explanation: We need to divide the number of possibilities of 5 numbers being chosen from the rest of the 44 numbers (without the number we provide), with the number of possibilities for the 6 lottos numbers. That way we will get the probability of our chosen number appearing only once in the lotto numbers.

So by providing the same number 6 times, we need to try approximately $\frac{1}{0.13}=7.5$ times to play the lotto game.


So it seems all we need to do is just:

1. Play the game approximately 8 times by providing the same number 6 times each time

## Solution
We chose the arbitrary character "!" as its ascii value (33) is lower than 45 and it is easy to type, as oppose to the unprintable characters (those below 32).

```bash
lotto@ubuntu:~$ ./lotto
- Select Menu -
1. Play Lotto
2. Help
3. Exit
1
Submit your 6 lotto bytes : !!!!!!
Lotto Start!
bad luck...
- Select Menu -
1. Play Lotto
2. Help
3. Exit
1
Submit your 6 lotto bytes : !!!!!!
Lotto Start!
bad luck...
- Select Menu -
1. Play Lotto
2. Help
3. Exit
1
Submit your 6 lotto bytes : !!!!!!
Lotto Start!
bad luck...
- Select Menu -
1. Play Lotto
2. Help
3. Exit
1
Submit your 6 lotto bytes : !!!!!!
Lotto Start!
bad luck...
- Select Menu -
1. Play Lotto
2. Help
3. Exit
1
Submit your 6 lotto bytes : !!!!!!
Lotto Start!
bad luck...
- Select Menu -
1. Play Lotto
2. Help
3. Exit
1
Submit your 6 lotto bytes : !!!!!!
Lotto Start!
bad luck...
- Select Menu -
1. Play Lotto
2. Help
3. Exit
1
Submit your 6 lotto bytes : !!!!!!
Lotto Start!
bad luck...
- Select Menu -
1. Play Lotto
2. Help
3. Exit
1
Submit your 6 lotto bytes : !!!!!!
Lotto Start!
Sorry_mom_1_Forgot_to_check_duplicates
- Select Menu -
1. Play Lotto
2. Help
3. Exit
```