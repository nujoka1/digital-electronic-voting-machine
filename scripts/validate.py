#!/usr/bin/env python3
"""Run real checks and retain commands, outputs and source fingerprints."""
import argparse
import hashlib
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--host', action='store_true')
parser.add_argument('--mega', action='store_true')
args = parser.parse_args()
if not args.host and not args.mega:
    args.host = args.mega = True

def run(command, log, env=None):
    result = subprocess.run(command, cwd=ROOT, text=True, capture_output=True, env=env)
    log.parent.mkdir(parents=True, exist_ok=True)
    log.write_text(f'UTC: {datetime.now(timezone.utc).isoformat()}\n$ {shlex.join(command)}\n'
                   f'{result.stdout}{result.stderr}\nExit: {result.returncode}\n')
    print(result.stdout + result.stderr, end='')
    result.check_returncode()

with tempfile.TemporaryDirectory(prefix='voting-validation-') as tmp:
    tmp = Path(tmp)
    if args.host:
        run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror',
             '-fsanitize=address,undefined', '-I', 'tests/host',
             'tests/host/test_firmware.cpp', '-o', str(tmp / 'test')],
            ROOT / 'evidence/host/compile.txt')
        env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')
        run([str(tmp / 'test')], ROOT / 'evidence/host/firmware-tests.txt', env)
        run(['python3', 'tests/reference_model_smoke.py'], ROOT / 'evidence/host/reference-smoke.txt')
        run(['python3', 'tests/test_requirements.py'], ROOT / 'evidence/host/structure.txt')
    if args.mega:
        run(['arduino-cli', 'compile', '--fqbn', 'arduino:avr:mega:cpu=atmega2560',
             '--build-path', str(tmp / 'mega'), '--output-dir', str(ROOT / 'build/arduino'),
             'arduino/DigitalVotingMachine'], ROOT / 'evidence/arduino/compile.txt')
        source = ROOT / 'build/arduino/DigitalVotingMachine.ino.hex'
        shutil.copy2(source, ROOT / 'evidence/arduino/DigitalVotingMachine.ino.hex')
        shutil.copy2(source, ROOT / 'proteus/firmware.hex')
def fingerprint(paths, destination):
    destination.write_text(''.join(
        f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.relative_to(ROOT)}\n' for p in paths))

sketch = ROOT / 'arduino/DigitalVotingMachine/DigitalVotingMachine.ino'
if args.host:
    fingerprint([sketch] + sorted((ROOT / 'tests/host').glob('*')),
                ROOT / 'evidence/host-sha256.txt')
if args.mega:
    fingerprint([sketch, ROOT / 'proteus/firmware.hex',
                 ROOT / 'evidence/arduino/DigitalVotingMachine.ino.hex'],
                ROOT / 'evidence/validation-sha256.txt')
    fingerprint([ROOT / 'evidence/arduino/DigitalVotingMachine.ino.hex'],
                ROOT / 'evidence/arduino/sha256.txt')
print('Requested checks passed. Hardware, Proteus, MATLAB and timing remain unverified.')
