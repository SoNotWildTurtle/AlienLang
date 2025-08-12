# AlienLang

AlienLang aims to provide an AI-focused coding environment and experimental operating system.
AlienOS integrates a self-evolving AI agent called Wah. The system invests roughly 20% of runtime from AlienLanguage programs into Wah's continuous learning. Wah ensures all changes pass emotional filtering and metacognitive checks before new code is merged back into the kernel.
Wah's scheduler hooks now invoke a background self-evolution routine every few
ticks so this compute "tax" fuels experiments. The `wah_self_evolve` function
records each cycle in the kernel log after passing `wah_emotion_filter` and
`wah_metacog_check` guards.

## Components
- **Wah**: A general AI interface that enables other agents to utilize AlienLang.
- **AlienOS**: Minimal GRUB-based OS intended to run in sandboxed environments. Wah can generate patches on the fly to heal faulty routines, merging successful fixes into the evolving kernel. Wah also operates as a cloud-based supervisor: the first boot starts a beacon so future AlienOS machines automatically connect back and receive instructions.
The kernel now ships with this beaconing routine built in. On initialization,
Wah broadcasts a `WAH_BEACON` message to every registered network endpoint so
other AlienOS nodes on secured or isolated networks can discover the
supervisor.

## Repository Layout
- `os/` contains the operating system sources. The kernel resides in `os/kernel`.
 - `lang/` contains the AlienLanguage toolchain. A tiny `alienc` compiler
   translates basic expressions into the bytecode consumed by the kernel VM.
 - `wah/` stores the Wah AI components.
- Support scripts, notes and patches live in the repository root.

Scripts such as `grub_bootstrap.sh` install dependencies via apt, build a minimal GRUB ISO, and boot it with QEMU in headless mode.
The build script also compiles a tiny multiboot kernel that prints a message at boot. See `session_goals.md` for short-term objectives that get updated every coding session.
The kernel now includes a simple bump allocator so future modules can request
memory during early initialization. A multilevel feedback scheduler provides
basic preemptive task switching with three priority queues. Wah runs at the
highest priority and audits system calls from other tasks. The scheduler tracks
CPU ticks for each task so Wah can report runtime statistics. The scheduler consults
the `security` module so only Wah may create new tasks. A very small
in-memory filesystem is now included so kernel modules can store and retrieve
static data during boot.
An equally small device manager lets modules register initialization hooks.
The kernel calls `device_init_all` during boot, providing a central place for
hardware drivers to attach as AlienOS evolves.
An initial networking layer has been added as a stub message queue so modules
can exchange packets without hardware support.
It now includes optional integrity checking: messages sent with
`net_send_secure` append a simple hash so `net_recv_secure` can verify data
before processing.
Wah now exposes a simple message interface built on this queue so other tasks
can send text commands to the Wah supervisor. Supported commands include
`PING`, `SETMODE`, `SETTRUST <id> <score>`, `GETMODE`, `GETTRUST [id]`, `GETLOG`, `CLRLOG`, `TIME`, `RTC`, `RANDOM`, `TASKS`, `PID`, `DEVICES`, `PKGS`, `USAGE`, `TRUSTS`, `MEM`, `STATUS`, `CFGSET <key> <val>`, `CFGGET <key>`, `NAME <label>`, `NAMES`, and `LOG <msg>` for
basic diagnostics. `CFGSET` and `CFGGET` manipulate a tiny in-memory configuration store. `TASKS` lists the IDs of currently active tasks while `PID` returns the caller's task ID and `USAGE`
reports CPU tick counts for each running task. `TRUSTS` replies with `id:score` pairs showing each task's trust value. `PKGS` returns the names of installed packages separated by spaces. `NAME` stores a human-readable label for the calling task and `NAMES` lists all `id:label` pairs. Wah also understands `VMSTART <image>`, `VMSTOP`, `VMINFO`, `IDE <image>`, and `IDEINFO` to
control sandboxed microVM sessions. `VMINFO` replies with the active image name so tools know which environment is running. `IDE <image>` starts a trusted IDE session for the current task and `IDEINFO` reports the running session's image and trust score.
`KILL <id>` terminates the specified task ID.
The kernel ships with a tiny in-memory package manager. `pkg_install` stores
packages in the filesystem so later updates can bundle new modules. A demo
package `welcome.txt` is installed during boot.
Wah also tracks trust scores for tasks and supports an environment mode
(development, test, or production) via `wah_set_env_mode`.
During boot the network layer rejects all inbound traffic until Wah is
verified. The `wah_init` routine now calls `wah_connect_loop`. Wah
generates a random token and begins a handshake by broadcasting
`HELLO<token>` messages on multiple ports. The network only marks her
verified after receiving an `ACK<token>` reply. She first probes ten
preferred ports for five seconds, then sweeps random ports for ten
seconds, and finally scans all ports for five seconds. If the handshake
fails she waits thirty seconds before retrying. This staged approach keeps
traffic low while ensuring Wah eventually secures the channel and becomes
the first authenticated peer.
The kernel also programs a periodic timer and installs an interrupt descriptor
table so scheduling occurs automatically while the CPU idles.
Boot events and Wah audits are appended to a small in-memory log. AlienLanguage
programs may write custom entries with the `logmsg` opcode.
All Wah messages and audit entries are prefixed with the system uptime in
milliseconds so every action carries a timestamp.

