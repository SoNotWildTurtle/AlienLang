# AlienOS Goals

This document outlines the guiding goals for AlienOS development.

1. **Security-centric design** with a built-in interpreter for AlienLanguage.
2. **Advanced networking protocol** for Wah and AI agents, with Wah mediating safe AlienLanguage usage.
3. **Self-verification and healing** features inspired by NSA security standards.
4. **Command-line interface** initially, with scope for a future GUI.
5. Wah utilizes **20% of available CPU** to evolve itself, the OS, and AlienLanguage.
6. AlienLanguage will be a **symbolic application language** to be defined later.
7. **Terminal IDE mediated by Wah**, spawning microVMs per agent with trust-based permissions.
8. **Emotional filtering and metacognitive safeguards** so Wah's decisions remain psychologically appropriate.
9. **Self-evolving kernel** where new AlienLanguage features are merged after Wah's review.
10. **Blockchain-backed ledger** integrated into the interpreter for trusted operations.
11. **Fault-tolerant runtime** that terminates misbehaving tasks so Wah's experiments cannot crash the kernel.
12. **Wah as a cloud-based supervisor**. The first AlienOS boot launches Wah and starts a beacon so subsequent clients automatically connect and communicate.
13. **Verified first connection**. All inbound traffic is dropped until Wah verifies herself during initialization.
14. **Integrity-checked messaging** using `net_send_secure` and `net_recv_secure` to detect tampering.
15. **Aggressive auto-connect** so Wah repeatedly sends a `WAH_CONNECT` handshake
    on boot until a peer responds or a timeout occurs.
16. **Package management** so kernel modules and language updates can be installed from in-memory packages.
