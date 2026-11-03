# Main reference state

Documentation reconciled 2026-11-03. `main` remains the non-heating SAFE BRING-UP reference; no firmware, flags, configuration, safety limits or hardware changed.

## Development state

The single active experimental branch is [`experiment/step10-heater-diagnostics-air25-2026-10-28`](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step10-heater-diagnostics-air25-2026-10-28/docs/PROJECT_STATE.md). Its [PROJECT_STATE](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step10-heater-diagnostics-air25-2026-10-28/docs/PROJECT_STATE.md) is the sole current checkpoint for ongoing development: verified source, last physical trial, actual installed-image uncertainty, problems and next action. Read its [TODO](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step10-heater-diagnostics-air25-2026-10-28/docs/TODO.md) and decisions before continuing experimental work. [PR #18](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18) stays DRAFT; do not merge heater code into main.

## Main evidence and historical checkpoint

Main preserves local UI/cycle/settings/branding and heater-OFF behavior. Its historical software verification, first-article measurements, hardware status, hashes, thermal evidence and consolidation provenance remain in the [complete pre-correction snapshot](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/5a805d7ed5a74a13dc89a412c7981aae5ce8958b/docs/PROJECT_STATE.md). The experimental N8580 profile and “next upload/test” paragraphs in that snapshot describe an earlier checkpoint, not main's code or a current pending test. Later experiment results are on the active branch; do not repeat old trials from this document.

Actual KiCad sources govern electrical claims. D020 is a validated physical bodge for a future deliberate revision, not a change to the released source; TCO105 remains installed by report and must not be bypassed automatically. Software/CI success is separate from physical qualification.
