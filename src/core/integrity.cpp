#include "etapki/core/integrity.h"

#include <algorithm>

namespace etapki::core {

IntegrityReport validateMidiExport(const MidiDocument& source,
                                   const std::vector<Byte>& outputBytes) {
    IntegrityReport report;
    report.inputEventCount = source.eventCount();
    report.sourceUnchanged = source.serialize() == source.sourceBytes();

    MidiError error;
    const auto output = MidiDocument::parse(outputBytes, error);
    report.outputReopens = output.has_value();
    if (!output.has_value()) {
        report.messages.push_back("Output did not reopen: " + error.message);
        report.valid = false;
        return report;
    }

    report.outputEventCount = output->eventCount();
    report.headerMatches = output->header().format == source.header().format &&
                           output->header().division == source.header().division;
    report.trackCountMatches = output->tracks().size() == source.tracks().size();
    report.valid = report.sourceUnchanged && report.outputReopens && report.headerMatches &&
                   report.trackCountMatches && report.inputEventCount == report.outputEventCount;
    if (!report.sourceUnchanged) report.messages.push_back("Source model changed during validation");
    if (!report.headerMatches) report.messages.push_back("MIDI header changed unexpectedly");
    if (!report.trackCountMatches) report.messages.push_back("MIDI track count changed unexpectedly");
    if (report.inputEventCount != report.outputEventCount) report.messages.push_back("Event count changed unexpectedly");
    return report;
}

} // namespace etapki::core
