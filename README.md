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


# Task2) Priority Scheduler Implementation in NachOS

## Overview
Replaced the default FIFO scheduler with a priority-based scheduler.
Threads with higher priority values run first.

---

## Files Modified

| File | Change |
|------|--------|
| `code/threads/thread.h` | Added `priority` field, `getPriority()`, `setPriority()` |
| `code/threads/thread.cc` | Initialized `priority = 0` in constructor |
| `code/threads/scheduler.h` | Changed `List` to `SortedList` |
| `code/threads/scheduler.cc` | Added `PriorityCompare`, used `SortedList`, changed `Append` to `Insert` |
| `code/threads/kernel.cc` | Added `PriorityTestThread` and test in `ThreadSelfTest()` |

---

## Implementation

### 1. `code/threads/thread.h`
```cpp
// in private section
int priority;

// in public section
void setPriority(int p) { priority = p; }
int getPriority() { return priority; }
```

### 2. `code/threads/thread.cc`
```cpp
// in constructor
priority = 0;  // default priority
```

### 3. `code/threads/scheduler.h`
```cpp
// replace:
List<Thread*>* readyList;
// with:
SortedList<Thread*>* readyList;
```

### 4. `code/threads/scheduler.cc`
```cpp
// comparator - higher priority goes to front of ready list
static int PriorityCompare(Thread* a, Thread* b) {
    if (a->getPriority() > b->getPriority()) return -1;
    if (a->getPriority() < b->getPriority()) return 1;
    return 0;
}

// constructor
Scheduler::Scheduler() {
    readyList = new SortedList<Thread*>(PriorityCompare);
    toBeDestroyed = NULL;
}

// ReadyToRun - Insert instead of Append
readyList->Insert(thread);
```

### 5. `code/threads/kernel.cc`
```cpp
// add above ThreadSelfTest()
static void PriorityTestThread(int which) {
    printf("*** thread priority %d running\n", which);
}

// add at end of ThreadSelfTest()
printf("\n--- Priority Scheduler Test ---\n");

IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);

Thread* t1 = new Thread("low-priority");
t1->setPriority(1);
t1->Fork((VoidFunctionPtr)PriorityTestThread, (void*)1);

Thread* t2 = new Thread("medium-priority");
t2->setPriority(5);
t2->Fork((VoidFunctionPtr)PriorityTestThread, (void*)5);

Thread* t3 = new Thread("high-priority");
t3->setPriority(10);
t3->Fork((VoidFunctionPtr)PriorityTestThread, (void*)10);

kernel->interrupt->SetLevel(oldLevel);
currentThread->Yield();
printf("--- Priority Scheduler Test Done ---\n");
```

---

## How It Works
- Every `Thread` has a `priority` field (default 0).
- `readyList` is a `SortedList` sorted by `PriorityCompare` — higher priority threads sit at the front.
- `FindNextToRun()` calls `RemoveFront()` which always returns the highest priority thread.
- Interrupts are disabled while forking all test threads so all 3 enter the ready list before any runs — this ensures the sorted order is respected.

---

## Build and Run
```bash
cd nachos-project-master/code/build.linux
make
./nachos -K
```

## Output
```
--- Priority Scheduler Test ---
*** thread priority 10 running
*** thread priority 5 running
*** thread priority 1 running
--- Priority Scheduler Test Done ---
```
<img width="878" height="545" alt="image" src="https://github.com/user-attachments/assets/d5c50227-bd2c-45bf-9dcf-1b5d713e6650" />
