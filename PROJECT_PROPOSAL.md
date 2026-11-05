# ETAPKI Comprehensive Project Proposal

## 1. Purpose

ETAPKI will become a non-destructive MIDI analysis, embellishment, and audition environment built in C++. The application will preserve existing vetted transformation formulas while adding a reliable analysis model, traceable variant management, a modern user interface, high-quality audio preview, effects, revision history, and integrity-gated export.

The proposal is intentionally additive. It does not authorize rewriting current formulas or treating a placeholder analyzer label as functional.

## 2. Product contract

Given a valid MIDI file, the user can select one or more analyzer labels, inspect detected targets, choose one or more of the seven embellishment types, select a specific variant or randomize within valid constraints, audition the result, reject or revise it, and export a new validated MIDI file. The original file is never overwritten.

The application must make every decision inspectable:

- which label detected a target;
- which detection method and confidence produced it;
- which embellishment type was selected;
- which exact variant and parameters were applied;
- which revision is currently being auditioned;
- what changed in the MIDI event model;
- whether the export passed integrity validation.

## 3. Proposed architecture

Use clear C++ module boundaries:

```text
UI / Presentation
    |
Session + Revision Controller
    |
Analysis Results ---- Transformation Planner ---- Variant Registry
    |                                      |
MIDI Parser/Model ---------------- Transformation Adapters
    |
Integrity Validator ---- MIDI Serializer
    |
Preview Scheduler ---- Instrument Renderer ---- Effects Chain
```

### 3.1 Domain model

Introduce strong domain types rather than passing strings throughout the application:

- `EmbellishmentTypeId`
- `VariantId`
- `AnalysisLabelId`
- `DetectionMethodId`
- `NoteId`, `TrackId`, `ChannelId`, `Tick`
- `RevisionId`
- `ConfidenceScore`

The seven types should be represented by a stable enum or registry key. Variants should be registered through metadata and resolved to an adapter that invokes the existing formula implementation. This creates traceability without modifying formula bodies.

### 3.2 Variant registry

The registry should support:

- canonical IDs and display names;
- historical aliases and whitespace normalization;
- type ownership;
- source file/function reference;
- direct, helper-backed, placeholder, or unavailable status;
- supported note/label constraints;
- deterministic and random selection;
- parameter schema and valid ranges;
- expected event-shape description;
- characterization-test reference.

The first registry import should preserve the current inventory exactly, including discrepancies. A validation report should identify declared variants without direct branches, implemented branches missing from selection pools, and strings that differ only by case or trailing whitespace.

## 4. Baseline inventory and reconciliation work

| Type | Declared/selectable | Direct branches | Required follow-up |
|---|---:|---:|---|
| Appoggiatura | 160 | 160 | Compare the main source and variant pool by canonical ID. |
| Compound appoggiatura | 106 | 120 direct | Reconcile 14 exact branch names that are not in the selectable generator pool. |
| Mordent | 6 | 8 | Decide whether `M` and `MR` should become selectable; preserve both implementations meanwhile. |
| Slide | 74 | 74 | Preserve all 74 exact, case-sensitive names and validate selection dispatch. |
| Snap | 5 | 5 direct | The fuller source includes a direct `SPvM3` branch; preserve it and test selection dispatch. |
| Trill | 56 | 56 | Remove trailing-space ambiguity at the registry boundary and retain aliases. |
| Turn | 55 | 55 | Preserve all current groupings and full variant names. |

The counts are not a license to delete or collapse variants. New source files may reveal additional variants; the registry version must increase and existing IDs must remain stable.

## 5. Analyzer plan

The analyzer should expose all 62 labels through a status-aware catalog. Each label receives a method contract:

```cpp
struct AnalysisMethodStatus {
    AnalysisLabelId label;
    DetectionMethodId method;
    ImplementationState state;
    std::string explanation;
};
```

Implemented methods should return structured findings, not only display strings:

