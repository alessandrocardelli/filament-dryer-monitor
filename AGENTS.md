# Project working instructions

## Authority

GitHub repository `alessandrocardelli/filament-dryer-monitor` is the authoritative project state.
Chat history, memory, summaries and previous assistant messages are secondary and must not override the repository.

For electrical connectivity and PCB implementation, the actual KiCad files are authoritative. In particular, the current `.kicad_sch`, `.kicad_pcb`, `.kicad_pro` and a freshly exported netlist override prose documentation if they disagree.

Manufacturer datasheets, reference designs and standards control device-specific requirements. The project Hardware Design Manual is a source-agnostic review framework and must not be modified as part of normal repository work.

## Before working

Every new chat must complete this bootstrap before proposing circuit changes, component choices, architecture changes or implementation work:

1. Identify the active Git branch and current HEAD. If work is on a feature branch, inspect that branch rather than `main`.
2. Read this `AGENTS.md` completely.
3. Read `README.md`.
4. Read `docs/PROJECT_STATE.md`.
5. Read `docs/DECISIONS.md`.
6. Read `docs/TODO.md`.
7. Inspect the actual source files relevant to the task. For current PCB work this normally includes `hardware/Filament_Dryer_Monitor.kicad_pcb`, `hardware/Filament_Dryer_Monitor.kicad_pro`, the relevant schematic sheet(s), and the current netlist when connectivity matters.
8. Check whether documentation is stale against source before relying on it.

Do not silently revisit or replace an established decision. If new primary-source evidence conflicts with a recorded decision, state the conflict explicitly before changing the design and update `docs/DECISIONS.md` only after the new direction is agreed.

## PCB review rules

- For bottom-side footprints, do not infer physical pad direction by manually reading raw local footprint coordinates. Use KiCad/pcbnew absolute pad positions or a trustworthy board view/screenshot.
- Preserve the current four-layer reference-plane decision unless explicitly reopened: both inner layers are solid GND planes.
- Do not interpret the L7987L `PGND`/`SGND` layout guidance as two separate electrical GND nets or split inner planes. Follow `docs/DECISIONS.md` for the project implementation.
- Use the ST L7987L datasheet and STEVAL-ISA198V1 layout/Gerbers as the primary buck layout reference, adapted to this four-layer board and its actual footprints.
- Do not claim the current board is fabrication-ready from the historical `hardware/DRC.rpt`. Run and review a fresh DRC after routing is complete.
- Do not regenerate or treat production Gerbers/CPL/BOM exports as final until layout review and current DRC are closed.

## Documentation discipline

Keep the handoff files current:

- `docs/PROJECT_STATE.md`: factual current checkpoint and source/branch state.
- `docs/DECISIONS.md`: durable engineering decisions that should not be silently revisited.
- `docs/TODO.md`: immediate next actions and open gates.
- `README.md`: high-level public project status.
- `docs/BUCK_L7987L_DESIGN.md`: detailed L7987L design and layout record.
- `hardware/docs/PROCUREMENT.md`: sourcing/footprint/manufacturing-preparation state.

When a substantial checkpoint is reached, update these before handing work to a new chat.