#include "etapki/core/midi.h"

#include <algorithm>
#include <array>
#include <limits>

namespace etapki::core {
namespace {

constexpr std::size_t kHeaderSize = 14;

std::uint16_t readU16(const std::vector<Byte>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[offset]) << 8U) |
                                      static_cast<std::uint16_t>(bytes[offset + 1]));
}

std::uint32_t readU32(const std::vector<Byte>& bytes, std::size_t offset) {
    return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 8U) |
           static_cast<std::uint32_t>(bytes[offset + 3]);
}

bool hasBytes(const std::vector<Byte>& bytes, std::size_t offset, std::size_t count) {
    return offset <= bytes.size() && count <= bytes.size() - offset;
}

bool isChannelStatus(Byte status) { return status >= 0x80U && status <= 0xEFU; }

std::size_t channelPayloadSize(Byte status) {
    const Byte command = static_cast<Byte>(status & 0xF0U);
    return (command == 0xC0U || command == 0xD0U) ? 1U : 2U;
}

std::size_t systemPayloadSize(Byte status) {
    switch (status) {
    case 0xF1U:
    case 0xF3U: return 1U;
    case 0xF2U: return 2U;
    case 0xF6U: return 0U;
    default: return 0U;
    }
}

bool readVlq(const std::vector<Byte>& bytes, std::size_t& offset, Tick& value, MidiError& error) {
    value = 0;
    for (std::size_t count = 0; count < 4; ++count) {
        if (!hasBytes(bytes, offset, 1)) {
            error = {"Truncated variable-length quantity", offset};
            return false;
        }
        const Byte current = bytes[offset++];
        if (value > (std::numeric_limits<Tick>::max() >> 7U)) {
            error = {"Variable-length quantity overflow", offset - 1};
            return false;
        }
        value = (value << 7U) | static_cast<Tick>(current & 0x7FU);
        if ((current & 0x80U) == 0U) return true;
    }
    error = {"Variable-length quantity exceeds four bytes", offset};
    return false;
}

bool equalTag(const std::vector<Byte>& bytes, std::size_t offset, const std::array<Byte, 4>& tag) {
    return hasBytes(bytes, offset, tag.size()) &&
           std::equal(tag.begin(), tag.end(), bytes.begin() + static_cast<std::ptrdiff_t>(offset));
}

bool parseTrack(const std::vector<Byte>& raw, std::vector<MidiEvent>& events, MidiError& error) {
    std::size_t offset = 0;
    Tick absoluteTicks = 0;
    Byte runningStatus = 0;
    bool sawEndOfTrack = false;

    while (offset < raw.size()) {
        const std::size_t eventOffset = offset;
        Tick delta = 0;
        if (!readVlq(raw, offset, delta, error)) return false;
        if (absoluteTicks > std::numeric_limits<Tick>::max() - delta) {
            error = {"Absolute tick overflow", eventOffset};
            return false;
        }
        absoluteTicks += delta;

        if (!hasBytes(raw, offset, 1)) {
            error = {"Missing MIDI event status", offset};
            return false;
        }

        Byte status = raw[offset++];
        bool usedRunningStatus = false;
        if (status < 0x80U) {
            if (!isChannelStatus(runningStatus)) {
                error = {"Data byte encountered without running status", offset - 1};
                return false;
            }
            status = runningStatus;
            --offset;
            usedRunningStatus = true;
        }

        MidiEvent event{delta, absoluteTicks, status, {}, eventOffset, false};
        if (status == 0xFFU) {
            if (!hasBytes(raw, offset, 1)) {
                error = {"Truncated meta-event type", offset};
                return false;
            }
            event.meta = true;
            event.payload.push_back(raw[offset++]);
            Tick length = 0;
            if (!readVlq(raw, offset, length, error)) return false;
            if (length > static_cast<Tick>(raw.size() - offset)) {
                error = {"Meta-event exceeds track bounds", offset};
                return false;
            }
            event.payload.insert(event.payload.end(), raw.begin() + static_cast<std::ptrdiff_t>(offset),
                                raw.begin() + static_cast<std::ptrdiff_t>(offset + static_cast<std::size_t>(length)));
            offset += static_cast<std::size_t>(length);
            runningStatus = 0;
            if (event.payload.front() == 0x2FU) sawEndOfTrack = true;
        } else if (status == 0xF0U || status == 0xF7U) {
            Tick length = 0;
            if (!readVlq(raw, offset, length, error)) return false;
            if (length > static_cast<Tick>(raw.size() - offset)) {
                error = {"System-exclusive event exceeds track bounds", offset};
                return false;
            }
            event.payload.insert(event.payload.end(), raw.begin() + static_cast<std::ptrdiff_t>(offset),
                                raw.begin() + static_cast<std::ptrdiff_t>(offset + static_cast<std::size_t>(length)));
            offset += static_cast<std::size_t>(length);
            runningStatus = 0;
        } else if (isChannelStatus(status)) {
            const std::size_t payloadSize = channelPayloadSize(status);
            const std::size_t payloadOffset = offset;
            if (usedRunningStatus) {
                if (!hasBytes(raw, payloadOffset, payloadSize)) {
                    error = {"Running-status event exceeds track bounds", payloadOffset};
                    return false;
                }
            } else if (!hasBytes(raw, offset, payloadSize)) {
                error = {"Channel event exceeds track bounds", offset};
                return false;
            }
            event.payload.insert(event.payload.end(), raw.begin() + static_cast<std::ptrdiff_t>(offset),
                                raw.begin() + static_cast<std::ptrdiff_t>(offset + payloadSize));
            offset += payloadSize;
            runningStatus = status;
        } else {
            const std::size_t payloadSize = systemPayloadSize(status);
            if (!hasBytes(raw, offset, payloadSize)) {
                error = {"System event exceeds track bounds", offset};
                return false;
            }
            event.payload.insert(event.payload.end(), raw.begin() + static_cast<std::ptrdiff_t>(offset),
                                raw.begin() + static_cast<std::ptrdiff_t>(offset + payloadSize));
            offset += payloadSize;
            if (status < 0xF8U) runningStatus = 0;
        }
        events.push_back(std::move(event));
    }

    if (!sawEndOfTrack) {
        error = {"Track does not contain an end-of-track meta-event", raw.size()};
        return false;
    }
    return true;
}

} // namespace

