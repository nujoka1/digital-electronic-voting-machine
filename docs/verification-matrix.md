# Verification matrix

Results below distinguish execution of the actual sketch with simulated host I/O from Mega compilation and real Proteus/device behaviour. Commands and outputs are retained; SHA-256 fingerprints tie evidence to source. Run `python3 scripts/validate.py` to reproduce the automated checks.

| Requirement ID | Instructor rubric item | Design element | Source file | Test case | Expected result | Observed result | Evidence file | Status |
|---|---|---|---|---|---|---|---|---|
| F-01 | Circuit model and firmware | Mega firmware | `arduino/DigitalVotingMachine/DigitalVotingMachine.ino` | Mega compile | HEX generated | 9286 bytes flash; 398 bytes static RAM | `evidence/arduino/compile.txt` | Passed |
| F-02 | Truth table and Boolean logic | Guarded vote acceptance | sketch, `design/truth-table.md` | `normal`, `selection`, `tenVoters` in `tests/host/test_firmware.cpp` | Reject invalid ballots; increment one counter per confirmed ballot | Host sketch tests passed; no exhaustive Boolean equivalence test | `evidence/host/firmware-tests.txt` | Passed |
| F-03 | State diagram | Nine-state FSM | `design/state-diagram.mmd` | Render and inspect diagram | Export matches state transitions | Source updated; export not rendered | None | Not Run |
| F-04 | Independent model | MATLAB model | `matlab/VotingMachine.m`, `matlab/run_tests.m` | MATLAB runner | Required scenarios execute | MATLAB/Octave unavailable on PATH; coverage incomplete | None | Blocked |
| F-05 | Circuit simulation | Genuine Proteus schematic | `proteus/PROTEUS_WIRING.md` | Full acceptance sequence | Real circuit matches workflow | No saved circuit or simulation evidence | None | Not Run |
| F-06 | Sequential control | Persistent poll phase, lockouts and counts | sketch | `normal`, `pinsAndTamper` | Restart restores open/final/locked state | Passed in host EEPROM model | `evidence/host/firmware-tests.txt` | Passed |
| F-07 | Persistence integrity | CRC-16 and transaction marker | sketch | `storage` | Interrupted/corrupt writes lock without erasure | 8 vote-write boundaries and 17 corrupted bytes checked | `evidence/host/firmware-tests.txt` | Passed |
| F-08 | Timing | Debounce, deadlines, response | sketch and Proteus | Instrumented simulation | Measured response with method | Host logical deadlines checked; physical timings unmeasured | None | Not Run |
| F-09 | Functional performance | Single selection and explicit confirm | sketch | `selection` | Bounce does not select; multiple buttons invalidate; held Confirm does not vote | Passed with synthetic input levels/time | `evidence/host/firmware-tests.txt` | Passed |
| F-10 | Count invariants | Counters, voted and accredited bitsets | sketch | `tenVoters`, `storage` | Total equals voted population and does not exceed accredited population | Ten-voter election and invalid-record checks passed | `evidence/host/firmware-tests.txt` | Passed |
| F-11 | Independent reference checks | Python model | `matlab/reference_model.py` | `tests/reference_model_smoke.py` | Existing smoke assertions pass | Passed; model is not parity-validated against revised firmware | `evidence/host/reference-smoke.txt` | Passed |
| F-12 | Source organization | Required source presence | repository | `tests/test_requirements.py` | Listed files exist | Passed; not a behaviour test | `evidence/host/structure.txt` | Passed |
| F-13 | Instructor deliverables | Report and presentation | `docs/`, `presentation/` | Render and inspect DOCX/PDF/PPTX | Complete office deliverables | Outlines only | None | Not Run |
| F-14 | Audit privacy | Event-only serial messages | sketch | `normal` plus source inspection | No voter ID or candidate name in audit stream | Checked host workflow; physical terminal not captured | `evidence/host/firmware-tests.txt` | Passed |

`Passed` applies only to the stated test boundary. It does not claim hardware, Proteus, MATLAB, electoral security or clinical/commercial validation. PlatformIO is a wrapper around the canonical sketch; only Arduino CLI was compiled here.
