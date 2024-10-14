# CharDev: A Linux Character Device Driver

## Overview

CharDev is a Linux Kernel Module (LKM) that implements a character device driver. Interface for processes to interact with a simulated 1KB buffer, supporting concurrent read and write operations.


## Features

- **1KB Buffer**: Simulates a fixed-size device of 1024 bytes.
- **Concurrent Access**: Supports multiple processes reading and writing simultaneously.
- **Full I/O Operations**: Implements open, read, write, seek, and release operations.
- **Persistent Data**: Data written by one process can be read by others.
- **Seek Functionality**: Allows repositioning within the buffer.

## Technical Specifications

- **Device Name**: `/dev/chardev`
- **Buffer Size**: 1024 bytes
- **Synchronization**: Spinlock-based for atomic operations
- **I/O Model**: Blocking I/O

## Installation



1. Compile the module:
   ```
   make
   ```
2. cleanup the module
    ```
    make clean
    make teardown //  will do clean unload and uninstall
    make uninstall
    ```
3. Load the module:
   ```
   sudo insmod chardev.ko
   ```
   or 
   ```
   make load
   ```

4. Create the device file (replace X with the assigned major number):
   ```
   sudo mknod /dev/chardev c X 0
   sudo chmod 666 /dev/chardev
   ```
   or 
   ```
   make install
   ```

   or 
   ```
   make setup // will do build load and install 
   ```

## Usage

Here are some examples of how to interact with the CharDev driver:

### Writing to the Device

```bash
echo "Hello, CharDev!" > /dev/chardev
```

### Reading from the Device

```bash
cat /dev/chardev
```

### Seeking and Reading

```bash
dd if=/dev/chardev bs=1 count=5 skip=7
```

## API Reference

The driver implements the following file operations:

| Operation | Function | Description |
|-----------|----------|-------------|
| Open      | `device_open`    | Opens the device for I/O operations |
| Read      | `device_read`    | Reads data from the device buffer |
| Write     | `device_write`   | Writes data to the device buffer |
| Seek      | `device_llseek`  | Changes the current position in the buffer |
| Release   | `device_release` | Closes the device |

## Performance Considerations

- The use of spinlocks ensures atomic operations but may impact performance under high concurrency.
- The 1KB buffer size limits the amount of data that can be stored at any given time.

## Testing

I have provided two test programs: `reader.c` and `writer.c`. Compile them using:

```bash
gcc -o reader reader.c
gcc -o writer writer.c
```

Run multiple instances to test concurrent access:

```bash
./reader &
./reader &
./writer &
./writer &
```


<p align="center">Made with ❤️ by Parth</p>