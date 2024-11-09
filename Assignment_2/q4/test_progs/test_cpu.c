/*********************************************************************
* Advanced OS Assignment 2
* File: test_cpu.c
*
* Purpose:
*     Test program to verify CPU usage limits enforced by cgroups.
*     Creates a CPU-intensive workload through an infinite loop to
*     validate that container resource constraints are working.
*
* Features:
*     - CPU stress testing
*     - Infinite busy loop
*     - Resource limit validation
*
* Author: Parth Thakkar
* Date: 8/11/24
*
* Copyright (c) 2024 Parth Thakkar
* All rights reserved.
*********************************************************************/

#include <stdio.h>     /* Standard I/O functions */

/**
* @function: main
*
* @purpose: Entry point that creates a CPU-intensive workload by
*          running an infinite loop. Used to test if the CPU
*          limits set in the container's cgroup are enforced.
*
* @returns: 0 (though never reached due to infinite loop)
*
* @note: This program will:
*      1. Attempt to use 100% of available CPU
*      2. Be restricted by cgroup CPU limits (e.g., 5%)
*      3. Allow monitoring of CPU throttling
*
* @usage: Run inside container and monitor:
*      - top command output
*      - cat /sys/fs/cgroup/mygroup/cpu.stat
*      - cat /sys/fs/cgroup/mygroup/cpu.max
*/
int main() {
   /* Infinite loop to consume CPU cycles */
   while(1) {
       /* Empty loop body creates maximum CPU load */
       /* Will be constrained by container's CPU limit */
   }

   /* Return statement never reached */
   return 0;
}

