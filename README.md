RTX
===

A basic real-time operating system implemented for the Keil MCB1700 board with an ARM Cortex-3 based microcontroller (NXP LPC1768).

Created By:
-----------
- [Karim Piyar Ali](https://github.com/KarimP)
- [Pauline Kwok](https://github.com/pkwok)
- [Monis Ahmed Khan](https://github.com/moniskhan)
- [Scott McDonald](https://github.com/ScottMcDo)


Summary of RTX Primitives and Services
--------------------------------------

### Memory Management
RTX supports a simple memory management system in which blocks of a fixed size are allocated and deallocated off of a memory queue. A calling process is blocked if no blocks are present.

### Processor Management
RTX supports primitives for invoking and transfering control to both user and system processes. A simple scheduler is used wherein processes with a higher priority are scheduled to run next. Processes may change their own priority.

#### System Processes
The following system processes are present in RTX:
- **A Keyboard command decoder**: This process reads keyboard input and displays the input on the screen. If the input matches a registered command then a process associated with the command will be called by the KCD with the given input. Processes may send messages to the KCD process to register custom commands.

- **A CRT Display process**: This process outputs any message sent to it on a display.

- **A Timer Interrupt-Process**: This process is executed everytime a hardware timer interrupt occurs (used for delayed message sending).

#### User Processes
The following user processes are present in RTX:
- **A 24 wall clock display process** which processes commands passed to it from the KCD process to start, stop and reset a wallclock.

- **A set priority process** which processes commands passed to it from the KCD process to change a process' priority.

### Inter-process communication
RTX support interprocess communication through basic message passing. There exists both blocking and non-blocking message sending along with a "delayed send" functionality to send a message after a delayed set of time.
