# Interpreter Checklist

Track development of the AlienLang bytecode interpreter.

 - [x] Fetch/decode loop for symbolic bytecode
 - [x] Stack and heap management
- [x] System call hooks into Wah
- [x] Blockchain operations (`OP_BCADD`, `OP_BCVERIFY`)
- [x] Wah messaging opcode (`OP_WAHMSG`)
- [x] Log message opcode (`OP_LOGMSG`)
- [x] Sleep opcode (`OP_SLEEP`)
- [x] Random opcode (`OP_RANDOM`)
- [x] Time opcode (`OP_TIME`)
- [x] Environment query opcode (`OP_GETENV`)
- [x] Random port opcode (`OP_RANDPORT`)
- [x] Hash opcode (`OP_HASH`)
 - [x] Variable store/load opcodes (`OP_STORE`, `OP_LOAD`)
 - [x] Control flow opcodes (`OP_JMP`, `OP_JZ`)
  - [x] Debugging features with symbolic tracing
 - [x] JIT or optimization experiments
 - [x] Wah task kill opcode (`OP_WAHKILL`)
 - [x] PID opcode (`OP_PID`)

