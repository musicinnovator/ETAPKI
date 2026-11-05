#include "etapki/core/transformation.h"

namespace etapki::core {

AdapterResult ReferenceOnlyTransformationAdapter::validate(const TransformationOperation& operation,
                                                            const MidiDocument& source) const {
    if (operation.type != type_) {
        return {false, "Transformation operation was routed to the wrong type adapter"};
    }
    if (operation.variantId.empty()) {
        return {false, "Transformation variant ID is required"};
    }
    for (const auto target : operation.targetEventIndexes) {
        if (target >= source.eventCount()) {
            return {false, "Transformation target is outside the source MIDI model"};
        }
    }
    return {false, "Formula adapter is reference-only until the legacy function is wrapped and characterized"};
}

TransformationRouter::TransformationRouter() {
    for (const auto& type : embellishmentTypes()) {
        adapters_.push_back(std::make_unique<ReferenceOnlyTransformationAdapter>(type.id));
    }
}

AdapterResult TransformationRouter::validate(const TransformationOperation& operation,
                                             const MidiDocument& source) const {
    for (const auto& adapter : adapters_) {
        if (adapter->type() == operation.type) return adapter->validate(operation, source);
    }
    return {false, "No adapter is registered for the requested embellishment type"};
}

} // namespace etapki::core
