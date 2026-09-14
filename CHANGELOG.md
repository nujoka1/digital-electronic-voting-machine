# Changelog

## Unreleased

- Recreated the project scaffold from the available topic document and diagram.
- Added Arduino Mega firmware, independent reference-model tests, design documentation and validation placeholders.

## Firmware persistence and input repair

- Persist poll phase, final closure, PIN attempts, lockout and accreditation.
- Validate explicit record encoding, CRC-16, bitsets and count invariants; fail closed on damaged or interrupted writes without erasing legacy elections.
- Replace blocking input delays with debounced edges; require fresh confirmation and reset ambiguous selections.
- Enter voter ID directly after opening and document deadlines/cancellation.
- Add actual-sketch host tests, retained evidence and reproducible Mega/HEX export script.
- Update state/logic documentation and Proteus checkpoint; hardware/simulation and final office deliverables remain pending.
