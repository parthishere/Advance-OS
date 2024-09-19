# Inter-Process Communication (IPC) System

## Overview

This project implements an Inter-Process Communication (IPC) system using shared memory, semaphores, and pipes in C. The system consists of a parent process and two child processes that communicate with each other through shared memory. The project demonstrates synchronization techniques and effective use of various IPC mechanisms in a UNIX-like environment.

## Components

The system consists of two main programs:

1. `parent_pgm.c`: The parent process that sets up the IPC resources and manages communication.
2. `child_pgm.c`: The child process program that is executed by both child processes.

## Features

- Shared memory communication between processes
- Semaphore-based synchronization
- Pipe-based initial setup communication
- Signal handling for graceful termination
- File I/O for logging communication


## Compilation

To compile the programs, use the following commands:

```bash
make
```

## Usage

1. First, run the parent process:

   ```bash
   ./parent_pgm
   ```

2. The parent process will automatically spawn two child processes.

3. Follow the prompts in each terminal window to input strings.

4. The communication will cycle through Parent -> Child One -> Child Two -> Parent.

5. To terminate the program, enter "TERMINATE" in the parent process.

## Program Flow

1. The parent process sets up shared memory, semaphores, and pipes.
2. It forks two child processes and executes `child_pgm` for each.
3. The parent sends initial setup information (shared memory ID, file descriptors) to children via pipes.
4. The processes then communicate in a round-robin fashion using shared memory and semaphores.
5. Each process reads from shared memory, writes to a file, then writes its own message to shared memory.
6. The cycle continues until the parent process receives a "TERMINATE" command.

## Debugging

Set the `INFO` macro to 1 in `child_pgm.c` to enable debug output.


