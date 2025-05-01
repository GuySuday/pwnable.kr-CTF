# 7 - leg

## Walkthrough
We first run `ls -la` to see which files exist in our machine:
```bash
ls -la
total 628
drwxr-xr-x   11 root     0                0 Apr  5  2025 .
drwxr-xr-x   11 root     0                0 Apr  5  2025 ..
drwxrwxr-x    2 root     0                0 Apr  5  2025 bin
drwxrwxr-x    2 root     0                0 Apr  5  2025 boot
drwxrwxr-x    2 root     0                0 Nov 10  2014 dev
drwxrwxr-x    3 root     0                0 Apr  5  2025 etc
-r--------    1 root     0               28 Apr  5  2025 flag
---s--x---    1 root     1000        636419 Nov 10  2014 leg
lrwxrwxrwx    1 root     0               11 Apr  5  2025 linuxrc -> bin/busybox
dr-xr-xr-x   33 root     0                0 Jan  1 00:00 proc
drwx------    2 root     0                0 Apr  5  2025 root
drwxrwxr-x    2 root     0                0 Apr  5  2025 sbin
drwxrwxr-x    2 root     0                0 Nov 10  2014 sys
drwxrwxr-x    4 root     0                0 Apr  5  2025 usr
/ $

```

See first exercise for reason we can't just read `flag` directly.

We then turn to the `leg.c` file and its corresponding compiled binary file `leg`:
```c
#include <stdio.h>
#include <fcntl.h>
int key1(){
	asm("mov r3, pc\n");
}
int key2(){
	asm(
	"push	{r6}\n"
	"add	r6, pc, $1\n"
	"bx	r6\n"
	".code   16\n"
	"mov	r3, pc\n"
	"add	r3, $0x4\n"
	"push	{r3}\n"
	"pop	{pc}\n"
	".code	32\n"
	"pop	{r6}\n"
	);
}
int key3(){
	asm("mov r3, lr\n");
}
int main(){
	int key=0;
	printf("Daddy has very strong arm! : ");
	scanf("%d", &key);
	if( (key1()+key2()+key3()) == key ){
		printf("Congratz!\n");
		int fd = open("flag", O_RDONLY);
		char buf[100];
		int r = read(fd, buf, 100);
		write(0, buf, r);
	}
	else{
		printf("I have strong leg :P\n");
	}
	return 0;
}
```

Unfortunetely, neither `gdb` nor `strace` exist in the machine:
```bash
/ $ gdb leg
sh: gdb: not found
/ $ strace leg
sh: strace: not found
/ $
```

