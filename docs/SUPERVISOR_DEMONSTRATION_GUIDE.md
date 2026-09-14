# Supervisor Demonstration Guide

## Project

**Design and Simulation of a Digital Electronic Voting Machine**

This project is an academic Arduino Mega 2560 prototype that demonstrates controlled voter identification, simulated biometric verification, candidate selection, vote confirmation, persistent storage and audit events.

It is an offline educational prototype. It is not an election-ready, production-secure or INEC-certified voting system.

## 1. Simple Explanation

The system starts with the poll closed. An officer opens the poll with a four-digit PIN. A voter enters an approved voter ID, passes a simulated biometric check, selects one candidate and confirms the ballot. The system stores only aggregate candidate totals and voter eligibility/status bits; it does not store a voter-to-candidate mapping.

The officer closes the poll with the same PIN. Only after closure are the final totals displayed. Invalid PIN attempts, tamper activation and storage corruption cause a persistent lockout.

## 2. Components Used

- Arduino Mega 2560: controller and sequential logic.
- 4x4 keypad: officer PIN and voter-ID entry.
- 20x4 LCD: prompts, status and final results.
- Candidate buttons A-D: candidate selection.
- Confirm and Cancel buttons: ballot decision controls.
- Biometric Match toggle: simulated biometric result.
- Open and Close buttons: officer controls.
- Tamper toggle: security input.
- Green, yellow and red LEDs: status indications.
- Passive buzzer: lockout warning.
- Virtual Terminal: event-only audit output.
- EEPROM: persistent poll state, counters and integrity-protected record.

## 3. Demonstration Sequence

### A. Show the initial state

Say:

> The machine begins closed, so a voter cannot cast a ballot before the officer authorizes the poll.

Show the LCD displaying the closed-poll message. Attempting voter input should be rejected.

### B. Open the poll

1. Press the physical Open Poll button.
2. Enter `5580` on the keypad.
3. Press `#`.

Say:

> The officer PIN is required before the poll becomes active. The LCD displays the entered digits so the wiring and keypad input can be verified during this academic demonstration.

The LCD should then request a voter ID. The audit terminal should show `POLL_OPENED` without showing the PIN or voter identity.

### C. Process a valid voter

1. Enter voter ID `1001` and press `#`.
2. Set Biometric Match LOW.
3. Press `#`.
4. Release the biometric control and candidate buttons.
5. Press candidate A.
6. Release candidate A.
7. Press Confirm as a fresh press.

Say:

> The voter must be registered, must pass the simulated biometric check and must select exactly one candidate. The vote is stored only after a separate confirmation action.

Expected result:

- LCD shows `VOTE STORED`.
- Green LED turns on briefly.
- Candidate A total increases by one.
- Audit terminal shows `BALLOT_ACCEPTED`.

### D. Demonstrate rejection and cancellation

Use separate voter attempts to show:

- Voter ID `2001`: rejected as an unknown voter.
- Voter ID `1001` again: rejected as a duplicate after the first accepted vote.
- A voter with Biometric Match not asserted: rejected as a biometric failure.
- A selected ballot followed by Cancel: cancelled without increasing the vote total.
- Two candidate buttons pressed together: rejected as multiple selection.

Say:

> These checks show that the system does not count an incomplete, duplicate, unidentified or ambiguous ballot.

### E. Demonstrate persistence

Stop and restart the simulation without erasing EEPROM.

Say:

> The stored poll phase, vote totals, voter status and security state survive a restart. The record includes a CRC and transaction marker so incomplete or corrupted writes fail closed.

Verify that the accepted vote remains counted and the poll remains open.

### F. Close the poll and show results

1. Ensure no ballot is active.
2. Press Close Poll.
3. Enter `5580` and press `#`.

Say:

> Closing the poll is a permanent state transition for this election. The final results are revealed only after successful closure.

Verify that the LCD displays the A-D totals and total votes. Try to open the poll again and explain that reopening is rejected.

### G. Explain security responses

On a disposable election state, demonstrate either:

- Three invalid officer PIN attempts, which create a persistent lockout.
- Tamper asserted LOW, which creates a persistent lockout.

Say:

> Lockout is fail-closed: releasing the input or restarting the machine does not automatically unlock the election.

## 4. What the Supervisor Should Observe

| Observation | What it demonstrates |
|---|---|
| Closed poll rejects voter input | Correct initial state and access control |
| Correct PIN opens the poll | Officer authentication |
| LCD updates during PIN entry | Keypad and display wiring |
| Valid voter reaches ballot selection | Identification and biometric flow |
| Confirmed vote changes one total | Sequential vote acceptance |
| Cancel leaves totals unchanged | Ballot cancellation |
| Duplicate and unknown IDs are rejected | One-person-one-vote and eligibility checks |
| Multiple candidate buttons are rejected | Unambiguous selection |
| Restart retains state | EEPROM persistence |
| Results appear after close only | Poll-phase control |
| Terminal contains event labels only | Audit privacy boundary |
| Three wrong PINs or tamper lock the system | Persistent fail-closed behavior |

## 5. Short Presentation Script

Use this summary if time is limited:

> This is an offline Arduino Mega voting-machine prototype. The officer first opens the poll with PIN 5580. Each voter enters a registered ID, completes a simulated biometric check, selects one of four candidates and presses Confirm. The firmware rejects unknown voters, duplicate voters, failed biometric checks, multiple selections and cancelled ballots. Votes are stored as aggregate totals with voter status bits, without a voter-to-candidate association. The officer closes the poll with the same PIN, after which final results are displayed and reopening is disabled. EEPROM preserves the state across restart, while invalid PIN attempts, tamper input and corrupted storage produce persistent lockout. Proteus demonstrates the circuit workflow; host tests verify firmware logic, but Proteus and physical timing claims require separate execution evidence.

## 6. Important Limitations to State

- Biometric verification is simulated by a switch; no real biometric sensor is used.
- The prototype is offline and has no network or central election database.
- Proteus results must not be claimed unless the simulation was actually run and evidence was saved.
- Host firmware tests do not prove LCD rendering, electrical keypad behavior, buzzer sound or physical timing.
- CRC detects storage corruption but is not cryptographic authentication.
- Displaying the PIN digits is intentional for wiring verification in this academic demonstration and would not be appropriate for a production voting system.
- This project demonstrates digital-system design concepts and must not be presented as a deployable election system.

## 7. Useful References

- Proteus setup and operation: `proteus/USAGE_GUIDE.md`
- Wiring and component placement: `proteus/PROTEUS_WIRING.md`
- Requirements and acceptance sequence: `docs/requirements.md`
- Pin allocation: `design/pin-map.md`
- Verification boundaries: `docs/verification-matrix.md`
