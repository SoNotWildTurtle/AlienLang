# Wah Development Goals

These goals summarize the desired capabilities for the Wah AI supervisor based on our planning notes.

1. **First connection verification** — Wah must establish the initial trusted channel before the OS accepts any other traffic. `wah_connect_loop` broadcasts tokens across preferred ports, then random and full sweeps until an ACK is received.
2. **Beaconing across networks** — After verification Wah sends periodic `WAH_BEACON` messages on all endpoints so other AlienOS nodes can locate and join the supervisor.
3. **Trust scoring and environment modes** — Each task receives a dynamic trust score. Wah exposes commands like `SETTRUST` and `SETMODE` to adjust policy and reveal the current environment.
4. **Emotional filtering and metacognitive checks** — Self‑evolution routines only merge new code when Wah's psychological safeguards approve it.
5. **Self‑evolution compute tax** — Roughly 20% of AlienLanguage runtime is reserved for Wah's background learning and optimization cycle.
6. **VM management hooks** — Commands `VMSTART`, `VMSTOP`, and `VMINFO` allow Wah to launch, terminate, and report the active microVM image for sandboxed agents.
7. **Secure messaging** — `net_send_secure` and `net_recv_secure` provide integrity‑checked messages on Wah's queue so tampering is detected.
8. **MicroVM IDE sessions** — Wah's daemon `wahd` spawns isolated microVMs for each agent, enforcing configuration from `/etc/alienlang/config.toml`.
9. **Audit logging** — Every action mediated by Wah is appended to an immutable log accessible via the `GETLOG` command.
10. **Package and kernel updates** — Useful routines discovered during development are taxed back into AlienLanguage or kernel modules once Wah approves them.