So we need to look at the given `leg.asm` file as well:
```assembly
(gdb) disass main
Dump of assembler code for function main:
   0x00008d3c <+0>:	push	{r4, r11, lr}
   0x00008d40 <+4>:	add	r11, sp, #8
   0x00008d44 <+8>:	sub	sp, sp, #12
   0x00008d48 <+12>:	mov	r3, #0
   0x00008d4c <+16>:	str	r3, [r11, #-16]
   0x00008d50 <+20>:	ldr	r0, [pc, #104]	; 0x8dc0 <main+132>
   0x00008d54 <+24>:	bl	0xfb6c <printf>
   0x00008d58 <+28>:	sub	r3, r11, #16
   0x00008d5c <+32>:	ldr	r0, [pc, #96]	; 0x8dc4 <main+136>
   0x00008d60 <+36>:	mov	r1, r3
   0x00008d64 <+40>:	bl	0xfbd8 <__isoc99_scanf>
   0x00008d68 <+44>:	bl	0x8cd4 <key1>
   0x00008d6c <+48>:	mov	r4, r0
   0x00008d70 <+52>:	bl	0x8cf0 <key2>
   0x00008d74 <+56>:	mov	r3, r0
   0x00008d78 <+60>:	add	r4, r4, r3
   0x00008d7c <+64>:	bl	0x8d20 <key3>
   0x00008d80 <+68>:	mov	r3, r0
   0x00008d84 <+72>:	add	r2, r4, r3
   0x00008d88 <+76>:	ldr	r3, [r11, #-16]
   0x00008d8c <+80>:	cmp	r2, r3
   0x00008d90 <+84>:	bne	0x8da8 <main+108>
   0x00008d94 <+88>:	ldr	r0, [pc, #44]	; 0x8dc8 <main+140>
   0x00008d98 <+92>:	bl	0x1050c <puts>
   0x00008d9c <+96>:	ldr	r0, [pc, #40]	; 0x8dcc <main+144>
   0x00008da0 <+100>:	bl	0xf89c <system>
   0x00008da4 <+104>:	b	0x8db0 <main+116>
   0x00008da8 <+108>:	ldr	r0, [pc, #32]	; 0x8dd0 <main+148>
   0x00008dac <+112>:	bl	0x1050c <puts>
   0x00008db0 <+116>:	mov	r3, #0
   0x00008db4 <+120>:	mov	r0, r3
   0x00008db8 <+124>:	sub	sp, r11, #8
   0x00008dbc <+128>:	pop	{r4, r11, pc}
   0x00008dc0 <+132>:	andeq	r10, r6, r12, lsl #9
   0x00008dc4 <+136>:	andeq	r10, r6, r12, lsr #9
   0x00008dc8 <+140>:			; <UNDEFINED> instruction: 0x0006a4b0
   0x00008dcc <+144>:			; <UNDEFINED> instruction: 0x0006a4bc
   0x00008dd0 <+148>:	andeq	r10, r6, r4, asr #9
End of assembler dump.
(gdb) disass key1
Dump of assembler code for function key1:
   0x00008cd4 <+0>:	push	{r11}		; (str r11, [sp, #-4]!)
   0x00008cd8 <+4>:	add	r11, sp, #0
   0x00008cdc <+8>:	mov	r3, pc
   0x00008ce0 <+12>:	mov	r0, r3
   0x00008ce4 <+16>:	sub	sp, r11, #0
   0x00008ce8 <+20>:	pop	{r11}		; (ldr r11, [sp], #4)
   0x00008cec <+24>:	bx	lr
End of assembler dump.
(gdb) disass key2
Dump of assembler code for function key2:
   0x00008cf0 <+0>:	push	{r11}		; (str r11, [sp, #-4]!)
   0x00008cf4 <+4>:	add	r11, sp, #0
   0x00008cf8 <+8>:	push	{r6}		; (str r6, [sp, #-4]!)
   0x00008cfc <+12>:	add	r6, pc, #1
   0x00008d00 <+16>:	bx	r6
   0x00008d04 <+20>:	mov	r3, pc
   0x00008d06 <+22>:	adds	r3, #4
   0x00008d08 <+24>:	push	{r3}
   0x00008d0a <+26>:	pop	{pc}
   0x00008d0c <+28>:	pop	{r6}		; (ldr r6, [sp], #4)
   0x00008d10 <+32>:	mov	r0, r3
   0x00008d14 <+36>:	sub	sp, r11, #0
   0x00008d18 <+40>:	pop	{r11}		; (ldr r11, [sp], #4)
   0x00008d1c <+44>:	bx	lr
End of assembler dump.
(gdb) disass key3
Dump of assembler code for function key3:
   0x00008d20 <+0>:	push	{r11}		; (str r11, [sp, #-4]!)
   0x00008d24 <+4>:	add	r11, sp, #0
   0x00008d28 <+8>:	mov	r3, lr
   0x00008d2c <+12>:	mov	r0, r3
   0x00008d30 <+16>:	sub	sp, r11, #0
   0x00008d34 <+20>:	pop	{r11}		; (ldr r11, [sp], #4)
   0x00008d38 <+24>:	bx	lr
End of assembler dump.
(gdb) 
```

