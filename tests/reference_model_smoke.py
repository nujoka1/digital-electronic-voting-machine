from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "matlab"))
from reference_model import VotingMachine


def check(condition, message):
    if not condition:
        raise AssertionError(message)


machine = VotingMachine()
check(machine.open_poll("5580"), "valid opening")
check(not machine.vote(2001, True, "A", True), "unknown voter rejected")
check(not machine.vote(1001, False, "A", True), "biometric mismatch rejected")
check(machine.vote(1001, True, "A", True), "valid vote accepted")
check(not machine.vote(1001, True, "B", True), "duplicate rejected")
check(machine.cancel_ballot(1002, True), "cancel accepted")
check(machine.vote(1002, True, "B", True), "second valid vote accepted")
check(machine.total_votes() == 2, "count invariant")
check(machine.close_poll("5580"), "valid closure")
check(machine.results() == {"A": 1, "B": 1, "C": 0, "D": 0}, "results")
print("reference_model_smoke: PASS")