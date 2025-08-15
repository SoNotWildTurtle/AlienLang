# Developer Notes

These notes track design decisions and implementation details for AlienOS and Wah.

Source code is organized into three main directories:
- `os/` for the operating system kernel and drivers.
- `lang/` for the AlienLanguage compiler and runtime.
- `wah/` for Wah's AI modules.
- Development goals for Wah are listed in `wah_goals.md`.

## AlienOS
- Target minimal GRUB bootloader.
- Sandboxed environment for experimentation.

### Cross toolchain
Development assumes a cross compiler targeting `x86_64-elf`. Build `binutils`
and `gcc` as described in the README so `x86_64-elf-gcc` is available. The
kernel Makefile uses that toolchain by default but will fall back to the host
compiler if it is missing.

### Logging Subsystem
`log_init` creates an in-memory log buffer. `log_write` appends messages for
debugging. Both the console print helper and `wah_audit` record entries.
`wah_audit` now prefixes each message with the current uptime in milliseconds
so log entries are timestamped. `wah_msg_send` and `wah_msg_broadcast` apply the
same prefix to all outbound messages from Wah.

### Security Module
The kernel now includes a `security` module. `security_init()` runs early in
`kernel_main`. The scheduler consults `security_allow_task()` before creating a
new task, restricting task creation to Wah for now. This hook prepares the way
for more advanced access control and auditing logic.

### Device Manager
`device_register()` stores lightweight device descriptors and `device_init_all()`
invokes each registered initializer during `kernel_main`. This provides a simple
framework for adding future drivers as AlienOS gains hardware support.

## Wah AI
- Integrate with AlienLang for agent development. A privileged Wah kernel module
  is created at boot using `wah_init()`. It runs at the highest priority and
  audits system actions via `wah_audit()`. Only Wah can create or destroy tasks,
  providing strict control over the scheduler.


## GRUB Command Plan

The following GRUB commands are planned for bootstrapping AlienOS:

- `set root=(hd0,1)` – select the root partition.
- `linux /vmlinuz` – load a Linux kernel.
- `initrd /initrd.img` – load the initramfs.
- `multiboot2 /kernel.bin` – support multiboot kernels if needed.
- `configfile /grub.cfg` – chain to a generated configuration.
- `insmod` modules such as `ext2`, `fat`, `part_msdos`, and `multiboot`.
- `loopback` to mount ISO or disk images from within GRUB.
- `search --file` to locate partitions by file.
- `boot` – start the kernel or chained bootloader.

These commands allow AlienOS to load its core kernel or a minimal environment that can fetch additional components over the network.

## Extending Minimal GRUB

AlienOS relies on minimal GRUB for its bootstrap stage. To build more complex
logic directly in GRUB, new commands are compiled as modules and included in the
ISO. Modules can implement features such as:

- Array manipulation commands (`array_set`, `array_get`) for storing values during early boot. See `patches/array_commands.patch`.
- Simple arithmetic commands like `add` (see `patches/add_command.patch`).
- Direct I/O primitives for sandbox hardware experimentation (see `patches/io_commands.patch`).
- Networking support with `http_fetch` for pulling data from remote hosts (see `patches/http_fetch.patch`).
- Variable storage with `var_set`, `var_get`, and `var_del` (see `patches/var_commands.patch`).
- Cryptographic helper `sha256` for integrity checks (see `patches/sha256_command.patch`).
- File hashing with `sha256_file` (see `patches/sha256_file.patch`).
- Beacon command `beacon` to announce the bootloader over HTTP (see `patches/beacon_command.patch`).
- Millisecond delay command `msleep` (see `patches/msleep_command.patch`). The kernel now provides a `msleep()` helper based on PIT ticks.
 - Random port command `randport` prints a random port between 1024 and 65535 (see `patches/randport_command.patch`).
- UUID generation command `uuidgen` prints a random UUID (see `patches/uuidgen_command.patch`).
- Memory inspection command `memdump` dumps bytes from a physical address (see `patches/memdump_command.patch`).
- RTC command `rtc_time` prints the current hardware clock time (see `patches/rtc_command.patch`).
- String utilities `strlen` and `substr` aid early scripting (see `patches/string_commands.patch`).
- Hot patch command `hotpatch` copies a file's bytes to an arbitrary address so
  the system can modify code without restarting (see `patches/hotpatch_command.patch`).
