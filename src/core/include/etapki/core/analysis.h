#pragma once

#include "etapki/core/catalog.h"
#include "etapki/core/midi.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace etapki::core {

struct AnalyzedNote {
    std::size_t track{};
    Byte channel{};
    Byte pitch{};
    Byte velocity{};
    Tick start{};
    Tick end{};

    Tick duration() const noexcept { return end >= start ? end - start : 0; }
};

struct AnalysisFinding {
    std::string label;
    std::size_t track{};
    Byte channel{};
    Byte pitch{};
    Tick start{};
    Tick end{};
    double confidence{};
    std::string evidence;
};

struct AnalysisProgress {
    std::string label;
    std::size_t completed{};
    std::size_t total{};
    double fraction{};
};

struct AnalysisMethodStatus {
    std::string label;
    ImplementationState state;
    std::string methodId;
    std::string message;
};

struct AnalysisOptions {
    std::vector<std::string> selectedLabels;
    std::function<bool()> isCancelled;
    std::function<void(const AnalysisProgress&)> onProgress;
};

struct AnalysisReport {
    bool completed{};
    bool cancelled{};
    std::vector<AnalyzedNote> notes;
    std::vector<AnalysisFinding> findings;
    std::vector<AnalysisMethodStatus> methods;
};

std::vector<AnalyzedNote> extractNotes(const MidiDocument& document);
AnalysisReport analyzeMidi(const MidiDocument& document, const AnalysisOptions& options);

} // namespace etapki::core
