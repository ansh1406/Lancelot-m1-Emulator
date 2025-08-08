
# Lancelot M1 Emulator

## Introduction
Lancelot M1 Emulator is a software implementation of the Lancelot M1 CPU, originally designed as a hardware simulation in Logisim Evolution. This emulator allows you to execute Lancelot M1 binaries directly on your computer, providing much faster execution than hardware simulation.

## Features
- Direct execution of Lancelot M1 binaries
- Emulates hardware quirks and I/O (keyboard, TTY)
- Supports both file-based and standard input/output

## Project Context
- **Original Project:** Lancelot M1 CPU hardware simulation (Logisim Evolution)
- **This Project:** Software emulator for Lancelot M1
- **Purpose:** Run Lancelot M1 binaries natively for speed and convenience

---
For more details, see the source code and example binaries in the repository.

## Usage

### Build
Compile the emulator using a C++ compiler (e.g., g++):

```
g++ -g main.cpp -o main
```

### Run
Run the emulator with a Lancelot M1 binary:

```
main <program_file> [--stdin] [--stdout]
```

#### Options
- `--stdin`   : Use standard input for keyboard input (instead of keyboard.txt)
- `--stdout`  : Output TTY to standard output (instead of tty.txt)

#### Example

```
main examples/calculator.lm1 --stdout
```

This will run the calculator example and print TTY output to the console.

## Notes & Warnings

> **Warning:** The `--stdin` option may not work reliably in all environments. Input handling via standard input can sometimes break or behave unexpectedly, especially in certain terminals or when redirected. If you encounter issues, use `keyboard.txt` for input instead.

> **Warning:** Ensure all text files (such as binaries, `keyboard.txt`, and source files) use **LF (Line Feed)** line endings. Using **CRLF (Carriage Return + Line Feed)** line endings may cause the emulator to malfunction or misinterpret input/output.

> **Note:** These behaviors occur because the emulator is designed to closely replicate the actual Lancelot M1 hardware, including its quirks and limitations.
