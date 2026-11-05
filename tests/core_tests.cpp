#include "etapki/core/catalog.h"
#include "etapki/core/integrity.h"
#include "etapki/core/midi.h"
#include "etapki/core/revision.h"
#include "etapki/core/transformation.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::vector<etapki::core::Byte> minimalMidi() {
    return {
        'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 0, 0, 1, 0, 96,
        'M', 'T', 'r', 'k', 0, 0, 0, 12,
        0, 0x90, 60, 100,
        96, 0x80, 60, 0,
        0, 0xFF, 0x2F, 0
    };
}

void testCatalog() {
    using namespace etapki::core;
    assert(embellishmentTypes().size() == 7);
    assert(analysisLabels().size() == 62);
    assert(embellishmentTypes().front().declaredVariantCount == 160);
    assert(embellishmentTypes().back().declaredVariantCount == 55);
}

void testMidiRoundTrip() {
    using namespace etapki::core;
    const auto input = minimalMidi();
    MidiError error;
    const auto document = MidiDocument::parse(input, error);
    assert(document.has_value());
    assert(document->header().format == 0);
    assert(document->tracks().size() == 1);
    assert(document->eventCount() == 3);
    assert(document->serialize() == input);
    const auto report = validateMidiExport(*document, document->serialize());
    assert(report.valid);
}

void testMalformedMidiRejected() {
    using namespace etapki::core;
    auto input = minimalMidi();
    input.pop_back();
    MidiError error;
    assert(!MidiDocument::parse(input, error).has_value());
    assert(!error.message.empty());
}

void testRevisionIsNonDestructive() {
    using namespace etapki::core;
    const auto input = minimalMidi();
    MidiError parseError;
    const auto document = MidiDocument::parse(input, parseError);
    assert(document.has_value());
    MidiRevision revision(*document);
    RevisionError revisionError;
    assert(revision.addOperation({"op-1", EmbellishmentTypeId::Trill, "trill.example", {1}, 42}, revisionError));
    assert(document->serialize() == input);
    RevisionError exportError;
    assert(revision.exportBytes(exportError).empty());
    assert(!exportError.message.empty());
    assert(revision.removeLastOperation());
    assert(revision.exportBytes(exportError) == input);
}

void testSevenTypeRouterIsExplicit() {
    using namespace etapki::core;
    const auto input = minimalMidi();
    MidiError parseError;
    const auto document = MidiDocument::parse(input, parseError);
    assert(document.has_value());
    TransformationRouter router;
    for (const auto& type : embellishmentTypes()) {
        const auto result = router.validate({"router-test", type.id, "example", {1}, 7}, *document);
        assert(!result.accepted);
        assert(result.message.find("reference-only") != std::string::npos);
    }
}

} // namespace

int main() {
    testCatalog();
    testMidiRoundTrip();
    testMalformedMidiRejected();
    testRevisionIsNonDestructive();
    testSevenTypeRouterIsExplicit();
    std::cout << "ETAPKI core tests passed\n";
    return 0;
}
