# Process Scheduling

This project implements Non-preemptive (Shortest Job First) and preemptive (Round Robin) process scheduling.

## Overview

The project first implements SJF and RR scheduling algorithms on simulated processes and then applies them to real processes created by forking. It also includes two memory management strategies:

- **Infinite Memory Strategy**: Processes are allocated memory without considering memory constraints.
- **Best-Fit Memory Strategy**: Allocates memory to processes by finding the smallest available partition that is large enough to fit the process. This strategy aims to minimize wasted memory by selecting the best-fitting available partition.

## How to Run

To run the program, execute the following command in the terminal:

```sh
allocate -f <filename> -s (SJF | RR) -m (infinite | best-fit) -q (1 | 2 | 3)
```
where:
filename is the valid input file describing the processes
-s is either SJF or RR
-m is the memory strategy
-q is the quantum
