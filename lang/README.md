# AlienLanguage

This directory contains the userland tools for the AlienLanguage programming environment.
A small prototype compiler `alienc` translates simple expressions into the
bytecode understood by the kernel VM. Usage:

```
make -C lang
./alienc "add 2 3"
```

The compiler currently recognizes several forms:

- `add A B` — push integers `A` and `B`, add and print the result
- `sub A B` — subtract and print
- `mul A B` — multiply and print
- `div A B` — divide and print (integer division)
- `print N` — print the integer `N`
- `cfgset KEY VAL` — send `CFGSET KEY VAL` to Wah's config store
- `cfgget KEY` — request the value for `KEY` from Wah

AlienLanguage is designed to evolve. Wah can register new opcodes at
runtime using `vm_register_handler`, enabling the language to gain new
features without rebooting the OS.

It outputs a sequence of opcodes that can run under the in-kernel interpreter.
Future work will extend the grammar and produce richer bytecode.
