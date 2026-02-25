# Task 1) Abs System Call Implementation in NachOS

## Overview
Implemented the `Abs` system call in NachOS that computes the absolute value of an integer passed from a user program running inside the NachOS MIPS simulator.

---

## Files Modified

| File | Change |
|------|--------|
| `code/userprog/syscall.h` | Added SC_Abs number and declaration |
| `code/userprog/ksyscall.h` | Added SysAbs kernel implementation |
| `code/userprog/exception.cc` | Added handler function and switch case |
| `code/test/start.S` | Added MIPS assembly stub |
| `code/test/Makefile` | Added build rules for abs |
| `code/test/abs.c` | New test program |

---

## Implementation

### 1. `code/userprog/syscall.h`
```c
#define SC_Abs 55

int Abs(int val);
```

### 2. `code/userprog/ksyscall.h`
```c
#include <stdint.h>   // added to fix INT32_MIN error

int SysAbs(int val) { return val < 0 ? -val : val; }
```

### 3. `code/userprog/exception.cc`
```cpp
void handle_SC_Abs() {
    int val = (int)kernel->machine->ReadRegister(4);
    int result = SysAbs(val);
    kernel->machine->WriteRegister(2, result);
    return move_program_counter();
}
```
In the switch-case inside `ExceptionHandler()`:
```cpp
case SC_Abs:
    return handle_SC_Abs();
```

### 4. `code/test/start.S`
```asm
	.globl Abs
	.ent	Abs
Abs:
	addiu $2,$0,SC_Abs
	syscall
	j	$31
	.end Abs
```

### 5. `code/test/Makefile`
Added `abs` to `PROGRAMS`, then:
```makefile
abs.o: abs.c
	$(CC) $(CFLAGS) -c abs.c

abs: abs.o start.o
	$(LD) $(LDFLAGS) start.o abs.o -o abs.coff
	$(COFF2NOFF) abs.coff abs
```

### 6. `code/test/abs.c`
```c
#include "syscall.h"

int main() {
    int result = Abs(-42);
    PrintNum(result);
    Halt();
}
```

---

## Setup Issues & Fixes

- **Missing MIPS compiler symlinks** — Makefile expected `mips-decstation-ultrix-*` but binaries were named `decstation-ultrix-*`. Fixed with:
```bash
cd /home/ssl56/oslab/usr/local/nachos/bin
ln -s decstation-ultrix-gcc    mips-decstation-ultrix-gcc
ln -s decstation-ultrix-ld     mips-decstation-ultrix-ld
ln -s decstation-ultrix-as     mips-decstation-ultrix-as
ln -s decstation-ultrix-ar     mips-decstation-ultrix-ar
ln -s decstation-ultrix-ranlib mips-decstation-ultrix-ranlib
```

- **Missing coff2noff binary** — Built from source:
```bash
cd nachos-project-master/coff2noff
make
mkdir -p ../code/coff2noff
cp coff2noff.x86Linux ../code/coff2noff/
```

- **`INT32_MIN` undeclared** — Added `#include <stdint.h>` to `ksyscall.h`.

---

## How to Run
```bash
# Build test program
cd nachos-project-master/code/test
make abs

# Build NachOS kernel
cd ../build.linux
make

# Run
./nachos -x ../test/abs
```

## Output
```
42Machine halting!
```
`Abs(-42)` returns `42` — system call works end-to-end.<img width="766" height="270" alt="Screenshot from 2026-02-25 16-19-48" src="https://github.com/user-attachments/assets/4b7531cb-8537-461a-9e2b-65ea97187bbe" />
