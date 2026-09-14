# Firmware review and remaining checkpoint

## Findings

| Severity | Finding before repair | Current disposition |
|---|---|---|
| Critical | Final closure and lockouts were lost on restart | Poll phase, attempt count and lock flag are persisted; host restart tests pass |
| Critical | Invalid EEPROM silently created an empty election | Blank-only initialization; corrupt, legacy and incomplete records lock without erasure |
| Critical | Stored count validation checked only served <= 10 | Total equals voted population; voted subset of accredited; all masks/counters range checked |
| Major | Interrupted write could destroy state without an explicit fault | Commit marker plus CRC-16; interrupted transaction fails closed; host injection checks pass |
| Major | Confirmation could use held inputs and stale selection | Debounced input edges; multiple buttons clear latch; release and fresh Confirm required |
| Major | Blocking debounce delays slowed control loop | Removed explicit delays; 30 ms input stability filter; EEPROM/LCD calls remain synchronous |
| Major | Voter entry required undocumented initial star key | Open poll enters voter identification immediately |
| Major | Host tests exercised only a separate Python model | Added actual-sketch harness with synthetic pins, keypad, time and EEPROM |
| Critical | Required authentic Proteus evidence absent | Still pending; host tests and successful compile do not replace simulation |
| Critical | Required DOCX/PDF/PPTX absent | Still pending; current documents are outlines |
| Major | MATLAB/reference-model feature parity incomplete | Still pending; Python smoke passing is not proof of revised firmware parity |

## Storage contract

EEPROM byte 0x1F is the transaction marker. Bytes 0x20–0x30 hold a fixed 17-byte record: magic (2), version (1), poll phase (1), failed PIN attempts (1), lock flag (1), four candidate totals (4), served count (1), voted bitset (2), accredited bitset (2), CRC-16/CCITT-FALSE (2). Multi-byte values use low byte first except the two fixed magic bytes. CRC covers the first 15 bytes.

Before writing, marker becomes zero; payload is written and read back; marker 0xA5 commits it. Restart accepts only a committed, CRC-valid and internally consistent record. A failure between writes locks and preserves bytes rather than restoring a possibly stale election. Recovery after such a fault requires a separate supervised inspection procedure; no user-facing reset or automatic rollback is implemented. This sacrifices availability to avoid silently losing accepted totals.

Only an entirely erased marker and 32-byte record region initializes automatically. Prior-format EEPROM is intentionally incompatible and preserved. Do not flash a new format over a valuable election without first preserving its EEPROM. No physical EEPROM was changed during this work.

Accreditation is recorded after a successful simulated biometric check. Cancelled ballots retain accreditation, but no vote or voted flag. Stored data has aggregate candidate counts and voter eligibility bitsets, with no explicit mapping between voters and their candidate choices. This does not prevent inference by an adversary reading successive EEPROM snapshots; CRC is error detection, not cryptographic authentication.

## Timing contract

Inputs settle for 30 ms. Tamper is sampled raw before input processing, including during result display and feedback. No explicit `delay()` is used. EEPROM writes, LCD operations and serial output remain synchronous; real worst-case response must be measured. Host tests use synthetic time and do not measure MCU timing.

Officer entry and biometric verification each have a 30-second state deadline. ID entry has 30 seconds from its first digit (`*` clears entry). Ballot plus confirmation share a 30-second deadline from successful biometric verification. There is no idle-voter deadline. Feedback lasts 1200 ms and continues checking tamper.

## Verification limits

The host harness includes the actual sketch. AddressSanitizer and UndefinedBehaviorSanitizer checks passed. LeakSanitizer is disabled because this environment runs under tracing, where it failed to initialize. Mock EEPROM writes are byte-atomic; analog brownout, electrical button behaviour, exact AVR timing, screen layout and sound are not covered. Mega compilation checks the real target toolchain, not execution on a Mega.

Minimum next test: build the genuine Proteus schematic, load `proteus/firmware.hex`, and execute the full documented acceptance sequence, including restart with EEPROM retained, final closure and separate persistent-lockout scenarios. Then reconcile the independent models and complete/inspect the report and presentation.
