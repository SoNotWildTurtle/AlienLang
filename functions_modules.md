# AlienOS Functions and Modules Checklist

This document tracks planned and implemented functionality. Mark checkboxes as features are completed.

## Bootloader Extensions
- [x] Custom GRUB commands (see patches directory)
- [x] Array manipulation primitives
- [x] Arithmetic operations (e.g., `add`)
- [x] Advanced math commands (`math_commands.patch`)
- [x] Direct hardware I/O commands (see `patches/io_commands.patch`)
- [x] Network fetch capability
- [x] Variable key-value storage (see `patches/var_commands.patch`)
- [x] SHA-256 hashing command (see `patches/sha256_command.patch`)
 - [x] SHA-256 file hashing command (see `patches/sha256_file.patch`)
 - [x] Beacon networking command (see `patches/beacon_command.patch`)
- [x] Millisecond delay command (see `patches/msleep_command.patch`)
- [x] Random port command (see `patches/randport_command.patch`)
- [x] UUID generation command (see `patches/uuidgen_command.patch`)
- [x] Memory dump command (see `patches/memdump_command.patch`)
- [x] Hot patch command (see `patches/hotpatch_command.patch`)
- [x] Hot patch test script (`hotpatch_test.sh`) verifying runtime patches
- [x] RTC time command (see `patches/rtc_command.patch`)
- [x] String utility commands (`strlen`, `substr`)

## Kernel and OS Features
- [x] Minimal kernel boot via GRUB
- [x] Simple memory allocator
- [x] Preemptive scheduler
- [x] Prototype AlienLang interpreter
- [x] Prototype `alienc` expression compiler
- [x] Paging structures for task isolation
- [x] Basic filesystem drivers
- [x] Networking stack initialization
- [x] Secure networking with hashed messages
- [x] Wah AI communication interface
- [x] Wah KILL command for task termination
- [x] Beaconing routine for Wah cloud supervisor
- [x] Interrupt-driven timer and IDT setup
- [x] Per-task page tables and CR3 switching
- [x] Page fault handler stub
- [x] Wah-audited memory faults
- [x] Fault-tolerant task termination on page faults
- [x] Built-in blockchain ledger with VM opcodes
- [x] Simple hash opcode for integrity checks
- [x] Wah messaging opcode for text communication
- [x] Kernel logging facility with `logmsg` opcode
- [x] Sleep opcode for pausing tasks
- [x] Random number opcode for nondeterministic programs
- [x] Time opcode for retrieving milliseconds
- [x] Random port opcode for networking experiments
- [x] Environment mode query opcode
- [x] Variable store/load opcodes
- [x] Wah VM management commands
- [x] VMINFO command reports active microVM image
 - [x] Control flow opcodes (`OP_JMP`, `OP_JZ`)
- [x] Kernel log retrieval command
- [x] Kernel log clearing command
- [x] TIME command to query uptime
- [x] RTC command to query hardware clock time
- [x] RANDOM command to retrieve pseudo-random numbers
- [x] TASKS command lists active task IDs
- [x] USAGE command reports per-task CPU ticks
- [x] TRUSTS command reports per-task trust scores
- [x] AlienLang IDE with trust-managed microVMs (`IDE` / `IDEINFO` commands)
- [x] MEM command reports heap usage
- [x] Security hardening modules
- [x] Wah aggressive auto-connect handshake
- [x] Package management and update mechanism
- [x] Device manager for driver registration
- [x] DEVICES command lists registered device names
- [x] PKGS command lists installed packages
- [x] Configuration key-value store with Wah `CFGSET`/`CFGGET`
 - [x] PID opcode and command report current task ID
- [x] Task naming via `NAME` and `NAMES` commands
- [x] Timestamped Wah messages and audit logs

Add new entries as development progresses. The goal is to expand GRUB minimal functionality step by step until AlienOS can bootstrap a fully functional system.