```cpp
struct AnalysisFinding {
    AnalysisLabelId label;
    TrackId track;
    ChannelId channel;
    Tick start;
    Tick end;
    NoteId subject;
    ConfidenceScore confidence;
    std::string evidence;
};
```

Placeholder methods should return an explicit `NotImplemented` result. They should not use permissive fallbacks that create false positives. Each new detection method should be added with:

1. a musical definition;
2. supported MIDI assumptions;
3. boundary and ambiguity rules;
4. positive and negative fixtures;
5. a confidence policy;
6. a performance budget;
7. a regression test;
8. a user-facing explanation.

## 6. Non-destructive revision engine

Use an immutable source model plus revisions:

```text
Original MIDI bytes
      |
Parsed source model (read-only)
      |
Revision 0: analysis findings
      |
Revision 1..N: transformation operations
      |
Preview/export candidate
```

Each operation should record its source finding, type ID, variant ID, parameters, random seed if applicable, affected notes/events, and validation status. This allows undo, redo, before/after comparison, deterministic re-audition, and an audit trail without mutating the source.

## 7. UI/UX proposal

### 7.1 Primary workspace

Use a three-panel layout:

- **Source and analysis panel:** file import, file metadata, 62-label catalog, implemented/placeholder badges, and selected findings.
- **Transformation panel:** seven type cards, variant search/filter, compatible-variant status, user-selected/random mode, and parameter controls.
- **Timeline and audition panel:** note/event timeline, 2D score-like view, optional 3D scene, before/after controls, revision history, and export gate status.

### 7.2 3D interaction model

The 3D model should communicate musical relationships:

- horizontal axis: time/ticks;
- vertical axis: pitch;
- depth axis: track/channel or revision layer;
- colored markers: analyzer labels;
- shaped overlays: selected embellishment type;
- glow/intensity: confidence or selection state;
- ghosted source notes: original;
- solid transformed notes: current revision.

All 3D actions must have a 2D equivalent. A user must be able to select notes, labels, types, variants, and revisions without a 3D-capable GPU.

### 7.3 Interaction requirements

- drag-and-drop MIDI import;
- explicit source-file lock/read-only state;
- multi-label selection with search and grouping;
- finding list with jump-to-timeline;
- type compatibility filtering;
- exact variant selection and random selection with visible seed;
- instant preview re-render after a changed selection;
- play/pause/stop/loop and before/after A/B;
- undo/redo and revision comparison;
- keyboard shortcuts and screen-reader-friendly labels;
- clear error states with recovery actions;
- export preview showing output path, integrity checks, and file size.

## 8. High-quality preview and effects

The renderer should be a replaceable service. The project must not embed an unlicensed sound library or a low-quality placeholder synth as a final product sound. Support a user-configured or legally distributable high-quality sample source, such as an approved SoundFont/SFZ or sampled-instrument backend, with a documented license and installation path.

The preview chain should be:

```text
MIDI revision -> event scheduler -> instrument renderer -> mixer -> effects -> output device
```

Initial effects:

- stereo pan;
- volume and gain staging;
- parametric or at least three-band EQ;
- reverb with room-size, damping, mix, and pre-delay;
- room ambience or early-reflection control;
- bypass and reset for every effect;
- saved preview presets separate from MIDI transformations.

Realtime preview should be cancellable and revision-aware so an old render cannot replace a newer selection. Offline rendering may be used for higher quality, but the UI must remain responsive and show progress/cancellation.

## 9. MIDI integrity gate

Before export:

1. Confirm the source path and source checksum are unchanged.
2. Validate the parsed source model.
3. Apply the revision operations to a copy-on-write model.
4. Validate note ordering, non-negative durations, tick bounds, channels, and event relationships.
5. Serialize to a new E: output path.
6. Reopen the output with the project parser.
7. Compare header, track count, supported metadata, event bounds, and expected transformation deltas.
8. Run a second independent readability check when available.
9. Mark the output exportable only if every check passes.

Failures must preserve the original and the prior valid revision. Partially written output should be isolated and clearly marked as failed, never presented as a successful MIDI file.

