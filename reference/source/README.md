# Imported source evidence

The C++ files in this directory were copied from the supplied `WorkingSourceFiles.zip` archive without formula edits. They are retained as provenance and characterization inputs while the new core wraps them through typed adapters.

The imported legacy files are intentionally not compiled into the new core target yet. Their original entry points and dependencies are not safe to combine until each formula is isolated, characterized, and assigned to one of the seven registered embellishment types.

Use `tools/create-reference-manifest.ps1` to regenerate the SHA-256 manifest after a deliberate source refresh. A manifest change requires review because it may indicate a formula change.
