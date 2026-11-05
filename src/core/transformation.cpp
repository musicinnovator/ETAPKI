#include "etapki/core/transformation.h"

namespace etapki::core {

AdapterResult RegistryBackedTransformationAdapter::validate(const TransformationOperation& operation,
                                                            const MidiDocument& source) const {
    if (operation.type != type_) {
        return {false, false, "Transformation operation was routed to the wrong type adapter"};
    }
    if (operation.variantId.empty()) {
        return {false, false, "Transformation variant ID is required"};
    }
    for (const auto target : operation.targetEventIndexes) {
        if (target >= source.eventCount()) {
            return {false, false, "Transformation target is outside the source MIDI model"};
        }
    }
    const auto variant = VariantRegistry::findById(operation.variantId);
    if (!variant.has_value()) return {false, false, "Variant ID is not present in the machine-readable registry"};
    if (variant->type != type_) return {false, false, "Variant belongs to a different embellishment type"};
    if (!variant->selectable) return {false, false, "Variant is implemented in reference source but is not in the selectable pool"};
    if (variant->state == ImplementationState::Placeholder) {
        return {true, false, "Selection recorded, but this variant has no directly selectable formula branch"};
    }
    return {true, false, "Selection validated; formula bridge pending for " + sourceFile_ + "::" + functionName_};
}

TransformationRouter::TransformationRouter() {
    adapters_.push_back(std::make_unique<AppoggiaturaAdapter>());
    adapters_.push_back(std::make_unique<CompoundAppoggiaturaAdapter>());
    adapters_.push_back(std::make_unique<MordentAdapter>());
    adapters_.push_back(std::make_unique<SlideAdapter>());
    adapters_.push_back(std::make_unique<SnapAdapter>());
    adapters_.push_back(std::make_unique<TrillAdapter>());
    adapters_.push_back(std::make_unique<TurnAdapter>());
}

AdapterResult TransformationRouter::validate(const TransformationOperation& operation,
                                             const MidiDocument& source) const {
    for (const auto& adapter : adapters_) {
        if (adapter->type() == operation.type) return adapter->validate(operation, source);
    }
    return {false, false, "No adapter is registered for the requested embellishment type"};
}

std::optional<VariantSelection> TransformationRouter::chooseRandom(EmbellishmentTypeId type,
                                                                    std::uint64_t seed) const {
    const auto selection = VariantRegistry::chooseRandom(type, seed);
    if (!selection.has_value()) return std::nullopt;
    for (const auto& adapter : adapters_) {
        if (adapter->type() == type) return selection;
    }
    return std::nullopt;
}

AppoggiaturaAdapter::AppoggiaturaAdapter()
    : RegistryBackedTransformationAdapter(EmbellishmentTypeId::Appoggiatura,
                                          "NewEnhancedAppoggiaturaTransformationMultipleUserChoice.cpp",
                                          "applyAppoggiatura") {}
CompoundAppoggiaturaAdapter::CompoundAppoggiaturaAdapter()
    : RegistryBackedTransformationAdapter(EmbellishmentTypeId::CompoundAppoggiatura,
                                          "NewEnhancedCompoundAppoggiaturaTransformationMultipleUserChoice.cpp",
                                          "applyCompAppReg") {}
MordentAdapter::MordentAdapter()
    : RegistryBackedTransformationAdapter(EmbellishmentTypeId::Mordent,
                                          "NewEnhancedMordentTransformationMultipleUserChoice.cpp",
                                          "applyMordentVariants") {}
SlideAdapter::SlideAdapter()
    : RegistryBackedTransformationAdapter(EmbellishmentTypeId::Slide,
                                          "NewEnhancedSlidesTransformationMultipleUserChoice.cpp",
                                          "applySlideVariants") {}
SnapAdapter::SnapAdapter()
    : RegistryBackedTransformationAdapter(EmbellishmentTypeId::Snap,
                                          "NewEnhancedSnapTransformationMultipleUserChoice.cpp",
                                          "applySnapVariants") {}
TrillAdapter::TrillAdapter()
    : RegistryBackedTransformationAdapter(EmbellishmentTypeId::Trill,
                                          "NewEnhancedTrillTransformationMultipleUserChoice.cpp",
                                          "applyTrill") {}
TurnAdapter::TurnAdapter()
    : RegistryBackedTransformationAdapter(EmbellishmentTypeId::Turn,
                                          "NewEnhancedTurnsTransformationMultipleUserChoice.cpp",
                                          "applyTurnVariants") {}

} // namespace etapki::core
