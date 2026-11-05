#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace etapki::core {

using Byte = std::uint8_t;
using Tick = std::uint64_t;

struct MidiError {
    std::string message;
    std::size_t offset{};
};

struct MidiHeader {
    std::uint16_t format{};
    std::uint16_t trackCount{};
    std::uint16_t division{};
};

struct MidiEvent {
    Tick deltaTicks{};
    Tick absoluteTicks{};
    Byte status{};
    std::vector<Byte> payload;
    std::size_t sourceOffset{};
    bool meta{};
};

struct MidiTrack {
    std::vector<Byte> rawBytes;
    std::vector<MidiEvent> events;
};

class MidiDocument {
public:
    static std::optional<MidiDocument> parse(const std::vector<Byte>& bytes, MidiError& error);

    const MidiHeader& header() const noexcept { return header_; }
    const std::vector<MidiTrack>& tracks() const noexcept { return tracks_; }
    const std::vector<Byte>& sourceBytes() const noexcept { return sourceBytes_; }
    std::size_t eventCount() const noexcept;

    // Unmodified documents serialize byte-for-byte. Future transformation
    // adapters will create a validated modified model before serialization.
    std::vector<Byte> serialize() const;

private:
    MidiHeader header_{};
    std::vector<MidiTrack> tracks_;
    std::vector<Byte> sourceBytes_;
};

} // namespace etapki::core
