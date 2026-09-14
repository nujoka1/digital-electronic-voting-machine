from pathlib import Path

ROOT = Path(__file__).parents[1]

required = [
    ROOT / "arduino/DigitalVotingMachine/DigitalVotingMachine.ino",
    ROOT / "matlab/VotingMachine.m",
    ROOT / "matlab/run_tests.m",
    ROOT / "design/state-diagram.mmd",
    ROOT / "docs/verification-matrix.md",
]

for path in required:
    assert path.exists(), f"missing required source: {path}"
print("repository_structure: PASS")