In this exercise we will refer to an [Arm 32 bit cheat sheet](https://azeria-labs.com/downloads/cheatsheetv1.3-1920x1080.png).

The purpose, according to the `main` function, is to enter a key using `stdin`, (see `scanf(3)`) which will match the sum of three function return values: `key1()`, `key2()`, `key3()`. Because `scanf(3)` expects an integer, we should pass it in base 10 (decimal) and not as bytes. After the check passes, 100 bytes are read from the `flag` file, and then there are written to fd 0 (usually `stdin`). According to the assembly of `main`, we see that the functions are called one after the other.

Let's focus on each of the three function, and figure how what each of them does:

### `key1()`
Reading the assembly code of `key1` alongside `leg.c`'s `key1()` function reveals:

```assembly
   0x00008cdc <+8>:	mov	r3, pc
   0x00008ce0 <+12>:	mov	r0, r3
```

The rest of the assembly commands are its prologue and epilogue.

We can see each command is 4 bytes long, which means that this code in in 32-bit architecture of ARM, and we are in Arm state.

A `pc` (Program Counter), which is a register that holds the next instruction to execute, is assigned to register `r3`. Then, is is assigned to `r0`. As we stated before, we are in arm32 bit, and more specifically the state is arm and not thumb. So, according to the [arm's documentation](https://developer.arm.com/documentation/ka005679/latest/), in this state `pc` is read as the current instruction plus 8 (because of pipelining).

It means that when `mov r3,pc` is executed, `pc=0x8ce4`, so `r3=r0=0x8ce4` at the end of this function.


According to `main`'s assembly:
```assembly
   0x00008d68 <+44>:	bl	0x8cd4 <key1>
   0x00008d6c <+48>:	mov	r4, r0
```

Which means that after executing `key1()`: `r4=0x8ce4`

### `key2()`
Reading the assembly code of `key2` alongside `leg.c`'s `key2()` function reveals: 

```assembly
   0x00008cf8 <+8>:	push	{r6}		; (str r6, [sp, #-4]!)
   0x00008cfc <+12>:	add	r6, pc, #1
   0x00008d00 <+16>:	bx	r6
   0x00008d04 <+20>:	mov	r3, pc
   0x00008d06 <+22>:	adds	r3, #4
   0x00008d08 <+24>:	push	{r3}
   0x00008d0a <+26>:	pop	{pc}
   0x00008d0c <+28>:	pop	{r6}		; (ldr r6, [sp], #4)
   0x00008d10 <+32>:	mov	r0, r3
```

The rest of the assembly commands are its prologue and epilogue.

`r6` is backed up in order to be used (`push {r6}`), and later it is restored (`pop {r6}`). `r6` is assigned to the value of `pc` plus 1.

As we stated before, in Arm mode, pc is read as the current instruction address plus 8, which means `r6=pc+1=(0x8cfc+8)+1=0x8d05`. Then, `bx 0x8d05` is performed. This is the branch & exchange command, which, [according to its documentation](https://developer.arm.com/documentation/dui0056/d/interworking-arm-and-thumb/assembly-language-interworking/the-branch-and-exchange-instruction), branches (jumps) to the target address and executes it as Arm if the lsb is 0, or as Thumb if the lsb is 1. Since the lsb is 1 (`add 46, pc, #1` when `pc` is always even), Thumb is picked. As stated in the docs, bit 0 is always unused as the address, as it is already used as the indicator for the Arm/Thumb state. So the jump is actually to `0x8d04`. To conclude this section, its entire purpose is to switch from Arm state to Thumb state.

Now that we are in Thumb state, we execute 16-bit commands (half words/2 bytes). As stated before in the docs, when in Thumb state, the `pc` read is plus 4 from the current executed command, which makes `mov r3, pc` assign `r3=pc=0x8d08`.

Then `adds r3, #4` is executed, assigning `r3=0x8d08+4=0x8d0c`. The next `push {r3}` pushes this value to the stack, whereas the next instruction, `pop {pc}`, assigns the recently pushed item as it pops it from the stack `pc=0x8d0c`, which is the next command to run anyway.

As we mentioned before, `r6` is restored, and then `r0=r3=0x8d0c`.

According to `main`'s assembly:
```assembly
   0x00008d70 <+52>:	bl	0x8cf0 <key2>
   0x00008d74 <+56>:	mov	r3, r0
   0x00008d78 <+60>:	add	r4, r4, r3
```

Which means that after executing `key2()`: `r3=r0=0x8d0c` and `r4=0x8ce4+0x8d0c=0x119f0`.


### `key3()`
Reading the assembly code of `key3` alongside `leg.c`'s `key3()` function reveals: 

```assembly
   0x00008d28 <+8>:	mov	r3, lr
   0x00008d2c <+12>:	mov	r0, r3
```

The rest of the assembly commands are its prologue and epilogue.

`r3` is assigned to the value of `lr`, which is the Link Register, a register that holds the address that should be jumped to using `bl` or `blx`. In our case, each function is epilogued with a `bx lr`, so we know that in each function, the value in `lr` is the address of the next instruction in `main`. After we finish `key3()`, we suppose to execute the next instruction after the `0x00008d7c <+64>:	bl	0x8d20 <key3>` instruction, so it means that `lr=0x8d80`, according to `main`'s assembly:
```assembly
   0x00008d7c <+64>:	bl	0x8d20 <key3>
   0x00008d80 <+68>:	mov	r3, r0
```

So `r3=lr=0x8d80`, and then `r0=r3=0x8d80`.


According to `main`'s assembly:
```assembly
   0x00008d80 <+68>:	mov	r3, r0
   0x00008d84 <+72>:	add	r2, r4, r3
```

Which means that after executing `key3()`: `r3=r0=0x8d80` and `r2=r4+r3=0x119f0+0x8d80=0x1a770`.

### Summary
So it seems all we need to do is just:

1. Send via `stdin` the decimal value of `0x1a770`, which is `108400`

## Solution


```bash
/ $ ./leg
Daddy has very strong arm! : 108400
Congratz!
daddy_has_lot_of_ARM_muscl3
```