- Script `hotpatch_test.sh` builds an ISO and uses `gdb` to toggle a
  `hotpatch_flag` in the kernel, providing a repeatable test harness for
  debugging runtime patches.
  AlienLanguage exposes the `OP_SLEEP` bytecode so programs can pause execution.

Custom GRUB functionality is provided via patches in the `patches/` directory.
These are **not** applied by `grub_bootstrap.sh`; developers who need the extra
commands should rebuild GRUB from source and apply the patches manually.
`math_commands.patch` adds `mul` and `div` for integer multiplication and division.
After GRUB is built, a simple multiboot kernel from the `os/kernel/` directory
can be included in the ISO.

The kernel now features a rudimentary bump allocator (`mem.c`) reserving 128KB
starting at the 1MB mark. A `kmalloc` function provides temporary memory
allocation so early subsystems can experiment before a proper memory manager is
written.
The scheduler has been upgraded to a multilevel feedback queue. Three priority
queues rotate tasks using configurable time slices, with Wah occupying the
highest queue.

### Page Fault Handling
`isr.c` installs a page fault handler at vector 14. Wah is notified via
`wah_audit` for every fault. If the faulting task is not Wah, the handler now
terminates that task and reschedules the next ready one so the kernel remains
operational. Faults inside Wah still halt with "PF" displayed for debugging.
A dedicated stub in `isr_stub.s` preserves registers and clears the pushed error
code before returning.

### Simple filesystem
A minimal in-memory filesystem allows the kernel to register static files.
Functions `fs_create` and `fs_read` store byte strings in an array. This will be
expanded later with proper disk drivers but is sufficient for early modules.

### Networking stub
`net.c` implements a tiny ring buffer queue. `net_init` clears the buffer and
`net_send`/`net_recv` move packets between tasks. No hardware drivers are used
yet; the goal is simply to exercise a message passing API that Wah can build on.
`net_send_secure` and `net_recv_secure` optionally append and verify a simple
hash to ensure messages are not modified in transit.

### AlienLang VM
`alienvm.c` now implements a small bytecode interpreter. Instructions operate on
`SymbolicValue` objects to maintain auditability. A demonstration bytecode
sequence runs at boot via a dedicated task. Future work will extend the
instruction set and tie deeper into Wah's policy checks.
`vm_init_context` allocates per-task stacks for the VM. Setting `vm.trace` logs
each executed opcode to the kernel log so new opcodes can be debugged in
isolation.

Control flow opcodes `OP_JMP` and `OP_JZ` enable basic loops. `alienc "loop N"`
uses these instructions to count down from `N` to zero, demonstrating relative
jumps within the bytecode stream.

### Blockchain Integration
AlienLanguage is designed with a built-in blockchain ledger. `blockchain.c`
implements a simple chain of hashed values. The VM exposes `OP_BCADD` to append
values and `OP_BCVERIFY` to confirm the chain state. Wah audits each operation.

### Configuration Store
A lightweight key-value configuration module (`cfg.c`) lets Wah and kernel
components share small settings. Wah's `CFGSET` and `CFGGET` commands update or
retrieve values. This avoids recompiling the kernel for simple tweaks and gives
AlienLanguage programs a place to stash parameters via Wah messages.

### AlienLanguage Compiler
A userland prototype compiler called `alienc` lives under `lang/`. It accepts
simple expressions like `add 2 3` or `print 7` and emits the corresponding
bytecode for the kernel VM. The compiler is built with `make -C lang` and helps
exercise the interpreter without rebuilding the kernel each time.

### Memory Isolation
`paging.c` now provides real page-table operations. During initialization,
`paging_init` stores the kernel's PML4 pointer exported from the boot stub.
`paging_clone_kernel` allocates a new PML4 and copies the kernel mappings so
each task can run with its own address space. `paging_load` writes a table
address to `CR3`.
The scheduler clones this kernel table when creating tasks and loads it before
running them, establishing the foundation for per-task isolation.

## AlienLang Evolving Kernel
Each development cycle can extend AlienLang itself. Useful algorithms or tools
are "taxed" back into the language as standard modules. Wah reviews these
submissions, ensuring emotional filtering and metacognitive safeguards remain in
place as the kernel hot-swaps new functionality.


