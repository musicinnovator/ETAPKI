#include "etapki/core/analysis.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>

namespace etapki::core {
namespace {

struct ActiveNote {
    Tick start{};
    Byte velocity{};
};

using ActiveMap = std::map<std::pair<Byte, Byte>, std::vector<ActiveNote>>;

bool cancelled(const AnalysisOptions& options) {
    return options.isCancelled && options.isCancelled();
}

int interval(const AnalyzedNote& first, const AnalyzedNote& second) {
    return static_cast<int>(second.pitch) - static_cast<int>(first.pitch);
}

bool isDissonantSemitoneClass(int distance) {
    const int normalized = ((std::abs(distance) % 12) + 12) % 12;
    return normalized == 1 || normalized == 2 || normalized == 6 || normalized == 10 || normalized == 11;
}

std::vector<std::vector<AnalyzedNote>> byTrack(const std::vector<AnalyzedNote>& notes) {
    std::size_t trackCount = 0;
    for (const auto& note : notes) trackCount = std::max(trackCount, note.track + 1);
    std::vector<std::vector<AnalyzedNote>> tracks(trackCount);
    for (const auto& note : notes) tracks[note.track].push_back(note);
    for (auto& track : tracks) {
        std::sort(track.begin(), track.end(), [](const auto& left, const auto& right) {
            if (left.start != right.start) return left.start < right.start;
            if (left.pitch != right.pitch) return left.pitch < right.pitch;
            return left.channel < right.channel;
        });
    }
    return tracks;
}

} // namespace

std::vector<AnalyzedNote> extractNotes(const MidiDocument& document) {
    std::vector<AnalyzedNote> notes;
    for (std::size_t trackIndex = 0; trackIndex < document.tracks().size(); ++trackIndex) {
        ActiveMap active;
        for (const auto& event : document.tracks()[trackIndex].events) {
            const Byte command = static_cast<Byte>(event.status & 0xF0U);
            if ((command != 0x80U && command != 0x90U) || event.payload.size() < 2) continue;
            const Byte channel = static_cast<Byte>(event.status & 0x0FU);
            const Byte pitch = event.payload[0];
            const Byte velocity = event.payload[1];
            const auto key = std::make_pair(channel, pitch);
            const bool noteOn = command == 0x90U && velocity != 0U;
            if (noteOn) {
                active[key].push_back({event.absoluteTicks, velocity});
                continue;
            }
            auto found = active.find(key);
            if (found == active.end() || found->second.empty()) continue;
            const auto start = found->second.back();
            found->second.pop_back();
            if (event.absoluteTicks >= start.start) {
                notes.push_back({trackIndex, channel, pitch, start.velocity, start.start, event.absoluteTicks});
            }
        }
    }
    std::sort(notes.begin(), notes.end(), [](const auto& left, const auto& right) {
        if (left.track != right.track) return left.track < right.track;
        return left.start < right.start;
    });
    return notes;
}

AnalysisReport analyzeMidi(const MidiDocument& document, const AnalysisOptions& options) {
    AnalysisReport report;
    report.notes = extractNotes(document);
    const auto tracks = byTrack(report.notes);
    const auto& catalog = analysisLabels();
    const std::size_t total = std::max<std::size_t>(1, options.selectedLabels.size() * std::max<std::size_t>(1, report.notes.size()));
    std::size_t completed = 0;

    for (const auto& label : options.selectedLabels) {
        if (cancelled(options)) {
            report.cancelled = true;
            return report;
        }

        const auto descriptor = std::find_if(catalog.begin(), catalog.end(), [&label](const auto& item) {
            return item.id == label;
        });
        if (descriptor == catalog.end()) {
            report.methods.push_back({label, ImplementationState::Blocked, "unknown." + label, "Label is not present in the 62-label catalog."});
            continue;
        }
        if (!isIntegratedAnalysisLabel(label)) {
            report.methods.push_back({label, descriptor->state, descriptor->detectionMethodId,
                                      descriptor->state == ImplementationState::Placeholder
                                          ? "Placeholder: no detection method is available yet."
                                          : "Reference detection exists but is not integrated into the additive analyzer yet."});
            continue;
        }

        report.methods.push_back({label, ImplementationState::Integrated, "analysis." + label,
                                  "Integrated Phase 2 detection method."});
        for (const auto& track : tracks) {
            for (std::size_t index = 0; index < track.size(); ++index) {
                if (cancelled(options)) {
                    report.cancelled = true;
                    return report;
                }
                const auto& note = track[index];
                auto add = [&](double confidence, const std::string& evidence) {
                    report.findings.push_back({label, note.track, note.channel, note.pitch, note.start, note.end, confidence, evidence});
                };
                const AnalyzedNote* next = index + 1 < track.size() ? &track[index + 1] : nullptr;
                const AnalyzedNote* nextNext = index + 2 < track.size() ? &track[index + 2] : nullptr;

                if (label == "RN" && next != nullptr && note.pitch == next->pitch) add(1.0, "Adjacent notes have the same MIDI pitch.");
                else if (label == "U2R" && nextNext != nullptr && interval(note, *next) == 2 && interval(*next, *nextNext) == -2) add(0.98, "Two-semitone ascent returns by two semitones.");
                else if (label == "FN" && index + 1 == track.size()) add(1.0, "Final note in the track.");
                else if (label == "I8" && next != nullptr && std::abs(interval(note, *next)) == 12) add(1.0, "Adjacent notes are one octave apart.");
                else if (label == "LP" && next != nullptr && std::abs(interval(note, *next)) > 2) add(0.95, "Note immediately precedes a leap larger than a major second.");
                else if (label == "RLN" && note.duration() >= 2048) add(0.9, "Duration meets the reference long-note threshold of 2048 ticks.");
                else if (label == "QN" && note.duration() <= 256) add(0.9, "Duration meets the reference quick-note threshold of 256 ticks.");
                else if (label == "I3" && next != nullptr && (std::abs(interval(note, *next)) == 3 || std::abs(interval(note, *next)) == 4)) add(1.0, "Adjacent notes form a minor or major third.");
                else if (label == "BM" && index == 0) add(1.0, "First note in the track.");
                else if (label == "MM" && !track.empty() && index == track.size() / 2) add(1.0, "Middle note in the track.");
                else if (label == "EM" && index + 1 == track.size()) add(1.0, "Ending note in the track.");
                else if (label == "LAD" && next != nullptr && std::abs(interval(note, *next)) > 2) add(0.95, "Adjacent notes form an ascending or descending leap.");
                else if (label == "DLP3" && next != nullptr && (interval(note, *next) == -3 || interval(note, *next) == -4)) add(1.0, "Next note descends by a minor or major third.");
                else if (label == "DI" && next != nullptr && isDissonantSemitoneClass(interval(note, *next))) add(0.85, "Adjacent interval belongs to the configured dissonant semitone classes.");

                ++completed;
                if (options.onProgress) options.onProgress({label, completed, total, static_cast<double>(completed) / static_cast<double>(total)});
            }
        }
    }
    report.completed = !report.cancelled;
    if (options.onProgress) options.onProgress({"complete", total, total, 1.0});
    return report;
}

} // namespace etapki::core
