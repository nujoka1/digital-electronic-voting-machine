#include <Arduino.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>

// Canonical implementation; PlatformIO includes this file directly.
enum class State : uint8_t { BOOT, POLL_CLOSED, OFFICER_AUTHENTICATION,
  VOTER_IDENTIFICATION, BIOMETRIC_VERIFICATION, BALLOT_ENABLED, CONFIRMATION,
  RESULTS, TAMPER_LOCKOUT };
enum class Poll : uint8_t { NOT_OPENED, OPEN, FINAL_CLOSED };
constexpr uint8_t candidatePins[] = {36,37,38,39};
constexpr uint8_t confirmPin=40, cancelPin=41, biometricPin=42, openPin=43,
  closePin=44, tamperPin=45, greenPin=46, redPin=47, yellowPin=48, buzzerPin=49;
constexpr uint8_t rowPins[] = {28,29,30,31}, colPins[] = {32,33,34,35};
const char keypadMap[4][4] = {{'1','2','3','A'},{'4','5','6','B'},
  {'7','8','9','C'},{'*','0','#','D'}};
constexpr uint32_t debounceMs=30, sessionTimeoutMs=30000, feedbackMs=1200;
constexpr uint8_t voterCount=10, recordSize=17;
constexpr uint16_t eepromAddress=0x20, markerAddress=0x1F, voterMask=0x03FF;
LiquidCrystal lcd(22,23,24,25,26,27);

// Aggregate counters and eligibility bitsets only: no voter-to-candidate mapping.
struct Persisted {
  Poll poll;
  uint8_t attempts, locked, votes[4], served;
  uint16_t voted, accredited;
};
Persisted record{};
State state=State::BOOT;
bool storageHealthy=false, authenticatingForOpen=false, biometricPassed=false;
int8_t activeVoter=-1, selectedCandidate=-1;
char inputBuffer[5] = {};
uint8_t inputLength=0;
uint32_t sessionStartedAt=0, feedbackStartedAt=0;
bool feedbackActive=false, selectionArmed=false, confirmArmed=false;

