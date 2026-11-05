# ETAPKI

ETAPKI is a C++ MIDI analysis and non-destructive embellishment workspace. It analyzes selected musical labels in a MIDI file, lets the user apply one or more of seven traceable embellishment types, previews the result, and exports a validated new MIDI file without altering the original.

## Seven embellishment types

All transformation variants remain traceable to exactly one of these types:

- Appoggiatura
- Compound appoggiatura
- Mordent
- Slide
- Snap
- Trill
- Turn

The current exact, case-sensitive source inventory is a baseline rather than a ceiling. Variant strings with different capitalization remain distinct until an explicit alias policy is approved:

| Type | Declared/selectable | Direct implementation branches |
|---|---:|---:|
| Appoggiatura | 160 | 160 |
| Compound appoggiatura | 106 | 120 direct branches; selection reconciliation pending |
| Mordent | 6 | 8 |
| Slide | 74 | 74 |
| Snap | 5 | 5 |
| Trill | 56 | 56 |
| Turn | 55 | 55 |

Additional variants have already been found outside this baseline and must be added through a typed registry with stable IDs, source traceability, selection rules, and regression tests.

## MIDI File Analyzer

The analyzer defines 62 analysis labels. Labels are separate from embellishment variants. Some labels are currently placeholders because their detection methods have not yet been implemented. The UI must show whether each label is implemented, placeholder, blocked, or deprecated; a placeholder must never report a positive match.

The intended workflow is:

1. Import a MIDI file.
2. Select one or more analyzer labels.
3. Analyze using only available detection methods.
4. Select one or more valid embellishment types.
5. Choose a variant manually or let the program choose randomly.
6. Preview before/after audio and revise the selection without restarting.
7. Export only after MIDI integrity gates pass.

## Non-destructive editing

The source MIDI remains unchanged and readable. Transformations are applied to an in-memory working model or a new revision. Users can audition, reject, compare, undo, and reselect transformations before explicitly exporting a new file.

Export validation includes parsing the input, validating the transformed model, serializing to a new path, reopening the output, and verifying MIDI structure, event bounds, timing, and expected changes.

## UI and audio direction

The planned interface combines:

- a modern analyzer dashboard for file import and label selection;
- a searchable seven-type/variant browser;
- a 2D timeline and optional interactive 3D note/analysis view;
- before/after playback, looping, target audition, undo, and re-selection;
- high-quality user-supplied or legally distributable sampled rendering;
- bypassable reverb/room ambience, pan, volume, and EQ controls;
- an accessible keyboard-friendly 2D fallback for every 3D interaction.

The audio renderer and effects chain are separate from MIDI transformation formulas so sound-library or preview changes do not alter vetted musical behavior.

## Repository and storage

Canonical repository: [GitHub](https://github.com/musicinnovator/ETAPKI)

Backup/mirror destination: [Google Drive folder](https://drive.google.com/drive/folders/1BtS-0Rgc7Te11W9CX-4XIKnaICUsDo2H)

- E: is the main working and packaging drive.
- D: is the local mirror/backup drive and uses FAT32.
- C: is not used for project files, source, builds, packages, caches, logs, or temporary work.

Before writing builds or packages, verify both external drives, available space, and FAT32 file-size limits. Never silently redirect output to C: and never delete previous builds automatically.

## Development principles

- Existing vetted formulas are frozen until explicitly approved for change.
- New work is additive and type-safe.
- Every variant and label is registered and traceable.
- Placeholder analysis labels are explicit and cannot pretend to be implemented.
- Errors are reported at user-facing boundaries.
- Compiler warnings, static analysis, unit tests, integration tests, MIDI round-trip tests, and sanitizers are part of normal validation.
- No packaging occurs before validation and explicit approval.

See [AGENTS.md](AGENTS.md) for the complete repository operating rules and [PROJECT_PROPOSAL.md](PROJECT_PROPOSAL.md) for the implementation roadmap and UI/UX proposal.

## Current implementation

The first additive core is in place:

- `data/variant_registry.json` records the exact case-sensitive source inventory;
- `data/analysis_label_registry.json` records all 62 labels, including 24 current placeholders;
- `src/core` provides a bounds-checked MIDI parser, byte-preserving serializer, integrity gate, revision plan, and explicit seven-type reference-only router;
- `reference/source` contains the imported C++ evidence unchanged, with `reference/source-manifest.json` for SHA-256 provenance;
- Launching `etapki_cli.exe` without arguments opens a persistent interactive session with `catalog`, `inspect <file.mid>`, `help`, and `quit`; the same commands are available as one-shot command-line arguments.

Build and test output belongs outside the repository on E:, for example:

```powershell
cmake -S E:\ETAPKI_GitHub -B E:\ETAPKIBuilds\etapki-core -G "MinGW Makefiles" -DETAPKI_BUILD_TESTS=ON
cmake --build E:\ETAPKIBuilds\etapki-core --parallel 2
ctest --test-dir E:\ETAPKIBuilds\etapki-core --output-on-failure
```

Use `tools/package-external.ps1` only after validation, with a required label and an E: build directory. It creates distinct, verified package outputs on E: and D:.