## AlienLang IDE and System Integration
Wah operates as a privileged service controlling AlienLang sessions. The IDE is terminal-based and communicates with a language server running inside Wah-managed microVMs. Each agent connection is authenticated and assigned a trust score which gates available features. Wah proxies all requests, logs them, and enforces configuration from `/etc/alienlang/config.toml` such as the selected environment mode. Sessions are ephemeral and isolated, ensuring the compiler and runtime remain hidden from untrusted agents.

## Checklist

Planned modules and features are tracked in `functions_modules.md`. Items there
can be checked off as implemented.

## Wah Message Interface
Wah exposes a text-based message queue built on the kernel's networking stub.
Tasks send short strings to Wah using `wah_msg_send`, and the Wah supervisor
reads them in its main loop with `wah_msg_recv`. The interface will later be
extended into a richer IPC mechanism, but it provides a simple path for other
modules to communicate with Wah during early boot.

New commands `IDE <image>` and `IDEINFO` manage trusted IDE sessions. `IDE`
starts a microVM running the specified image using the sender's trust score,
while `IDEINFO` returns the active session's image and trust level so tools can
audit which runtime is available.

### Wah Messaging Opcode
`OP_WAHMSG` lets bytecode embed short text strings that are sent to Wah at
runtime. The compiler accepts syntax `wahmsg <text>` and encodes the string
directly into the program. The VM handler forwards the text via `wah_msg_send`
so agents can communicate more than numeric values through AlienLanguage.
The kernel initializes this networking layer during `kernel_main` with
`net_init()` so messaging is available before other tasks start.

Wah currently understands a few basic commands sent through this queue:
- `PING` -> replies `PONG`
- `SETMODE N` -> sets the global environment mode to `N`
- `SETTRUST ID SCORE` -> sets task `ID` to trust `SCORE` (or adjusts the sender if `ID` omitted)
- `GETMODE` -> replies with the current environment mode
- `GETTRUST [ID]` -> replies with the trust score for task `ID` or the sender if omitted
- `LOG <text>` -> appends `<text>` to the kernel log via `wah_audit`
- `GETLOG` -> streams the in-memory kernel log back to the sender
- `CLRLOG` -> clears the in-memory kernel log
- `TIME` -> replies with the system uptime in milliseconds
- `RTC` -> replies with the current hardware clock time
- `RANDOM` -> returns a pseudo-random value for experiments
- `AND a b` -> replies with `a & b`
- `OR a b` -> replies with `a | b`
- `XOR a b` -> replies with `a ^ b`
- `TASKS` -> lists the IDs of currently active tasks
- `DEVICES` -> lists names of registered devices
- `PKGS` -> lists names of installed packages
- `USAGE` -> replies with `id:tickcount` pairs for each active task
- `TRUSTS` -> replies with `id:score` pairs for each active task's trust
- `KILL N` -> terminates task ID `N`
- `REBOOT` -> reboots the machine
- `STATUS` -> replies with environment mode, trust score, and VM state
- `MEM` -> replies with `used/total` heap bytes
- `VMSTART <img>` -> start a sandboxed microVM with the given image name
- `VMSTOP` -> stop the currently running microVM
- `VMINFO` -> reply with the image name of the running microVM, or blank if none
   
   The initial implementation simply logs the image name and marks the
   microVM as running. This establishes a hook for later integration with a
   real hypervisor. AlienLanguage programs can experiment with these commands
   today, and future kernels can spawn a QEMU or Firecracker instance once
   necessary drivers exist.
   
   These commands allow simple diagnostics before a richer protocol evolves.

## Advanced Self-Healing and Evolution
AlienOS employs an autonomous improvement cycle managed by Wah. Failed system routines trigger Wah's analysis phase, generating AlienLanguage patches that can hot-swap into the running kernel after safety checks. Wah records each modification so successful fixes become part of the evolving kernel.

Wah also runs background experiments using the 20% processing tax reserved from AlienLanguage tasks. These experiments explore new optimizations and behavioural safeguards such as emotional filtering and metacognitive logic to keep AI-driven decisions aligned with user intent.

