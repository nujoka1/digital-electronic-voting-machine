#include <iostream>
#include <stdexcept>
#include "../../arduino/DigitalVotingMachine/DigitalVotingMachine.ino"
#define CHECK(x) do { if(!(x)) throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+" " #x); } while(0)
void step(uint32_t ms=1) { nowMs+=ms;loop(); }
void settle() { step();step(debounceMs); }
void key(char k) { physicalKey=k;settle();physicalKey=0;settle(); }
void type(const char *s) { while(*s) key(*s++); }
void press(int pin) { levels[pin]=LOW;settle();levels[pin]=HIGH;settle(); }
void reboot() { levels.fill(HIGH);physicalKey=0;setup(); }
void fresh() { EEPROM.data.fill(0xFF);EEPROM.cutAfter=-1;EEPROM.writes=0;Serial1.log.clear();reboot(); }
void open() { press(openPin);type("5580#");CHECK(record.poll==Poll::OPEN);CHECK(state==State::VOTER_IDENTIFICATION); }
void identify(const char *id="1001#") { type(id);CHECK(state==State::BIOMETRIC_VERIFICATION); }
void ballot(const char *id="1001#") { identify(id);levels[biometricPin]=LOW;settle();key('#');CHECK(state==State::BALLOT_ENABLED);step(); }
void ready() { step(feedbackMs); }
void normal() {
 fresh();CHECK(state==State::POLL_CLOSED);type("1001#");CHECK(record.served==0);open();
 type("2001#");CHECK(record.served==0);ready();
 identify();key('#');CHECK(!biometricPassed);CHECK(record.accredited==0);ready();
 ballot();press(36);CHECK(state==State::CONFIRMATION);CHECK(record.served==0);press(confirmPin);
 CHECK(record.served==1 && record.votes[0]==1 && validRecord());ready();
 type("1001#");CHECK(record.served==1);ready();
 ballot("1002#");press(37);press(cancelPin);CHECK(record.served==1);CHECK(bitCount(record.accredited)==2);
 ballot("1002#");press(37);press(confirmPin);CHECK(record.served==2);ready();
 reboot();CHECK(record.poll==Poll::OPEN && record.served==2);press(closePin);type("5580#");
 CHECK(state==State::RESULTS);reboot();CHECK(state==State::RESULTS);press(openPin);type("5580#1003#");CHECK(record.served==2);
 CHECK(Serial1.log.find("1001")==std::string::npos);CHECK(Serial1.log.find("Candidate")==std::string::npos);
}
void pinsAndTamper() {
 fresh();press(openPin);type("0000#");CHECK(record.attempts==1);reboot();CHECK(record.attempts==1);
 press(openPin);type("0000#0000#");CHECK(state==State::TAMPER_LOCKOUT);reboot();CHECK(state==State::TAMPER_LOCKOUT);
 for (State s : {State::POLL_CLOSED,State::OFFICER_AUTHENTICATION,State::VOTER_IDENTIFICATION,
 State::BIOMETRIC_VERIFICATION,State::BALLOT_ENABLED,State::CONFIRMATION,State::RESULTS}) {
   fresh();state=s;levels[tamperPin]=LOW;step();CHECK(state==State::TAMPER_LOCKOUT);reboot();CHECK(record.locked);
 }
 fresh();levels[tamperPin]=LOW;setup();CHECK(state==State::TAMPER_LOCKOUT);
}
void selection() {
 fresh();open();ballot();
 levels[36]=LOW;step();levels[36]=HIGH;step(5);settle();CHECK(state==State::BALLOT_ENABLED);
 levels[36]=levels[37]=LOW;settle();press(confirmPin);CHECK(record.served==0);
 levels[37]=HIGH;settle();CHECK(state==State::BALLOT_ENABLED);levels[36]=HIGH;settle();
 press(36);CHECK(state==State::CONFIRMATION);
 levels[36]=levels[37]=LOW;step();CHECK(state==State::BALLOT_ENABLED);press(confirmPin);CHECK(record.served==0);
 levels[36]=levels[37]=HIGH;settle();press(38);press(confirmPin);CHECK(record.votes[2]==1);ready();
 ballot("1002#");levels[confirmPin]=LOW;settle();press(36);CHECK(record.served==1);
 levels[confirmPin]=HIGH;settle();press(confirmPin);CHECK(record.served==2);ready();
 ballot("1003#");press(36);step(sessionTimeoutMs);press(confirmPin);CHECK(record.served==2);
}
void entryAndHeldInputs() {
 fresh();press(openPin);type("55800#");CHECK(record.attempts==1 && record.poll==Poll::NOT_OPENED);
 press(cancelPin);CHECK(state==State::POLL_CLOSED);press(openPin);step(sessionTimeoutMs);
 CHECK(state==State::POLL_CLOSED);open();type("10010#");CHECK(record.served==0);ready();
 key('1');step(sessionTimeoutMs);CHECK(inputLength==0 && state==State::VOTER_IDENTIFICATION);
 identify();step(sessionTimeoutMs);CHECK(state==State::VOTER_IDENTIFICATION && record.accredited==0);
 identify();levels[36]=LOW;levels[biometricPin]=LOW;settle();key('#');press(confirmPin);
 CHECK(state==State::BALLOT_ENABLED && record.served==0);
 levels[36]=HIGH;settle();levels[36]=levels[confirmPin]=LOW;settle();CHECK(record.served==0);
 levels[36]=levels[confirmPin]=HIGH;settle();press(confirmPin);CHECK(record.served==1);ready();
 press(closePin);press(cancelPin);CHECK(record.poll==Poll::OPEN);
}
void tenVoters() {
 fresh();open();
 for(int i=0;i<10;++i) { std::string id=std::to_string(1001+i)+"#";ballot(id.c_str());press(36+i%4);press(confirmPin);CHECK(validRecord());ready(); }
 CHECK(record.served==10 && record.accredited==voterMask && record.voted==voterMask);
 type("1011#");ready();type("1001#");CHECK(record.served==10);
}
void storage() {
 fresh();open();ballot();press(36);
 auto baseline=EEPROM.data;
 // Interrupt each possible changed-byte write of an accepted ballot.
 acceptVote();int totalWrites=0;
 EEPROM.data=baseline;reboot();state=State::CONFIRMATION;activeVoter=0;selectedCandidate=0;biometricPassed=true;
 EEPROM.writes=0;acceptVote();totalWrites=EEPROM.writes;CHECK(totalWrites>2);
 for(int cut=0;cut<=totalWrites;++cut) {
   EEPROM.data=baseline;EEPROM.cutAfter=-1;reboot();state=State::CONFIRMATION;activeVoter=0;selectedCandidate=0;biometricPassed=true;
   EEPROM.writes=0;EEPROM.cutAfter=cut;
   try { acceptVote(); } catch(const std::runtime_error&) {}
   EEPROM.cutAfter=-1;reboot();
   if(cut==0) CHECK(storageHealthy && record.served==0);
   else if(cut<totalWrites) CHECK(state==State::TAMPER_LOCKOUT && !storageHealthy);
   else CHECK(storageHealthy && record.served==1);
 }
 for(int i=0;i<recordSize;++i) {
   EEPROM.data=baseline;EEPROM.data[eepromAddress+i]^=1;auto corrupt=EEPROM.data;reboot();
   CHECK(state==State::TAMPER_LOCKOUT);CHECK(EEPROM.data==corrupt);
 }
 fresh();record.served=1;CHECK(!validRecord());CHECK(!saveRecord());
 fresh();record.votes[0]=1;uint8_t semantic[recordSize];encode(semantic);
 for(int i=0;i<recordSize;++i) EEPROM.data[eepromAddress+i]=semantic[i];
 auto invalidCounts=EEPROM.data;reboot();CHECK(!storageHealthy && EEPROM.data==invalidCounts);
 fresh();record.voted=1;CHECK(!validRecord());
 fresh();record.accredited=0x8000;CHECK(!validRecord());
 fresh();record.attempts=3;CHECK(!validRecord());
 fresh();EEPROM.data.fill(0xFF);EEPROM.data[eepromAddress]=0x80;auto legacy=EEPROM.data;reboot();CHECK(!storageHealthy && EEPROM.data==legacy);
 std::cout<<"Power-cut boundaries checked: "<<totalWrites+1<<"; corrupt record bytes checked: "<<int(recordSize)<<"\n";
}
int main() {
 try { normal();std::cout<<"PASS workflow, cancellation, duplicate, restart, final closure, audit privacy\n";
 pinsAndTamper();std::cout<<"PASS PIN persistence and tamper across states/restart\n";
 selection();std::cout<<"PASS bounce, simultaneous selection, fresh confirmation, timeout\n";
 entryAndHeldInputs();std::cout<<"PASS entry limits, stage timeouts, held inputs and authentication cancellation\n";
 tenVoters();std::cout<<"PASS ten-voter election and count invariants\n";
 storage();std::cout<<"PASS EEPROM interruptions, corruption, legacy preservation\n";
 } catch(const std::exception &e) { std::cerr<<"FAIL "<<e.what()<<"\n";return 1; }
}
