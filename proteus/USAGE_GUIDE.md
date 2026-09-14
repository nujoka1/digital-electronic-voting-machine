# Proteus Design Usage Guide

This guide explains how to open, configure and demonstrate the academic digital voting machine prototype in Proteus 8 Professional.

The project is an offline academic prototype. It is not an election-ready or production-secure voting system. Proteus execution and physical timing must be recorded separately from the automated host tests.

## 1. Required Files

Use these files from the repository:

- `proteus/DigitalVotingMachine.pdsprj` - Proteus project file.
- `proteus/firmware.hex` - compiled Arduino Mega firmware.
- `proteus/PROTEUS_WIRING.md` - detailed component and wiring reference.
- `design/pin-map.md` - authoritative Mega pin allocation.
- `docs/requirements.md` - acceptance sequence and expected behavior.

To regenerate the firmware HEX before opening Proteus, run from the repository root:

```sh
python3 scripts/validate.py --mega
```

The generated HEX is written to `proteus/firmware.hex`.

## 2. Open the Design

1. Start Proteus 8 Professional.
2. Open `proteus/DigitalVotingMachine.pdsprj`.
3. Open the Arduino Mega 2560 component properties.
4. Set the program file to `proteus/firmware.hex`.
5. Confirm the selected device is an Arduino Mega 2560 and keep its clock configuration consistent with the project.
6. Confirm the LCD, keypad, buttons, LEDs, buzzer and Virtual Terminal are wired as described in `proteus/PROTEUS_WIRING.md`.
7. Set the Virtual Terminal to 9600 baud, 8 data bits, no parity and 1 stop bit.
8. Save the project before starting the simulation.

Do not replace the genuine Proteus project with a hand-written placeholder file.

## 3. Pin and Control Map

| Function | Mega pin | Use in the simulation |
|---|---:|---|
| LCD RS, E, D4-D7 | 22-27 | LCD connection |
| Keypad rows | 28-31 | Matrix rows, in order |
| Keypad columns | 32-35 | Matrix columns, in order |
| Candidate A-D | 36-39 | Momentary buttons, active LOW |
| Confirm | 40 | Momentary button, active LOW |
| Cancel | 41 | Momentary button, active LOW |
| Biometric match | 42 | Maintained toggle, asserted LOW |
| Open poll | 43 | Momentary button, active LOW |
| Close poll | 44 | Momentary button, active LOW |
| Tamper | 45 | Maintained toggle, asserted LOW |
| Green, red, yellow LEDs | 46-48 | Status outputs |
| Buzzer | 49 | Passive buzzer output |
| Virtual Terminal RXD | Mega TX1 pin 18 | Audit output |

The 4x4 keypad layout is:

```text
1 2 3 A
4 5 6 B
7 8 9 C
* 0 # D
```

- `#` submits a PIN, voter ID or biometric step.
- `*` clears the current PIN or voter-ID entry.
- Keypad letters do not select candidates.
- Candidate buttons A-D are separate physical inputs.

## 4. Demonstration Workflow

### Start a new disposable election

Use erased EEPROM only for a new disposable demonstration. The firmware preserves existing election data and has no normal reset or erase command.

1. Start the simulation with the poll closed.
2. Verify that a voter attempt is rejected before opening.
3. Press the physical Open Poll button.
4. Enter `5580` on the keypad. The LCD displays the entered PIN for verification.
5. Press `#`.
6. The LCD should request a voter ID.

### Cast a ballot

1. Enter a valid voter ID from `1001` to `1010` and press `#`.
2. Set the Biometric Match toggle LOW and allow it to settle.
3. Press `#` to complete biometric verification.
4. Return the biometric toggle to its normal HIGH state when appropriate.
5. Release all candidate buttons.
6. Press and release exactly one candidate button, A, B, C or D.
7. Press and release Confirm as a fresh action.
8. The green LED and `VOTE STORED` message indicate a saved vote.

A held Confirm button must not store a vote. Simultaneous candidate buttons are rejected and indicated by the yellow LED/message. Cancel returns to voter entry without increasing the vote count.

### Test rejection cases

Demonstrate these cases using the acceptance sequence in `docs/requirements.md`:

- Unknown voter ID `2001`.
- A duplicate attempt using an already accepted voter ID.
- Biometric mismatch with the biometric toggle not asserted.
- Cancelled ballot.
- Simultaneous candidate selection.
- Voter attempt after final closure.
- Three invalid officer PIN attempts, which create a persistent lockout.
- Tamper asserted LOW, which creates a persistent lockout.

### Close the poll

1. Ensure the machine is at voter entry and no ballot is active.
2. Press Close Poll.
3. Enter `5580` and press `#`.
4. Confirm that final results are displayed.
5. Verify that another Open Poll attempt is rejected.

Final closure is permanent for that stored election. Restarting the simulation must not reopen it.

## 5. Restart and EEPROM Checks

Stop and restart the simulation without erasing EEPROM. Verify that:

- The poll phase is restored.
- Accepted-voter and vote-count state is retained.
- Failed PIN attempts remain retained.
- A tamper or three-attempt lockout remains locked.
- Final results remain final after restart.

If the EEPROM contains an invalid CRC, an older record format or an incomplete write, the firmware enters storage lockout. Preserve that state as evidence; do not erase it unless starting a separate disposable election.

## 6. Audit Terminal

The Virtual Terminal receives event labels such as `POLL_OPENED`, `BALLOT_ACCEPTED`, `DUPLICATE_ATTEMPT`, `POLL_CLOSED` and `LOCKOUT`.

The audit output must not contain voter IDs or candidate choices. Capture the terminal at important transitions and check that only event-level information is shown.

## 7. Evidence to Capture

For a complete Proteus demonstration, save screenshots under `proteus/screenshots/` showing:

1. The complete schematic, initial closed LCD and Mega program-file setting.
2. Poll opening, biometric verification, candidate selection and a confirmed vote.
3. Unknown voter, duplicate, biometric failure, cancellation and multiple-selection rejection.
4. State restoration after restart and final results after closure.
5. Persistent PIN and tamper lockouts on disposable election states.
6. The audit terminal without voter-to-candidate associations.

Record the vote totals before and after cancellation and at final closure. Do not describe host tests as Proteus execution, and do not claim measured timing unless it was measured with an instrument.

## 8. Troubleshooting

- **LCD is blank:** check power, contrast potentiometer, RW-to-GND and LCD pins 22-27.
- **Keypad input does not work:** verify row order 28-31, column order 32-35 and that only one key is pressed.
- **Buttons behave backwards:** inputs are active LOW and must connect to GND when pressed or asserted.
- **No audit output:** connect Mega TX1 pin 18 to Virtual Terminal RXD and verify 9600/8-N-1 settings.
- **Program does not start:** check that the Mega program file points to `proteus/firmware.hex` and regenerate it with `python3 scripts/validate.py --mega`.
- **System is locked:** inspect the LCD and terminal reason. Releasing Tamper or restarting does not unlock a persistent lockout.

Proteus behavior, LCD rendering, electrical keypad scanning, EEPROM persistence and buzzer output require real Proteus execution; they are not proven by the host firmware tests alone.
