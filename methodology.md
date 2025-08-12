# Development Methodology

This document explains how to use the various notes files while extending the project.

- **notes.md** – captures high level project planning and design decisions. Review
  this when starting a new feature.
- **dev_notes.md** – contains implementation details, cross compiler setup and
  descriptions of existing modules. Add brief rationale or gotchas after
  modifying code.
- **functions_modules.md** – general checklist for modules and OS features.
- **alienos_checklist.md** – tracks core operating system tasks.
- **wah_checklist.md** – tracks development of the Wah AI supervisor.
- **alienlanguage_checklist.md** – tracks language design tasks.
- **interpreter_checklist.md** – tracks work on the bytecode interpreter.
- **personal_notes.gz** – compressed private notes not shared publicly.
- **personal_notes.gz** – compressed private notes not shared publicly.

## Directory Layout

- `os/` – operating system source. Initially this hosts the `kernel/` but will
  grow with drivers and system libraries.
- `lang/` – AlienLanguage compiler and runtime sources.
- `wah/` – Wah AI agent components.

Each component evolves independently but should remain synchronized through the
notes above.

The build script `grub_bootstrap.sh` compiles the kernel from `os/kernel` and
creates a minimal ISO for testing. Future scripts will build the language and
AI components from their respective directories.