### Cross-Compilation
AlienOS builds a 64-bit kernel. For portability a cross toolchain targeting
`x86_64-elf` is recommended. After installing dependencies such as
`build-essential`, `bison`, and `flex`, build `binutils` and `gcc` with the
`--target=x86_64-elf` option. Once `x86_64-elf-gcc` and `x86_64-elf-ld` are
available on your `PATH`, run `./grub_bootstrap.sh` to compile the kernel and
create an ISO. If the cross compiler is missing the script falls back to the
host compiler.

Custom GRUB commands live under `patches/`. To use them you must rebuild GRUB
from source and apply the patches manually; the default `grub_bootstrap.sh`
 script only copies system modules. Example patches include `add_command.patch` for addition, `math_commands.patch` for multiply and divide, `array_commands.patch` for early array support,
  `http_fetch.patch` for basic networking, `io_commands.patch` for simple hardware
  I/O, `var_commands.patch` for variable storage, `sha256_command.patch` which
 adds a SHA‑256 hashing command for integrity checks, `sha256_file.patch` for
 hashing files, `beacon_command.patch` to broadcast a beacon over HTTP,
  `msleep_command.patch` adding a millisecond delay command,
  `randport_command.patch` which prints a random network port,
  `uuidgen_command.patch` to generate a random UUID,
  `string_commands.patch` providing `strlen` and `substr` utilities,
`memdump_command.patch` to inspect memory contents, and
`hotpatch_command.patch` to copy a file's contents into memory so new code
 can be applied without rebooting.

`rtc_command.patch` adds an `rtc_time` command that prints the current
hardware clock time for early boot diagnostics.

## Hotpatch testing

Run `./hotpatch_test.sh` to build the kernel and ISO, boot it in QEMU, and
flip the `hotpatch_flag` via `gdb`. When the flag is set the kernel prints
`Hotpatch active`, providing a minimal framework for debugging and verifying
kernel hot-patches within the evolution pipeline.
The kernel exports a matching `msleep()` helper so tasks can pause using the PIT timer.

### Building the AlienLanguage Compiler
To experiment with AlienLanguage code outside the kernel, a small prototype
compiler lives in `lang/`. Build it with:

```
make -C lang
```

