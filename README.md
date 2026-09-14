# Design and Simulation of a Digital Electronic Voting Machine

COEN 558 Digital Systems Design, Department of Computer Engineering, Ahmadu Bello University, Zaria, 2025/2026. Group 2: Fauzeeya, Nuhu, Rabiu and Farida.

Academic offline Arduino Mega 2560 prototype. Not INEC-certified, election-ready or production-secure.

## Current status

The canonical Arduino firmware compiles for Mega (9370 bytes flash; 398 bytes static RAM). Actual-sketch host tests pass for voting, cancellation, duplicate rejection, input handling, restart, final closure, persistent lockout and storage faults. See `docs/verification-matrix.md` for test boundaries and retained evidence.

Proteus circuit construction/acceptance testing, physical timing, MATLAB execution, independent-model parity, rendered diagrams and final DOCX/PDF/PPTX deliverables remain pending. The MATLAB reference model now follows the firmware's open, final-closed, accreditation, PIN-lockout and results rules, but still requires execution in MATLAB or GNU Octave. The source ZIP is a development snapshot, not a completed submission.

## Build and test

With Arduino CLI, the Arduino AVR core, LiquidCrystal, Python 3 and g++ installed:

```sh
python3 scripts/validate.py          # host tests, Mega build, evidence, HEX export
python3 scripts/validate.py --host  # actual-sketch tests and Python smoke
python3 scripts/validate.py --mega  # Mega compile and HEX export only
```

The script uses a temporary build directory and writes evidence to `evidence/`. Load `proteus/firmware.hex` into the simulated Mega. VS Code tasks expose validation and build commands. `firmware/src/main.cpp` includes the canonical `.ino`; do not maintain a second firmware copy. PlatformIO execution has not been verified.

## Workflow

Open Poll → press the physical Open button → enter `5580#` → voter ID `1001`–`1010` followed by `#` → biometric toggle LOW then `#` → release inputs and select one candidate → fresh Confirm press. During officer authentication, the LCD displays the entered PIN digits for verification; `*` clears the entry. Cancel abandons a ballot. Close Poll from voter entry → `5580#` → permanent final results. No network or real biometric recognition is used.

Poll phase, accreditation, votes, failed PIN attempts and lockout survive restart. Three invalid PINs or Tamper lock persistently. Corrupt/older/incompletely written EEPROM is preserved and locks; only truly erased storage initializes a new election. There is no automatic erase/reset command. See `docs/firmware-review.md` and `proteus/PROTEUS_WIRING.md` before simulation.

## Layout and collaboration

- `arduino/`: canonical firmware; `firmware/`: PlatformIO wrapper.
- `tests/host/`: actual-sketch harness; `scripts/validate.py`: reproducible checks.
- `matlab/`: independent reference model and tests aligned with firmware poll phases and lockout rules; execution remains environment-dependent.
- `design/`: pin map, Boolean logic and diagram sources.
- `proteus/`: usage guide, wiring checkpoint and compiled HEX; no fabricated schematic files.
- `docs/`, `presentation/`: traceability, supervisor demonstration guide and pending office deliverables.
- `evidence/`: genuine executed-check outputs and source fingerprints.

Fauzeeya, Nuhu, Rabiu and Farida should preserve existing changes, work on focused branches, run the relevant validation command and review evidence before merging. Keep pin map and wiring instructions consistent. Do not label simulated host tests as Proteus execution. No commits, pushes or deployments are performed by the validation script.
