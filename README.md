# Filament Dryer Monitor

ESP32 controller for eSUN eBOX Lite. **`main` is the non-heating SAFE BRING-UP reference; its heater output remains locked OFF.**

Current development continues exclusively on [`experiment/step10-heater-diagnostics-air25-2026-10-28`](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step10-heater-diagnostics-air25-2026-10-28/docs/PROJECT_STATE.md), [PR #18 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18). Read that branch's [PROJECT_STATE](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step10-heater-diagnostics-air25-2026-10-28/docs/PROJECT_STATE.md) for current source, physical evidence, installed-image uncertainty and unresolved qualification, and [TODO](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step10-heater-diagnostics-air25-2026-10-28/docs/TODO.md) for its priority. The experimental branch is HEATER-capable and is not an autonomous-use approval. No experimental code is integrated here.

Main's local menu, cycle timer, settings, branding and recorded peripheral bring-up remain available. Read [AGENTS](AGENTS.md), [main state](docs/PROJECT_STATE.md), [decisions](docs/DECISIONS.md), [firmware](firmware/README.md), [assembly](docs/ASSEMBLY.md) and [buck design](docs/BUCK_L7987L_DESIGN.md).

The former detailed README is preserved in the [historical snapshot](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/5a805d7ed5a74a13dc89a412c7981aae5ce8958b/README.md). Its N8580 upload/next-test instructions are superseded by the active branch's state; do not execute them as a current plan. D020 belongs to a deliberate future hardware revision; installed series TCO105 remains unchanged (D029).