## 10. Testing strategy

### Unit tests

- variant registry ownership and uniqueness;
- alias/case/trailing-space normalization;
- label status and placeholder behavior;
- tick and duration arithmetic;
- each adapter's formula invocation contract;
- revision undo/redo and deterministic random seeds;
- effects parameter bounds.

### Integration tests

- import, analyze, transform, preview, reject, reselect, and export;
- multiple labels and multiple transformation types;
- user-selected and random variants;
- format 0 and format 1 MIDI;
- tempo, program, controller, and meta events;
- malformed input rejection;
- cancellation of analysis and audio rendering;
- output reopen and structural comparison.

### Regression tests

Create characterization fixtures for every currently implemented variant before changing any formula. The tests should assert meaningful event properties and not rely only on exact byte equality when valid serializers are allowed to reorder equivalent data.

### Quality gates

- compiler warnings treated as errors for new modules;
- static analysis and formatting;
- unit/integration tests;
- sanitizer-enabled runs where supported;
- MIDI integrity gate;
- manual UI accessibility pass;
- manual before/after listening pass with the approved sound library;
- no C: artifacts;
- no package/export before all applicable gates pass.

## 11. Build, storage, and synchronization

Use E: for the canonical local working tree and build output. Use explicit E: paths for build, cache, test, preview, and export artifacts. Use D: for a non-destructive mirror/backup; because D: is FAT32, check free space and reject individual files above 4 GB.

The GitHub remote is the source repository. Commits should be focused, reviewed with `git diff --check`, and pushed to the current branch using `git push -u origin HEAD` after the user-authorized change passes validation. A corresponding backup snapshot should be uploaded to the specified Google Drive folder and/or mirrored to D: using the available authenticated tooling. Previous snapshots must remain recoverable.

If an external destination is unavailable, do not silently substitute C: or claim success. Report the blocked destination and keep the validated E: work intact.

## 12. Phased roadmap

### Phase 0: Baseline and inventory

- import the seven source modules and analyzer evidence;
- create the typed variant and label registries;
- reconcile direct branches, helper-backed variants, and selection mismatches;
- freeze characterization tests around existing formulas.

### Phase 1: Safe MIDI core

- implement robust parser/model boundaries;
- add non-destructive revisions;
- add serializer and integrity gate;
- add placeholder-aware analyzer API.

### Phase 2: Analysis coverage

- implement missing label methods individually;
- add fixtures and confidence explanations;
- expose progress, cancellation, and method status in the UI.

### Phase 3: Transformation integration

- add typed adapters for all seven types;
- support user-selected and deterministic random variants;
- preserve all existing formulas and add newly discovered variants through the registry.

### Phase 4: Preview and effects

- integrate the approved high-quality sound library boundary;
- add before/after playback, looping, target audition, cancellation, and effects;
- add preview presets separate from MIDI revisions.

### Phase 5: Modern UI

- implement the analyzer dashboard and variant browser;
- implement the 2D timeline;
- add the optional interactive 3D view and complete 2D fallback;
- add accessibility, keyboard controls, and clear error recovery.

### Phase 6: Release hardening

- run full validation and integrity gates;
- verify E:/D: storage policy;
- produce a user-facing report of variants, labels, tests, and known limitations;
- package only after explicit approval.

## 13. Immediate next actions

1. Confirm the seven full transformation source files and their provenance in the repository.
2. Generate a machine-readable variant registry without changing formula code.
3. Cross-reference all 106 compound-appoggiatura selectable entries against the 120 exact direct branches.
4. Resolve the mordent selection mismatch and document the 14 compound branch-only names.
5. Normalize trill lookup at the registry boundary while preserving historical aliases.
6. Produce the 62-label implementation-status matrix.
7. Add characterization tests before integrating the UI.
8. Decide the approved high-quality sound-library format and license.
9. Select the C++ UI/audio framework after a small playback and MIDI round-trip spike.
10. Keep every working, build, preview, and mirror artifact on E: or D:, never C:.