std::optional<MidiDocument> MidiDocument::parse(const std::vector<Byte>& bytes, MidiError& error) {
    if (bytes.size() < kHeaderSize) {
        error = {"MIDI file is shorter than the header", bytes.size()};
        return std::nullopt;
    }
    if (!equalTag(bytes, 0, {'M', 'T', 'h', 'd'})) {
        error = {"Missing MThd header", 0};
        return std::nullopt;
    }
    const std::uint32_t headerLength = readU32(bytes, 4);
    if (headerLength < 6U || headerLength > bytes.size() - 8U) {
        error = {"Invalid MIDI header length", 4};
        return std::nullopt;
    }

    MidiDocument document;
    document.sourceBytes_ = bytes;
    document.header_ = {readU16(bytes, 8), readU16(bytes, 10), readU16(bytes, 12)};
    if (document.header_.format > 2U || document.header_.trackCount == 0U) {
        error = {"Unsupported MIDI format or zero tracks", 8};
        return std::nullopt;
    }

    std::size_t offset = 8U + static_cast<std::size_t>(headerLength);
    document.tracks_.reserve(document.header_.trackCount);
    for (std::uint16_t trackIndex = 0; trackIndex < document.header_.trackCount; ++trackIndex) {
        if (!equalTag(bytes, offset, {'M', 'T', 'r', 'k'}) || !hasBytes(bytes, offset + 4U, 4U)) {
            error = {"Missing or truncated MTrk chunk", offset};
            return std::nullopt;
        }
        const std::uint32_t trackLength = readU32(bytes, offset + 4U);
        const std::size_t rawOffset = offset + 8U;
        if (trackLength > bytes.size() - rawOffset) {
            error = {"MTrk chunk exceeds file bounds", offset + 4U};
            return std::nullopt;
        }
        MidiTrack track;
        track.rawBytes.insert(track.rawBytes.end(), bytes.begin() + static_cast<std::ptrdiff_t>(rawOffset),
                              bytes.begin() + static_cast<std::ptrdiff_t>(rawOffset + trackLength));
        if (!parseTrack(track.rawBytes, track.events, error)) return std::nullopt;
        document.tracks_.push_back(std::move(track));
        offset = rawOffset + static_cast<std::size_t>(trackLength);
    }

    if (offset != bytes.size()) {
        error = {"Trailing bytes after declared MIDI tracks", offset};
        return std::nullopt;
    }
    return document;
}

std::size_t MidiDocument::eventCount() const noexcept {
    std::size_t count = 0;
    for (const auto& track : tracks_) count += track.events.size();
    return count;
}

std::vector<Byte> MidiDocument::serialize() const { return sourceBytes_; }

} // namespace etapki::core
