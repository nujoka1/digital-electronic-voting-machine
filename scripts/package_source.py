#!/usr/bin/env python3
"""Package development sources; this does not certify a complete submission."""
from pathlib import Path
import zipfile

root = Path(__file__).resolve().parents[1]
output = root / 'COEN558_Digital_Voting_Machine_Source_Code.zip'
entries = ['arduino', 'firmware', 'matlab', 'design', 'docs', 'proteus', 'tests',
           'scripts', 'README.md', 'AGENTS.md', 'CHANGELOG.md', '.vscode', '.gitignore']
paths = []
for entry in entries:
    path = root / entry
    paths.extend([path] if path.is_file() else path.rglob('*'))
with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as archive:
    for path in sorted(paths):
        if not path.is_file() or any(part in {'__pycache__', '.pio', '.git'} for part in path.parts):
            continue
        if path.suffix in {'.pyc', '.tmp'}:
            continue
        archive.write(path, path.relative_to(root))
with zipfile.ZipFile(output) as archive:
    assert archive.testzip() is None
print(f'Created {output.name}; development source snapshot, not final submission.')