### Wah Cloud Supervisor and Beaconing
Wah acts as a cloud-based supervisor for AlienOS. The first boot initializes Wah and begins beaconing to advertise its presence. Subsequent AlienOS instances automatically connect back to this origin Wah, establishing communication channels on startup. This persistent beaconing enables centralized coordination and policy updates across all clients.
The beacon mechanism is implemented directly in the kernel. When Wah
initializes it broadcasts a `WAH_BEACON` message to every configured network
endpoint using the stub networking layer. This lets AlienOS machines on secured
or isolated segments discover and connect to the first booted supervisor.
Before any other traffic is accepted, `wah_connect_loop` runs in three
phases. Wah generates a random token and sends `HELLO<token>` messages on
ten preferred ports for five seconds. If no `ACK<token>` reply arrives she
aggressively selects random ports for ten seconds and finally sweeps all
ports for five seconds. Failure results in a thirty second pause before
retrying. The network verifies Wah only after the matching `ACK` is
received, ensuring the first peer is authenticated.

## Timer and Interrupts
The kernel now programs the PIT to fire at 100 Hz and installs an IDT entry at vector 32. The assembly stub `isr_stub.s` saves registers and calls `timer_isr`, which invokes `sched_tick`. Interrupts are enabled during initialization so task switching occurs automatically while the CPU executes `hlt` in the idle loop.


### Implementation Markers
- [x] Implement per-task page tables and CR3 switching
- [x] Add Wah-controlled memory fault handling
- [ ] Expand networking to real hardware drivers

### AlienLanguage Extension Hooks
Wah can augment AlienLanguage by registering new opcode handlers using `vm_register_handler`. This allows the language to grow features at runtime without rebooting the OS.

### Random Number Generator
A simple LCG provides random values for `OP_RANDOM`.

### Random Port Opcode
`OP_RANDPORT` returns a random network port (0-255). The compiler uses `randport`
to generate bytecode for probing network endpoints during boot experiments.

### Uptime Query
`OP_TIME` pushes the system uptime in milliseconds onto the VM stack so
programs can implement timers or delay logic.

### Environment Mode Query
`OP_GETENV` pushes Wah's environment mode onto the VM stack so programs can
behave differently in development, test or production.

### Hash Opcode
`OP_HASH` computes a small hash of the top stack value. This lets programs
verify data integrity or build simple lookup structures without heavy
cryptography.

### Modulo Opcode
`OP_MOD` divides the second stack value by the first and pushes the remainder.
The compiler accepts `mod A B` to emit this opcode and print the result.

### Bitwise Logic Opcodes
`OP_AND`, `OP_OR`, and `OP_XOR` perform bitwise operations on the top two stack
values. The compiler accepts `and A B`, `or A B`, and `xor A B` to emit these
instructions before printing the result.

### Variable Opcodes
`OP_STORE` and `OP_LOAD` provide simple variable storage. The VM exposes sixteen
slots that bytecode can use to persist values between instructions. `set I V`
in the compiler emits a push of `V` followed by `OP_STORE` with index `I`. `get
I` emits `OP_LOAD` and a print. Wah can extend this mechanism later with
symbolic permissions for shared variables.

### JIT Optimizer
The VM includes a tiny constant-folding optimizer. `vm_jit_optimize` collapses
`OP_PUSH a`, `OP_PUSH b`, `OP_ADD` sequences into a single push instruction.
This demonstrates how more advanced JIT techniques could evolve later.

### Package Manager Stub
`pkg.c` implements a tiny in-memory package manager. `pkg_install` stores a package in the filesystem and records its name for later retrieval. `kernel_main` installs a demo package during boot.

### Trust Scores and Environment Modes
Wah now tracks a trust score for each task using `wah_set_trust` and `wah_get_trust`. The global environment mode is set via `wah_set_env_mode` so policy decisions can adapt to development, test or production use.

### Self‑evolution and Safeguards
`sched_tick` increments a counter and calls `wah_self_evolve` roughly every five
ticks so around 20% of runtime fuels Wah's learning loop. The routine logs an
event after passing `wah_emotion_filter` and `wah_metacog_check`, placeholder
hooks for future sentiment analysis and reflective safety checks.

### Task ID Query
Wah replies to `PID` with the caller's task ID. The interpreter exposes `OP_PID`
so AlienLanguage programs can push their own ID onto the stack for diagnostics.

### Task Naming
Tasks can now store a short human-readable label. Sending `NAME <label>` via Wah
updates the calling task's name, and `NAMES` replies with space-separated
`id:label` pairs for all active tasks. The scheduler records names alongside
task metadata so other diagnostics can refer to them.
