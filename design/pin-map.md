# Arduino Mega 2560 pin map

| Function | Mega pin | Electrical behavior |
|---|---:|---|
| LCD RS, E, D4-D7 | 22, 23, 24, 25, 26, 27 | LCD outputs |
| Keypad rows 1-4 | 28, 29, 30, 31 | Matrix scan |
| Keypad columns 1-4 | 32, 33, 34, 35 | `INPUT_PULLUP` |
| Candidate A-D | 36, 37, 38, 39 | Pressed LOW |
| Confirm, Cancel | 40, 41 | Pressed LOW |
| Biometric match | 42 | Asserted LOW |
| Open, Close, Tamper | 43, 44, 45 | Asserted LOW |
| Green, red, yellow LED | 46, 47, 48 | Output through resistor |
| Buzzer | 49 | Passive buzzer output |
| Serial1 TX/RX | 18, 19 | 9600 8-N-1 |

Keypad rows are `1 2 3 A`, `4 5 6 B`, `7 8 9 C`, `* 0 # D`. Keypad letters are not candidate buttons. All physical button inputs connect from the Mega pin through the switch to GND; no pull-down resistors are used.