Then run `./alienc "add 2 3"` to produce bytecode instructions for the kernel
interpreter. The compiler understands basic arithmetic operations and has
`wahmsg` and `logmsg` forms for sending text to Wah or the kernel log (e.g.
`./alienc "wahmsg hello"` or `./alienc "logmsg debug"`).
These may be used to control Wah's microVM by sending `wahmsg "VMSTART img"`
or `wahmsg "VMSTOP"`.
`./alienc kill N` pushes task ID `N` and sends a `KILL` command to Wah.
You can also generate a random value with `./alienc rand` which emits
`OP_RANDOM` followed by a print and halt. `./alienc randport` emits
`OP_RANDPORT` so programs can probe a random network port via the VM.
`./alienc time` emits `OP_TIME` to push the system uptime in milliseconds.
`./alienc env` emits `OP_GETENV` so bytecode can query Wah's environment mode.
`./alienc hash N` emits `OP_HASH` to push a simple hash of integer `N`.
`./alienc set I V` stores value `V` into variable slot `I` using `OP_STORE` and
`./alienc get I` retrieves it with `OP_LOAD` followed by a print.
`./alienc "loop N"` counts down from `N` to zero using new `OP_JMP` and `OP_JZ`
opcodes for basic control flow.
`./alienc pid` emits `OP_PID` to push the current task ID.
`vm_register_handler` allows Wah to add new opcodes at runtime so the language
can grow as needed. Future work will extend the compiler with a real parser and
optimizer.

For planned GRUB commands, see `dev_notes.md`.

Development tasks and feature planning are tracked in `functions_modules.md`.
Project goals are outlined in `os_goals.md`. Additional planning notes,
including AlienLang's evolving kernel concepts, are recorded in `notes.md`.
`methodology.md` describes how to use these notes and directories during
development. Component-specific checklists live in `alienos_checklist.md`,
`wah_checklist.md`, `alienlanguage_checklist.md`, and `interpreter_checklist.md`.
High-level objectives for Wah are summarized in `wah_goals.md`.

### AlienLang Interpreter and Memory Isolation
The kernel now contains a very small AlienLang bytecode interpreter. Bytecode
programs operate on symbolic values and can request privileged actions through
Wah using the `OP_CALL_WAH` instruction. A new `OP_WAHMSG` opcode allows
bytecode to embed short text messages that Wah receives directly. The interpreter also includes
`OP_SLEEP` to pause execution in milliseconds using the kernel `msleep()` helper and
blockchain primitives so AlienLanguage code can append to and verify a simple
ledger with `OP_BCADD` and `OP_BCVERIFY`. A demo program executes during boot and
adds a value to the chain before verifying it and sending the result to Wah.
`OP_RANDOM` pushes a pseudo-random value generated by a basic LCG so programs can
experiment with nondeterminism. `OP_TIME` pushes the system uptime in
milliseconds, allowing programs to implement timers or measure delays.
`OP_GETENV` pushes Wah's current environment mode onto the stack so bytecode can
adapt behavior for development, test or production settings.
`OP_HASH` computes a simple hash of the top stack value, useful for quick
integrity checks.
`OP_RANDPORT` pushes a random network port number so programs can attempt
connections through the networking stub.
`OP_STORE` saves the top stack value into one of 16 variable slots and
`OP_LOAD` pushes the stored value back onto the stack so programs can maintain
state between instructions.
The VM allocates its own stack with `vm_init_context` so tasks may run larger
programs, and a `trace` flag records each opcode to the kernel log for
debugging.
The interpreter now applies a tiny JIT-style optimizer before running which
folds constant `OP_PUSH`/`OP_ADD` sequences into a single push instruction.
Each task now receives a private page table cloned
from the kernel. The scheduler loads this table before running the task so basic
isolation is in place. Helper routines `paging_new_table`, `paging_clone_kernel`
and `paging_load` lay the groundwork for more advanced memory management.
The IDT now includes a page fault handler which notifies Wah about faults.
If the fault originated from a regular task, the handler terminates that task
and reschedules the next one so the kernel continues running. Wah can still log
the event, but the system avoids crashing. When the fault occurs inside Wah
itself the handler halts after printing "PF" so developers can debug the issue.

## AlienLang IDE and Wah Integration
The project includes a terminal-based IDE built around Wah's permission model. Every action is
mediated by Wah, which spawns sandboxed microVM sessions for each agent and logs all
operations. The AlienLang compiler and language server run only inside these microVMs, ensuring
that language internals remain isolated while agents interact through Wah as a man-in-the-middle.
Trust scores determine available features and environment modes (development, test, production)
are selected via `/etc/alienlang/config.toml`.
Future graphical frontends can layer on top of this modular IDE.
