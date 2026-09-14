# Proteus manual checkpoint

Proteus 8 Professional is installed through Wine. Launch it from the desktop entry `Proteus 8 Professional`, or with the installed launcher command using Wine prefix `/home/nujoka/.wine`. The main executable is `/home/nujoka/.wine/dosdevices/c:/Program Files (x86)/Labcenter Electronics/Proteus 8 Professional/BIN/PDS.EXE`.

No `.dsn` or `.pdsprj` is included because no genuine Proteus project has yet been opened and validated on this host.

## Placement

Place one Arduino Mega 2560 at centre, a 4x4 keypad and officer controls at left, an LM044L 20x4 LCD with 10 kOhm contrast potentiometer upper-right, candidate/confirm/cancel buttons middle-right, LEDs and passive buzzer lower-right, and a Virtual Terminal at bottom.

## Wiring

1. Wire LCD VSS to GND, VDD to +5 V, VEE to the potentiometer wiper, RW to GND, RS/E/D4-D7 to 22/23/24/25/26/27. Leave D0-D3 open and wire the potentiometer outer terminals to +5 V and GND.
2. Wire keypad rows and columns to 28-35 in the order in `design/pin-map.md`.
3. Wire each momentary button from its Mega input to GND: A-D 36-39, Confirm 40, Cancel 41, Open 43 and Close 44.
4. Wire maintained biometric and tamper toggles so their normal state is HIGH and asserted state is LOW: 42 and 45.
5. Wire Mega outputs 46/47/48 through 220 or 330 Ohm resistors to LED anodes; connect cathodes to GND. Connect pin 49 to the passive buzzer positive and buzzer negative to GND.
6. Connect pin 18 (TX1) to Virtual Terminal RXD. Set terminal to 9600 baud, 8 data bits, no parity and 1 stop bit. RX1 to TXD is optional.
7. Compile the sketch and load the genuine generated HEX into the Mega's program-file property. Do not create a schematic file by hand.

## Acceptance sequence

Use the acceptance sequence in `docs/requirements.md`. Capture the LCD, LEDs, buzzer, terminal and Proteus schematic at meaningful transitions. Reset tamper on a disposable election state because lockout persists across restarts.

## Current firmware checkpoint (storage format 2)

Use `proteus/firmware.hex`, exported by `python3 scripts/validate.py --mega`.
The pin map is unchanged. No circuit or simulation is yet validated.

- Start with an erased EEPROM only for a **new disposable election**. The firmware automatically initializes genuinely erased storage. It deliberately locks on older-format data, bad CRC, inconsistent counters or an interrupted write. Preserve existing election EEPROM before any intentional reset; no erase/reset command is provided here.
- Open Poll, enter `5580`, then `#`. The LCD immediately requests voter ID; **no initial `*` is needed**. Enter four digits and `#`. `*` clears the current ID/PIN entry; keypad letters do not select candidates.
- With a valid voter, set Biometric Match LOW, allow it to settle, then press `#`. Accreditation is saved at this point, even if the ballot is later cancelled.
- Release candidate inputs before choosing. Press and release one candidate button, then press Confirm separately. Simultaneous candidate buttons invalidate the selection; release all and choose again. A Confirm button held before selection cannot store a vote.
- Cancel returns to voter entry without changing vote counters. A 30-second ballot deadline starts at successful biometric verification and includes confirmation. Officer authentication, ID entry and biometric verification also have 30-second deadlines. Idle voter entry has no deadline.
- Green indicates a saved vote for 1.2 seconds. Yellow indicates multiple selection. Red and sounder indicate lockout. These are configured intervals/behaviours, not measured Proteus timings.
- Close Poll is accepted from voter entry. Cancel the current ballot first if necessary. Authenticate with `5580#`; results appear and reopening is disabled, including after restart.
- Wrong-PIN attempts survive restart. Three wrong attempts lock persistently. Tamper LOW locks from every operational state, including results. Releasing Tamper or restarting does not unlock the election.
- Restart must retain EEPROM. An interrupted write may show `STORAGE LOCKOUT`; this is a deliberate fail-closed outcome, not permission to erase the record.

## Evidence to capture at the manual checkpoint

Build and save a genuine schematic using the placement and wiring above. Set the Mega Program File to `proteus/firmware.hex`; verify the selected Mega model and clock settings against its own documentation. Set the terminal to 9600/8-N-1. Run the full sequence in `docs/requirements.md` and save screenshots under `proteus/screenshots/` with:

1. Full schematic, readable net labels, Mega program-file configuration and initial closed LCD.
2. Open poll, valid biometric check, selected candidate and successful confirmed ballot.
3. Duplicate ID, unknown ID, biometric mismatch, cancellation and simultaneous-selection rejection.
4. Restored open state after restart, final results, and restored final closure after another restart.
5. Separate disposable-election runs showing persistent tamper and three-invalid-PIN lockouts.
6. Audit terminal showing event labels without voter IDs or candidate choices.

Record counts before/after cancellation through the acceptance sequence and final totals. Capture timing with a logic analyser/oscilloscope if available; do not infer a measured response from the configured delay. Host tests do not verify the LCD rendering, keypad electrical model, EEPROM behaviour in Proteus, or buzzer sound.
