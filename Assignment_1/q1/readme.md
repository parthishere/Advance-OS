# System Monitoring Tool

## Overview

This System Monitoring Tool is a C program designed to provide comprehensive information about a Linux system's hardware and performance. It offers both static system information and dynamic performance monitoring capabilities.

## Features

- Static System Information:
  - Processor details (manufacturer, speed, cache size, number of CPUs)
  - Kernel information (version, build info, distribution, build date)
  - Memory information (total, free, available)
  - Physical memory layout

- Dynamic Performance Monitoring:
  - CPU usage (user, system, idle percentages)
  - Memory usage
  - Disk I/O (read and write rates)
  - Context switches per second
  - Process creation rate

## Requirements

- Linux operating system
- GCC compiler
- Standard C libraries

## Compilation

To compile the program, use the following command:

```
make
```

## Usage

1. Static Information Mode:
   ```
   ./proc_parse
   ```
   This will display static system information.

2. Dynamic Monitoring Mode:
   ```
   ./proc_parse [read_interval] [print_interval]
   ```
   - `read_interval`: Time in seconds between data readings (default: 1 second)
   - `print_interval`: Time in seconds between printing averages (default: 10 seconds)

   Example:
   ```
   ./proc_parse 2 20
   ```
   This will read data every 2 seconds and print averages every 20 seconds.

## Output

- Static Information Mode: Displays processor details, kernel information, memory info, and physical memory layout.
- Dynamic Monitoring Mode: Continuously prints average CPU usage, memory usage, disk I/O rates, context switch rate, and process creation rate.

## Limitations

- This tool is designed for Linux systems and relies on the `/proc` filesystem.
- It may require root privileges to access some system information.
- The tool does not include extensive error handling for invalid inputs or system file access issues.


