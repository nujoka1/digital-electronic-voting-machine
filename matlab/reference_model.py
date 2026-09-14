class VotingMachine:
    def __init__(self):
        self.registered = list(range(1001, 1011))
        self.voted = set()
        self.counts = {candidate: 0 for candidate in "ABCD"}
        self.poll_open = False
        self.locked = False
        self.audit_log = []

    def open_poll(self, pin):
        if self.locked or pin != "5580":
            self.audit_log.append("OFFICER_AUTH_FAIL")
            return False
        self.poll_open = True
        self.audit_log.append("POLL_OPENED")
        return True

    def close_poll(self, pin):
        if self.locked or not self.poll_open or pin != "5580":
            return False
        self.poll_open = False
        self.audit_log.append("POLL_CLOSED")
        return True

    def cancel_ballot(self, voter_id, biometric):
        if not self.poll_open or voter_id not in self.registered or voter_id in self.voted or not biometric:
            return False
        self.audit_log.append("BALLOT_CANCELLED")
        return True

    def vote(self, voter_id, biometric, candidate, confirm, selections=1):
        if self.locked or not self.poll_open:
            self.audit_log.append("POLL_CLOSED_REJECTION")
            return False
        if voter_id not in self.registered:
            self.audit_log.append("UNKNOWN_VOTER")
            return False
        if voter_id in self.voted:
            self.audit_log.append("DUPLICATE_ATTEMPT")
            return False
        if not biometric:
            self.audit_log.append("BIOMETRIC_FAIL")
            return False
        if candidate not in self.counts or selections != 1:
            self.audit_log.append("INVALID_SELECTION")
            return False
        if not confirm:
            self.audit_log.append("BALLOT_CANCELLED")
            return False
        self.voted.add(voter_id)
        self.counts[candidate] += 1
        self.audit_log.append("BALLOT_ACCEPTED")
        return True

    def tamper(self):
        self.locked = True
        self.poll_open = False
        self.audit_log.append("LOCKOUT")

    def total_votes(self):
        return sum(self.counts.values())

    def results(self):
        return dict(self.counts) if not self.poll_open and not self.locked else None
