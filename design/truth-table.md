# Reduced truth table

The Boolean vote-enable expression is `$V = P . R . B . N . S . C$`. `X` means the remaining inputs are irrelevant after an earlier rejection.

| P | R | B | N | S | C | V | Decision |
|---|---|---|---|---|---|---|---|
| 0 | X | X | X | X | X | 0 | Poll closed |
| 1 | 0 | X | X | X | X | 0 | Unknown voter |
| 1 | 1 | 0 | X | X | X | 0 | Biometric mismatch |
| 1 | 1 | 1 | 0 | X | X | 0 | Duplicate voter |
| 1 | 1 | 1 | 1 | 0 | X | 0 | Zero or multiple candidate buttons |
| 1 | 1 | 1 | 1 | 1 | 0 | 0 | Confirmation absent/cancelled |
| 1 | 1 | 1 | 1 | 1 | 1 | 1 | Store exactly one vote |

For candidate inputs `A`, `B`, `C_d`, `D`, exactly one selection is:

`S = A.B'.C_d'.D' + A'.B.C_d'.D' + A'.B'.C_d.D' + A'.B'.C_d'.D`

Counter enables are `E_A = V.A`, `E_B = V.B`, `E_C = V.C_d`, and `E_D = V.D`. The required duplicate-rejection expression is `D_r = P.R.B.N'`. The firmware also rejects a known duplicate earlier during identification (`P.R.N'`), without requesting another biometric match. Alarm/lockout is `L = T + K`, where `K` is the third invalid officer-PIN attempt. Firmware implements these products as ordered guards in the voter-identification, biometric, ballot and confirmation states.


`S` denotes a valid latched single-candidate selection, not a requirement to keep the button held during confirmation. Zero buttons before selection does not enable a vote; releasing a valid selection preserves it. Multiple buttons clear the latch and require all buttons to be released before retry. Counter enables use the latched candidate. A fresh Confirm edge is required after selection; Cancel never increments a counter.

The primary expression is evaluated only in an operational, unlocked controller. Storage-integrity failure adds a separate fail-closed alarm condition beyond the required `L = T + K`; final closure prevents reopening.