uint8_t bitCount(uint16_t value) {
  uint8_t count=0;
  while (value) { count += value & 1; value >>= 1; }
  return count;
}
bool validRecord() {
  uint16_t total=0;
  for (uint8_t count : record.votes) { if (count>voterCount) return false; total+=count; }
  return static_cast<uint8_t>(record.poll)<=2 && record.attempts<=3 && record.locked<=1 &&
    (record.attempts<3 || record.locked) && !(record.voted & ~voterMask) &&
    !(record.accredited & ~voterMask) && !(record.voted & ~record.accredited) &&
    total==record.served && total==bitCount(record.voted) && total<=bitCount(record.accredited) &&
    (record.poll!=Poll::NOT_OPENED || (total==0 && record.accredited==0));
}
uint16_t crc16(const uint8_t *data, uint8_t length) {
  uint16_t crc=0xFFFF;
  for (uint8_t i=0;i<length;++i) {
    crc ^= static_cast<uint16_t>(data[i])<<8;
    for (uint8_t b=0;b<8;++b) crc = (crc & 0x8000) ? (crc<<1)^0x1021 : crc<<1;
  }
  return crc;
}
void encode(uint8_t *bytes) {
  bytes[0]=0x55; bytes[1]=0x80; bytes[2]=2;
  bytes[3]=static_cast<uint8_t>(record.poll); bytes[4]=record.attempts; bytes[5]=record.locked;
  for (uint8_t i=0;i<4;++i) bytes[6+i]=record.votes[i];
  bytes[10]=record.served;
  bytes[11]=record.voted & 0xFF; bytes[12]=record.voted>>8;
  bytes[13]=record.accredited & 0xFF; bytes[14]=record.accredited>>8;
  uint16_t crc=crc16(bytes,15); bytes[15]=crc & 0xFF; bytes[16]=crc>>8;
}
void audit(const __FlashStringHelper *event) { Serial1.print(F("AUDIT: ")); Serial1.println(event); }
void message(const __FlashStringHelper *first, const __FlashStringHelper *second=nullptr) {
  lcd.clear(); lcd.setCursor(0,0); lcd.print(first);
  if (second) { lcd.setCursor(0,1); lcd.print(second); }
}
void storageFault() {
  storageHealthy=false; state=State::TAMPER_LOCKOUT;
  digitalWrite(redPin,HIGH); digitalWrite(greenPin,LOW); digitalWrite(yellowPin,LOW);
  tone(buzzerPin,2200); message(F("STORAGE LOCKOUT"),F("Preserve EEPROM")); audit(F("STORAGE_FAULT"));
}
bool saveRecord() {
  if (!storageHealthy || !validRecord()) { storageFault(); return false; }
  uint8_t bytes[recordSize]; encode(bytes);
  // Fail closed on interrupted writes. Never silently roll back an acknowledged vote.
  EEPROM.update(markerAddress,0);
  for (uint8_t i=0;i<recordSize;++i) EEPROM.update(eepromAddress+i,bytes[i]);
  for (uint8_t i=0;i<recordSize;++i)
    if (EEPROM.read(eepromAddress+i)!=bytes[i]) { storageFault(); return false; }
  EEPROM.update(markerAddress,0xA5);
  if (EEPROM.read(markerAddress)!=0xA5) { storageFault(); return false; }
  return true;
}
bool loadRecord() {
  uint8_t bytes[recordSize]; bool erased=EEPROM.read(markerAddress)==0xFF;
  // Inspect the whole former record area too; legacy elections must not be erased.
  for (uint8_t i=0;i<32;++i) if (EEPROM.read(eepromAddress+i)!=0xFF) erased=false;
  if (erased) { record={}; storageHealthy=true; return saveRecord(); }
  for (uint8_t i=0;i<recordSize;++i) bytes[i]=EEPROM.read(eepromAddress+i);
  if (EEPROM.read(markerAddress)!=0xA5 || bytes[0]!=0x55 || bytes[1]!=0x80 || bytes[2]!=2 ||
      crc16(bytes,15)!=(static_cast<uint16_t>(bytes[16])<<8 | bytes[15])) {
    storageFault(); return false;
  }
  record.poll=static_cast<Poll>(bytes[3]); record.attempts=bytes[4]; record.locked=bytes[5];
  for (uint8_t i=0;i<4;++i) record.votes[i]=bytes[6+i];
  record.served=bytes[10]; record.voted=bytes[11] | static_cast<uint16_t>(bytes[12])<<8;
  record.accredited=bytes[13] | static_cast<uint16_t>(bytes[14])<<8;
  storageHealthy=validRecord();
  if (!storageHealthy) storageFault();
  return storageHealthy;
}
void lockout(const __FlashStringHelper *reason) {
  if (storageHealthy && !record.locked) { record.locked=1; if (!saveRecord()) return; }
  state=State::TAMPER_LOCKOUT; activeVoter=-1; selectedCandidate=-1; biometricPassed=false;
  digitalWrite(redPin,HIGH); digitalWrite(greenPin,LOW); digitalWrite(yellowPin,LOW);
  tone(buzzerPin,2200); message(F("SYSTEM LOCKED"),reason); audit(F("LOCKOUT"));
}

