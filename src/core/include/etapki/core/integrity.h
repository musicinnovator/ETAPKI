#pragma once

#include "etapki/core/midi.h"

#include <string>
#include <vector>

namespace etapki::core {

struct IntegrityReport {
    bool valid{};
    bool sourceUnchanged{};
    bool outputReopens{};
    bool headerMatches{};
    bool trackCountMatches{};
    std::size_t inputEventCount{};
    std::size_t outputEventCount{};
    std::vector<std::string> messages;
};

IntegrityReport validateMidiExport(const MidiDocument& source,
                                   const std::vector<Byte>& outputBytes);

} // namespace etapki::core
