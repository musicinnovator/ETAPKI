#pragma once

#include "etapki/core/midi.h"
#include "etapki/core/revision.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace etapki::core {

enum class PreviewMode {
    Before,
    After,
    Target
};

enum class PlaybackState {
    Stopped,
    Playing,
    Paused
};

struct SoundLibraryDescriptor {
    std::string id;
    std::string displayName;
    std::string path;
    std::string license;
    bool approved{};
    bool highQuality{};
};

struct SoundLibraryValidation {
    bool usable{};
    std::string message;
};

SoundLibraryValidation validateSoundLibrary(const SoundLibraryDescriptor& library);

struct AudioBuffer {
    std::uint32_t sampleRate{};
    std::vector<float> left;
    std::vector<float> right;

    std::size_t frames() const noexcept { return std::min(left.size(), right.size()); }
};

struct EffectSettings {
    float gainDb{};
    float pan{};
    float lowEqDb{};
    float midEqDb{};
    float highEqDb{};
    float reverbMix{};
    float roomSize{};
};

struct EffectPreset {
    std::string id;
    std::string displayName;
    EffectSettings settings;
};

const std::vector<EffectPreset>& effectPresets();

class EffectsChain {
public:
    explicit EffectsChain(EffectSettings settings) : settings_(settings) {}

    bool process(AudioBuffer& buffer,
                 const std::function<bool()>& isCancelled = {},
                 const std::function<void(double)>& onProgress = {}) const;

private:
    EffectSettings settings_;
};

struct LoopRegion {
    Tick start{};
    Tick end{};
    bool enabled{};
};

struct PreviewRequest {
    const MidiDocument& source;
    const MidiRevision* revision{};
    PreviewMode mode{PreviewMode::Before};
    LoopRegion loop;
    std::vector<std::size_t> targetEventIndexes;
    SoundLibraryDescriptor soundLibrary;
};

struct PreviewResult {
    bool rendered{};
    bool cancelled{};
    AudioBuffer audio;
    std::string message;
};

class PreviewBackend {
public:
    virtual ~PreviewBackend() = default;
    virtual PreviewResult render(const PreviewRequest& request,
                                 const std::function<bool()>& isCancelled,
                                 const std::function<void(double)>& onProgress) = 0;
};

// This boundary intentionally refuses to synthesize audio. A real backend
// must be connected to an approved high-quality library before rendering.
class ApprovedLibraryBackend final : public PreviewBackend {
public:
    PreviewResult render(const PreviewRequest& request,
                         const std::function<bool()>& isCancelled,
                         const std::function<void(double)>& onProgress) override;
};

class PreviewController {
public:
    void setMode(PreviewMode mode) noexcept { mode_ = mode; }
    void setLoop(LoopRegion loop) noexcept { loop_ = loop; }
    void setTargetEvents(std::vector<std::size_t> targets) { targetEvents_ = std::move(targets); }

    PreviewMode mode() const noexcept { return mode_; }
    const LoopRegion& loop() const noexcept { return loop_; }
    PlaybackState playbackState() const noexcept { return playbackState_; }

    PreviewResult render(const MidiDocument& source,
                         const MidiRevision* revision,
                         const SoundLibraryDescriptor& library,
                         PreviewBackend& backend,
                         EffectSettings effects,
                         const std::function<bool()>& isCancelled = {},
                         const std::function<void(double)>& onProgress = {});

    bool play() noexcept;
    void pause() noexcept;
    void stop() noexcept;
    const std::optional<AudioBuffer>& lastAudio() const noexcept { return lastAudio_; }

private:
    PreviewMode mode_{PreviewMode::Before};
    LoopRegion loop_;
    std::vector<std::size_t> targetEvents_;
    PlaybackState playbackState_{PlaybackState::Stopped};
    std::optional<AudioBuffer> lastAudio_;
};

} // namespace etapki::core
