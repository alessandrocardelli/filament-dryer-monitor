# Firmware diagnostics / test archive

Index and source provenance: [TEST_FIRMWARE_ARCHIVE.md](TEST_FIRMWARE_ARCHIVE.md).

- `validated_integrated_2026_10_08/`: exact source blobs copied from historical Git commit `3535dba9643e77d48e14eccc109606e3a5d758cf`, including dependencies/assets.
- `reconstructed_fan_2026_10_08/`: four Arduino sketches reconstructed from 2026-10-08 chat procedures. **Not verbatim originals; not freshly hardware-tested.**
- `../diagnostics/ntc_reference_viewer/`: existing heater-OFF diagnostic preserved in the normal source tree.

Always distinguish code provenance from historical test evidence. Do not enable a heater test just because a chat mentioned one.
