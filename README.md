# C0 Virtual Machine (C0VM)

This is my implementation of the "C0 Virtual Machine" assignment from Carnegie Mellon University (15-122).

## Note on Compilation

This repository contains the core implementation (`c0vm.c`) of the execution engine.

The driver code (`main.c`), the bytecode parser (`c0vm_main.c`), **all header files (`.h`)**, library definitions, and build artifacts (`.o` object files) are proprietary course materials from Carnegie Mellon University and **have been excluded** from this repository to respect copyright and academic integrity.

### Files included:
* `c0vm.c`: The complete bytecode interpreter, including the Fetch-Decode-Execute cycle, stack management, and Native Function Interface (my implementation).
* `c0vm`: Compiled binary for demonstration (Linux only).
* `tests/`: Contains the test cases. I've included both the **bytecode** (`.bc0`) and the **original source** (`.c0`) so you can verify exactly what the code is supposed to be doing.

## Most Challenging / Interesting Part

The arithmetic instructions were trivial. The real challenge was **Task 4: Function Calls (`invokestatic`)**.

I had to manually implement the **Call Stack** logic that we usually take for granted:

* **The Context Switch:** Implementing `invokestatic` meant I had to "freeze" the current function's world—snapshotting the Program Counter (`pc`), local variables (`V`), and operand stack (`S`) into a struct—before jumping to the new function.
* **The Return:** The `return` instruction was even trickier. I had to pop the top frame, restore the caller's state exactly as it was, and physically move the return value from the "dead" function's stack to the "live" caller's stack.

A single off-by-one error here usually caused the whole VM to segfault, so getting the memory management right for these frames was the most satisfying part of the project.

## Quick Demo

I have included the compiled binary and few "real-world" bytecode files (N-Queens solver, Fibonacci) so you can verify the execution logic immediately (Linux only).

### Example
This stresses the VM's backtracking recursion and array manipulation logic.
```bash
# Run the bytecode
./c0vm src/real_nqueens.bc0
```

```bash
# (Optional) Read the source to see the logic
cat src/real_nqueens.c0