struct Debounced {
  uint8_t raw=0, stable=0; uint32_t changedAt=0; bool rose=false;
  void update(uint8_t value) {
    rose=false;
    if (raw!=value) { raw=value; changedAt=millis(); }
    if (stable!=raw && static_cast<uint32_t>(millis()-changedAt)>=debounceMs) {
      stable=raw; rose=stable!=0;
    }
  }
};
Debounced buttons[10], keypad;
char scanKeypad() {
  char found=0; uint8_t count=0;
  for (uint8_t row=0;row<4;++row) {
    digitalWrite(rowPins[row],LOW); pinMode(rowPins[row],OUTPUT);
    for (uint8_t col=0;col<4;++col) if (digitalRead(colPins[col])==LOW) {
      found=keypadMap[row][col]; ++count;
    }
    pinMode(rowPins[row],INPUT_PULLUP);
  }
  return count==1 ? found : (count ? static_cast<char>(0x7F) : 0);
}
void readInputs() {
  for (uint8_t i=0;i<10;++i) buttons[i].update(digitalRead(36+i)==LOW);
  keypad.update(static_cast<uint8_t>(scanKeypad()));
}
void clearSession() { activeVoter=-1; selectedCandidate=-1; biometricPassed=false; inputLength=0; inputBuffer[0]=0; }
void showReady() {
  digitalWrite(greenPin,LOW); digitalWrite(yellowPin,LOW);
  if (record.poll==Poll::FINAL_CLOSED) {
    state=State::RESULTS; message(F("FINAL RESULTS"));
    lcd.setCursor(0,1); lcd.print(F("A:")); lcd.print(record.votes[0]); lcd.print(F(" B:")); lcd.print(record.votes[1]);
    lcd.setCursor(0,2); lcd.print(F("C:")); lcd.print(record.votes[2]); lcd.print(F(" D:")); lcd.print(record.votes[3]);
    lcd.setCursor(0,3); lcd.print(F("Total:")); lcd.print(record.served);
  } else if (record.poll==Poll::OPEN) {
    state=State::VOTER_IDENTIFICATION; message(F("POLL OPEN - VOTER ID"),F("Enter 4 digits then #"));
  } else { state=State::POLL_CLOSED; message(F("POLL CLOSED"),F("Officer: Open Poll")); }
}
void finishSession(const __FlashStringHelper *event, const __FlashStringHelper *text, bool success=false) {
  clearSession(); audit(event); message(text); state=State::VOTER_IDENTIFICATION;
  feedbackActive=true; feedbackStartedAt=millis(); digitalWrite(greenPin,success ? HIGH : LOW);
  digitalWrite(yellowPin,LOW);
}
void renderOfficerPin() {
  lcd.setCursor(0,1); lcd.print(F("PIN: "));
  for (uint8_t i=0;i<4;++i) lcd.print(i<inputLength && i<4 ? inputBuffer[i] : '_');
}
void officerAuthentication(bool opening) {
  clearSession(); authenticatingForOpen=opening; state=State::OFFICER_AUTHENTICATION;
  sessionStartedAt=millis(); message(opening ? F("OPEN POLL PIN") : F("CLOSE POLL PIN"));
  renderOfficerPin();
}
void appendDigit(char key) {
  if (key=='*') { inputLength=0; inputBuffer[0]=0; }
  else if (key>='0' && key<='9') {
    if (inputLength==0 && state==State::VOTER_IDENTIFICATION) sessionStartedAt=millis();
    if (inputLength<4) { inputBuffer[inputLength++]=key; inputBuffer[inputLength]=0; }
    else { inputLength=5; } // Overflow is rejected, never truncate an ID or PIN.
  }
  if (state==State::OFFICER_AUTHENTICATION) renderOfficerPin();
}
void handleOfficerKey(char key) {
  if (key!='#') { appendDigit(key); return; }
  if (inputLength==4 && strcmp(inputBuffer,"5580")==0) {
    record.attempts=0; record.poll=authenticatingForOpen ? Poll::OPEN : Poll::FINAL_CLOSED;
    if (!saveRecord()) return;
    audit(authenticatingForOpen ? F("POLL_OPENED") : F("POLL_CLOSED")); clearSession(); showReady();
  } else {
    ++record.attempts;
    if (record.attempts>=3) { lockout(F("THREE INVALID PINS")); return; }
    if (!saveRecord()) return;
    audit(F("OFFICER_AUTH_FAIL")); inputLength=0; inputBuffer[0]=0;
    message(F("INVALID PIN"),F("Try again, then #"));
  }
}
void acceptVote() {
  if (!storageHealthy || !validRecord()) { storageFault(); return; }
  if (digitalRead(tamperPin)==LOW) { lockout(F("TAMPER INPUT")); return; }
  if (state!=State::CONFIRMATION || record.locked || record.poll!=Poll::OPEN ||
      activeVoter<0 || activeVoter>=voterCount || !biometricPassed ||
      selectedCandidate<0 || selectedCandidate>=4 || record.served>=voterCount ||
      (record.voted & (1U<<activeVoter)) || !(record.accredited & (1U<<activeVoter))) return;
  record.voted |= 1U<<activeVoter; ++record.votes[selectedCandidate]; ++record.served;
  if (!saveRecord()) return;
  if (digitalRead(tamperPin)==LOW) { lockout(F("TAMPER INPUT")); return; }
  finishSession(F("BALLOT_ACCEPTED"),F("VOTE STORED"),true);
}
void setup() {
  state=State::BOOT; clearSession(); feedbackActive=false; storageHealthy=false;
  for (auto &button : buttons) button=Debounced{};
  keypad=Debounced{};
  Serial1.begin(9600); lcd.begin(20,4);
  for (uint8_t pin : rowPins) pinMode(pin,INPUT_PULLUP);
  for (uint8_t pin : colPins) pinMode(pin,INPUT_PULLUP);
  for (uint8_t pin=36;pin<=45;++pin) pinMode(pin,INPUT_PULLUP);
  for (uint8_t pin=46;pin<=49;++pin) { pinMode(pin,OUTPUT); digitalWrite(pin,LOW); }
  noTone(buzzerPin);
  if (!loadRecord()) return;
  if (record.locked || digitalRead(tamperPin)==LOW) { lockout(F("PERSISTENT/TAMPER")); return; }
  showReady(); audit(F("BOOT"));
}
void loop() {
  // Tamper is deliberately not debounced: assertion takes priority over UI actions.
  if (state==State::TAMPER_LOCKOUT) return;
  if (digitalRead(tamperPin)==LOW) { lockout(F("TAMPER INPUT")); return; }
  readInputs();
  char key=keypad.rose && keypad.stable!=0x7F ? static_cast<char>(keypad.stable) : 0;
  if (state==State::RESULTS) return;
  if (feedbackActive) {
    if (static_cast<uint32_t>(millis()-feedbackStartedAt)<feedbackMs) return;
    feedbackActive=false; showReady(); return;
  }
  if (state==State::POLL_CLOSED) {
    if (buttons[7].rose) officerAuthentication(true);
    else if (key) { audit(F("POLL_CLOSED_REJECTION")); message(F("POLL CLOSED"),F("Officer must open")); }
    return;
  }
  bool timed=state!=State::VOTER_IDENTIFICATION || inputLength>0;
  if (timed && static_cast<uint32_t>(millis()-sessionStartedAt)>=sessionTimeoutMs) {
    clearSession(); audit(F("SESSION_TIMEOUT")); showReady(); return;
  }
  if (buttons[5].rose) { clearSession(); audit(F("SESSION_CANCELLED")); showReady(); return; }
  if (state==State::OFFICER_AUTHENTICATION) { if (key) handleOfficerKey(key); return; }
  if (state==State::VOTER_IDENTIFICATION) {
    if (buttons[8].rose) { officerAuthentication(false); return; }
    if (key!='#') { if (key) appendDigit(key); return; }
    uint16_t id=0;
    if (inputLength==4) for (uint8_t i=0;i<4;++i) id=id*10+inputBuffer[i]-'0';
    if (id<1001 || id>1010) { finishSession(F("UNKNOWN_VOTER"),F("UNKNOWN VOTER")); return; }
    activeVoter=id-1001;
    if (record.voted & (1U<<activeVoter)) { finishSession(F("DUPLICATE_ATTEMPT"),F("ALREADY VOTED")); return; }
    state=State::BIOMETRIC_VERIFICATION; sessionStartedAt=millis(); message(F("BIOMETRIC"),F("Set match, then #")); return;
  }
  if (state==State::BIOMETRIC_VERIFICATION) {
    if (key!='#') return;
    if (!buttons[6].stable || !buttons[6].raw) { finishSession(F("BIOMETRIC_FAIL"),F("BIOMETRIC FAIL")); return; }
    biometricPassed=true; record.accredited |= 1U<<activeVoter;
    if (!saveRecord()) return;
    state=State::BALLOT_ENABLED; sessionStartedAt=millis(); selectionArmed=false;
    message(F("SELECT ONE"),F("Release, then A-D")); return;
  }
  if (state==State::BALLOT_ENABLED || state==State::CONFIRMATION) {
    uint8_t rawMask=0, stableMask=0;
    for (uint8_t i=0;i<4;++i) { if (buttons[i].raw) rawMask|=1<<i; if (buttons[i].stable) stableMask|=1<<i; }
    if (bitCount(rawMask)>1 || bitCount(stableMask)>1) {
      bool report=selectionArmed || state==State::CONFIRMATION;
      selectedCandidate=-1; state=State::BALLOT_ENABLED; selectionArmed=false; confirmArmed=false;
      digitalWrite(yellowPin,HIGH);
      if (report) { audit(F("MULTIPLE_SELECTION")); message(F("MULTIPLE REJECTED"),F("Release all, retry")); }
      return;
    }
    if (rawMask==0 && stableMask==0) selectionArmed=true;
    if (selectionArmed && rawMask==stableMask && bitCount(stableMask)==1) {
      for (uint8_t i=0;i<4;++i) if (buttons[i].rose) {
        selectedCandidate=i; state=State::CONFIRMATION; confirmArmed=false;
        digitalWrite(yellowPin,LOW); message(F("CONFIRM BALLOT?"),F("Confirm or Cancel"));
        lcd.setCursor(0,2); lcd.print(F("Candidate ")); lcd.print(static_cast<char>('A'+i));
      }
    }
    if (state==State::CONFIRMATION) {
      // Require a fresh confirmation after selection, and no unsettled candidate input.
      if (!buttons[4].raw && !buttons[4].stable) confirmArmed=true;
      if (confirmArmed && buttons[4].rose && rawMask==stableMask) acceptVote();
    }
  }
}
