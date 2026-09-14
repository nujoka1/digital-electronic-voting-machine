# Evidence

Reproduce automated checks with `python3 scripts/validate.py`.

- `arduino/compile.txt`: actual Mega compile command, output and exit code.
- `arduino/DigitalVotingMachine.ino.hex`: HEX from that compile; also exported to `proteus/firmware.hex`.
- `host/compile.txt`, `host/firmware-tests.txt`: actual-sketch C++ host build and behaviour tests, including simulated restart, tamper, input handling and power cuts.
- `host/reference-smoke.txt`: existing independent Python smoke assertions.
- `host/structure.txt`: file-presence checks only.
- `validation-sha256.txt`: fingerprints of Mega firmware source and exported HEX.
- `host-sha256.txt`: fingerprints of the source and harness used by the latest host tests.

Host EEPROM/pin/LCD mocks do not constitute Proteus or hardware evidence. No MATLAB execution, Proteus screenshots, measured timings or physical device results are claimed. Leak detection is disabled for the traced host runtime; address and undefined-behaviour sanitizers remain enabled.
