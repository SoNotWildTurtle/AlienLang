# Project Notes

This document collects general planning and progress notes for the AlienLang project.

- Establish baseline repository structure.
- Outline tasks for AlienOS and Wah AI development.
- Provide references and resources.

Current status: the bootstrap script reliably builds a GRUB-based ISO and boots
it briefly in QEMU. Development now focuses on adding kernel functionality such
as a minimal memory allocator.

The latest cycle introduces a prototype AlienLang interpreter. Paging now clones
the kernel's PML4 for each task and the scheduler loads the table on context
switch, establishing early per-task isolation for symbolic execution audited by
Wah.
The language now features a minimal blockchain implementation. Bytecode can
append values to the chain and verify its integrity. Wah records each chain
operation for auditing, laying groundwork for trusted distributed logs.
Bootloader patches continue to expand. A new `sha256_file` command computes the
SHA-256 hash of a file, improving integrity checks before modules load.
An in-memory filesystem now stores simple files for early modules to access.
The bootloader also offers a millisecond sleep command via `msleep_command.patch`, and the kernel mirrors this with a `msleep()` routine that delays using the PIT timer.
AlienLanguage now exposes this delay through `OP_SLEEP`, letting programs pause for a specified number of milliseconds.

Security groundwork began with a new kernel `security` module. It currently
contains stubs for initialization and task permission checks so future hardening
logic can hook into scheduling and memory management.

Cross-compilation instructions are provided so contributors can build the
64-bit kernel with an `x86_64-elf` toolchain on any Linux host.

Beaconing is now baked into the kernel. When Wah starts it broadcasts a short
`WAH_BEACON` message to all known network endpoints via the queue system so
other nodes can locate the first instance and establish communication even in
secured or segmented networks.
Network messages can now include a simple integrity hash using
`net_send_secure` and `net_recv_secure` so tasks can verify data has not been
tampered with.
Inbound messages are ignored until `wah_init` flags Wah as verified, forcing
her to be the first connection before the OS accepts any other traffic.
Wah now provides a `REBOOT` command so trusted tasks can reset the system after critical updates.
All Wah log entries and messages now include an uptime timestamp to aid
auditing and chronological analysis.

Trusted IDE sessions are now tracked through new `IDE` and `IDEINFO` commands.
Tools can start a microVM image under Wah's control using `IDE <image>` and
query the current session's image and trust score with `IDEINFO`.

## Core Operating System Functions
The OS must handle memory management, process scheduling, device and file management,
user interface services, security controls, and system performance monitoring. These
goals guide the incremental features added to AlienOS.

## AlienLang Evolving Kernel
AlienLang's kernel grows alongside development. Useful features and libraries
discovered during each cycle are fed back into the language as built-ins. Wah
acts as a gatekeeper, approving kernel updates while enforcing emotional
filtering and metacognitive logic so AI behavior remains aligned and
psychologically safe.


## AlienLang IDE Architecture
The IDE is terminal-based and relies on Wah's daemon (`wahd`) to spawn isolated microVM sessions for each AI agent. Within each session only the AlienLang runtime and language server are accessible. Wah enforces trust scoring, audit logging, and environment modes (development, test, production) defined in `/etc/alienlang/config.toml`. Agents interact with the IDE via a Wah-proxied language server protocol, keeping compiler internals hidden from untrusted code.

## Design and Development Plan
AlienOS evolves over repeated development cycles. Wah sits at the center,
auditing every task and collecting improvements. Useful routines can be
"taxed" back into AlienLanguage or kernel modules once Wah approves them.
Agents run inside sandboxed microVMs and receive trust scores that gate their
permissions. Starting from the minimal GRUB environment, the project will grow
into a secure networked OS capable of running Wah-controlled agents. This note
summarizes the vision for future coding sessions.

Wah now performs a staged connection attempt on boot. `wah_connect_loop`
gently probes a list of ten preferred ports for five seconds. Failing that,
she aggressively scans random ports for ten seconds and finally performs a
hyper aggressive full sweep for five seconds. If no node responds Wah waits
thirty seconds before repeating. This strategy balances low network noise
with a determined effort to secure the channel first.


Additional planning notes emphasize emotional filtering and metacognitive checks during Wah's self-evolution. AlienLanguage code submissions are taxed back into the kernel only after Wah verifies psychological safety. The scheduler allocates roughly 20% of AlienLanguage runtime to Wah's own learning threads, ensuring the OS grows smarter without starving user processes.
The new `wah_self_evolve` hook runs every few scheduler ticks and logs
"wah_evolve" to the system log. It currently performs no learning but provides a
placeholder for future optimization routines.

Recent work introduced a `randport` opcode so AlienLanguage programs can request
a random network port. This aids Wah's connection sweep without hardcoding
specific endpoints.
GRUB now also offers a `randport` command via `randport_command.patch` so early
boot scripts can select random ports before the kernel loads.
The new `uuidgen_command.patch` adds a `uuidgen` command to produce a random
UUID string for session identification during boot.

See `session_goals.md` for per-session objectives.
Development now includes a userland `alienc` compiler that outputs bytecode for the kernel VM. This allows quick testing of new language constructs without rebuilding the kernel.

## Implementation Markers
- [x] Hardware timer and IDT enabled
- [x] Per-task page tables
- [x] Wah security hooks for memory faults
- [x] Task termination on fault for kernel stability
- [ ] IDE microVM sandboxing
\nThe VM provides extension points so Wah can add new opcodes on demand. Executed bytecode is logged for pattern analysis, enabling automatic language healing if attacks corrupt scripts. The latest opcode, `&
The interpreter now includes `OP_LOGMSG` to append strings to a kernel log for diagnostics. Wah audits also use this log.
\n- Implemented OP_RANDOM for pseudo-random numbers.
\nA small in-memory package manager now lets the kernel store packages for later updates.
- Added constant-folding JIT optimizer
- Implemented OP_GETENV to expose Wah environment mode
- Added OP_TIME for fetching system uptime
- Added OP_HASH for lightweight integrity checks
- Added OP_STORE/OP_LOAD for simple variable storage in the VM
\nWah handshake now uses random tokens: HELLO<token>/ACK<token>.
- Added GETLOG command to retrieve the kernel log
- Added CLRLOG command to clear the kernel log
- Added TIME command so tasks can query uptime via Wah
- Added STATUS command reporting mode, trust score and VM activity
- Introduced OP_MOD and corresponding compiler support for modulo arithmetic
- Added VMINFO command to report the active microVM image
- Added OP_JMP and OP_JZ control flow opcodes; the `loop` compiler form uses
  them to count down from a starting value.

Hotpatch testing: run ./hotpatch_test.sh to flip the kernel hotpatch_flag via gdb and ensure runtime patches work.

Scheduler now tracks CPU tick usage per task. The new USAGE Wah command
lists "id:tick" pairs, providing basic job accounting.
Wah can now adjust or query trust scores for arbitrary task IDs via SETTRUST and GETTRUST.
- Wah now replies to `MEM` with used/total heap memory
- Added PID command and `OP_PID` to retrieve task IDs
- NAME/NAMES commands allow tasks to label themselves and query all task labels
- Added RTC command so tasks can read the hardware clock through Wah
