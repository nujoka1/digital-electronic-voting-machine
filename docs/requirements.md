# Requirements and acceptance sequence

The prototype supports candidates A-D, voter IDs 1001-1010, officer PIN 5580, one-person-one-vote, simulated biometric matching, explicit confirmation, cancellation, duplicate and unknown-voter rejection, hidden active-poll totals, post-close results, EEPROM persistence with CRC, tamper lockout, three-invalid-PIN lockout, offline operation, and audit output without voter-to-candidate association.

The mandatory demonstration begins closed, rejects a pre-opening voter attempt, opens with PIN 5580, accepts voter 1001 for A, rejects a duplicate and voter 2001, rejects failed biometric for 1002, cancels 1003 without count change, accepts 1003 for B, rejects simultaneous candidate buttons, restarts and checks EEPROM recovery, closes with PIN 5580, displays results, rejects post-close voting, checks count invariants, checks audit privacy, and runs tamper on a resettable election.
