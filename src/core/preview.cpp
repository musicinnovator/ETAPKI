#include "etapki/core/preview.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <numbers>

namespace etapki::core {
namespace {

float clamp(float value, float minimum, float maximum) {
    return std::max(minimum, std::min(maximum, value));
}

float decibelsToLinear(float decibels) {
    return std::pow(10.0F, decibels / 20.0F);
}

bool supportedSoundFile(const std::filesystem::path& path) {
    const auto extension = path.extension().wstring();
    return extension == L".sf2" || extension == L".sfz" || extension == L".wav" ||
           extension == L".flac" || extension == L".aif" || extension == L".aiff";
}

} // namespace

SoundLibraryValidation validateSoundLibrary(const SoundLibraryDescriptor& library) {
    if (library.id.empty() || library.displayName.empty()) return {false, "Sound library requires an ID and display name"};
    if (!library.approved || !library.highQuality) return {false, "Sound library must be explicitly approved and marked high-quality"};
    if (library.license.empty()) return {false, "Sound library license information is required"};
    if (library.path.empty()) return {false, "Sound library path is required"};
    const std::filesystem::path path(library.path);
    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) return {false, "Sound library file does not exist"};
    if (!supportedSoundFile(path)) return {false, "Sound library format is not supported by the approved backend boundary"};
    return {true, "Sound library passed approval, license, path, and format checks"};
}

const std::vector<EffectPreset>& effectPresets() {
    static const std::vector<EffectPreset> presets{
        {"dry", "Dry", {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F}},
        {"room", "Small room", {-1.0F, 0.0F, -1.0F, 0.5F, 1.0F, 0.18F, 0.25F}},
        {"hall", "Concert hall", {-2.0F, 0.0F, -2.0F, 0.0F, 2.0F, 0.32F, 0.72F}}
    };
    return presets;
}

bool EffectsChain::process(AudioBuffer& buffer,
                           const std::function<bool()>& isCancelled,
                           const std::function<void(double)>& onProgress) const {
    if (buffer.sampleRate == 0 || buffer.left.size() != buffer.right.size()) return false;
    const auto frameCount = buffer.frames();
    if (frameCount == 0) return true;
    const float gain = decibelsToLinear(clamp(settings_.gainDb, -60.0F, 12.0F));
    const float pan = clamp(settings_.pan, -1.0F, 1.0F);
    const float leftPan = std::cos((pan + 1.0F) * std::numbers::pi_v<float> / 4.0F);
    const float rightPan = std::sin((pan + 1.0F) * std::numbers::pi_v<float> / 4.0F);
    const float lowGain = decibelsToLinear(clamp(settings_.lowEqDb, -24.0F, 24.0F));
    const float midGain = decibelsToLinear(clamp(settings_.midEqDb, -24.0F, 24.0F));
    const float highGain = decibelsToLinear(clamp(settings_.highEqDb, -24.0F, 24.0F));
    const float room = clamp(settings_.roomSize, 0.0F, 1.0F);
    const float mix = clamp(settings_.reverbMix, 0.0F, 1.0F);
    const auto delayFrames = static_cast<std::size_t>(std::max(1.0F, static_cast<float>(buffer.sampleRate) * (0.025F + room * 0.15F)));
    std::vector<float> delayLeft(delayFrames, 0.0F);
    std::vector<float> delayRight(delayFrames, 0.0F);
    float lowLeft = 0.0F;
    float lowRight = 0.0F;
    const float lowPass = 0.08F + room * 0.12F;
    const float feedback = 0.18F + room * 0.38F;

    for (std::size_t index = 0; index < frameCount; ++index) {
        if (isCancelled && isCancelled()) return false;
        const float inputLeft = buffer.left[index] * gain * leftPan;
        const float inputRight = buffer.right[index] * gain * rightPan;
        lowLeft += lowPass * (inputLeft - lowLeft);
        lowRight += lowPass * (inputRight - lowRight);
        const float highLeft = inputLeft - lowLeft;
        const float highRight = inputRight - lowRight;
        const float processedLeft = lowLeft * lowGain + (inputLeft - lowLeft) * midGain * 0.65F + highLeft * highGain * 0.35F;
        const float processedRight = lowRight * lowGain + (inputRight - lowRight) * midGain * 0.65F + highRight * highGain * 0.35F;
        const std::size_t delayIndex = index % delayFrames;
        const float wetLeft = delayLeft[delayIndex];
        const float wetRight = delayRight[delayIndex];
        delayLeft[delayIndex] = processedLeft + wetLeft * feedback;
        delayRight[delayIndex] = processedRight + wetRight * feedback;
        buffer.left[index] = processedLeft * (1.0F - mix) + wetLeft * mix;
        buffer.right[index] = processedRight * (1.0F - mix) + wetRight * mix;
        if (onProgress && (index % 1024U == 0U || index + 1 == frameCount)) {
            onProgress(static_cast<double>(index + 1) / static_cast<double>(frameCount));
        }
    }
    return true;
}

PreviewResult ApprovedLibraryBackend::render(const PreviewRequest& request,
                                             const std::function<bool()>& isCancelled,
                                             const std::function<void(double)>& onProgress) {
    if (isCancelled && isCancelled()) return {false, true, {}, "Preview cancelled before rendering"};
    const auto validation = validateSoundLibrary(request.soundLibrary);
    if (!validation.usable) return {false, false, {}, validation.message};
    if (onProgress) onProgress(0.0);
    return {false, false, {}, "Approved library validated, but no concrete instrument backend is configured"};
}

PreviewResult PreviewController::render(const MidiDocument& source,
                                        const MidiRevision* revision,
                                        const SoundLibraryDescriptor& library,
                                        PreviewBackend& backend,
                                        EffectSettings effects,
                                        const std::function<bool()>& isCancelled,
                                        const std::function<void(double)>& onProgress) {
    PreviewRequest request{source, revision, mode_, loop_, targetEvents_, library};
    auto result = backend.render(request, isCancelled, onProgress);
    if (!result.rendered || result.cancelled) {
        playbackState_ = PlaybackState::Stopped;
        return result;
    }
    EffectsChain chain(effects);
    if (!chain.process(result.audio, isCancelled, onProgress)) {
        result.rendered = false;
        result.cancelled = isCancelled && isCancelled();
        result.message = result.cancelled ? "Preview cancelled during effects processing" : "Effects processing rejected the audio buffer";
        playbackState_ = PlaybackState::Stopped;
        return result;
    }
    lastAudio_ = result.audio;
    playbackState_ = PlaybackState::Stopped;
    return result;
}

bool PreviewController::play() noexcept {
    if (!lastAudio_.has_value() || lastAudio_->frames() == 0) return false;
    playbackState_ = PlaybackState::Playing;
    return true;
}

void PreviewController::pause() noexcept {
    if (playbackState_ == PlaybackState::Playing) playbackState_ = PlaybackState::Paused;
}

void PreviewController::stop() noexcept { playbackState_ = PlaybackState::Stopped; }

} // namespace etapki::core
