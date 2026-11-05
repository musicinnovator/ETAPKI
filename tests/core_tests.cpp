#include "etapki/core/catalog.h"
#include "etapki/core/analysis.h"
#include "etapki/core/integrity.h"
#include "etapki/core/midi.h"
#include "etapki/core/revision.h"
#include "etapki/core/transformation.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
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

std::vector<etapki::core::Byte> fixtureMidi(const std::vector<int>& pitches) {
    std::vector<etapki::core::Byte> track;
    for (const auto pitch : pitches) {
        track.insert(track.end(), {0, 0x90, static_cast<etapki::core::Byte>(pitch), 100,
                                   96, 0x80, static_cast<etapki::core::Byte>(pitch), 0});
    }
    track.insert(track.end(), {0, 0xFF, 0x2F, 0});
    const auto length = static_cast<std::uint32_t>(track.size());
    std::vector<etapki::core::Byte> result{
        'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 0, 0, 1, 0, 96,
        'M', 'T', 'r', 'k', static_cast<etapki::core::Byte>((length >> 24U) & 0xFFU),
        static_cast<etapki::core::Byte>((length >> 16U) & 0xFFU), static_cast<etapki::core::Byte>((length >> 8U) & 0xFFU),
        static_cast<etapki::core::Byte>(length & 0xFFU)
    };
    result.insert(result.end(), track.begin(), track.end());
    return result;
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

void testAnalysisMethodsAndCancellation() {
    using namespace etapki::core;
    const auto input = minimalMidi();
    MidiError parseError;
    const auto document = MidiDocument::parse(input, parseError);
    assert(document.has_value());

    std::size_t progressCalls = 0;
    AnalysisOptions options;
    options.selectedLabels = {"RN", "CS"};
    options.onProgress = [&progressCalls](const AnalysisProgress&) { ++progressCalls; };
    const auto report = analyzeMidi(*document, options);
    assert(report.completed);
    assert(!report.methods.empty());
    assert(report.methods[0].state == ImplementationState::Integrated);
    assert(report.methods[1].state == ImplementationState::Placeholder);
    assert(progressCalls > 0);

    AnalysisOptions cancelled;
    cancelled.selectedLabels = {"RN"};
    cancelled.isCancelled = [] { return true; };
    const auto cancelledReport = analyzeMidi(*document, cancelled);
    assert(cancelledReport.cancelled);
    assert(!cancelledReport.completed);
}

void testPhase2FixtureFindings() {
    using namespace etapki::core;
    const auto input = fixtureMidi({60, 62, 60, 72, 69, 65});
    MidiError parseError;
    const auto document = MidiDocument::parse(input, parseError);
    assert(document.has_value());
    AnalysisOptions options;
    options.selectedLabels = {"U2R", "I8", "LP", "I3", "BM", "MM", "EM", "DLP3", "DI", "QN"};
    const auto report = analyzeMidi(*document, options);
    assert(report.completed);
    for (const auto& label : options.selectedLabels) {
        assert(std::any_of(report.findings.begin(), report.findings.end(), [&label](const auto& finding) {
            return finding.label == label;
        }));
    }
}

} // namespace

int main() {
    testCatalog();
    testMidiRoundTrip();
    testMalformedMidiRejected();
    testRevisionIsNonDestructive();
    testSevenTypeRouterIsExplicit();
    testAnalysisMethodsAndCancellation();
    testPhase2FixtureFindings();
    std::cout << "ETAPKI core tests passed\n";
    return 0;
}
