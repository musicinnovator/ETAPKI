#include "etapki/core/variant_registry.h"

#include "etapki/core/generated_variant_data.h"

#include <algorithm>
#include <random>

namespace etapki::core {

const std::vector<RegisteredVariant>& VariantRegistry::all() { return generatedVariantData(); }

std::optional<RegisteredVariant> VariantRegistry::findById(const std::string& id) {
    const auto found = std::find_if(all().begin(), all().end(), [&id](const auto& variant) {
        return variant.id == id;
    });
    if (found == all().end()) return std::nullopt;
    return *found;
}

std::optional<RegisteredVariant> VariantRegistry::findByName(EmbellishmentTypeId type,
                                                             const std::string& name) {
    const auto found = std::find_if(all().begin(), all().end(), [type, &name](const auto& variant) {
        return variant.type == type && (variant.displayName == name || variant.canonicalName == name);
    });
    if (found == all().end()) return std::nullopt;
    return *found;
}

std::vector<RegisteredVariant> VariantRegistry::forType(EmbellishmentTypeId type,
                                                         bool selectableOnly) {
    std::vector<RegisteredVariant> result;
    for (const auto& variant : all()) {
        if (variant.type == type && (!selectableOnly || variant.selectable)) result.push_back(variant);
    }
    return result;
}

std::optional<VariantSelection> VariantRegistry::chooseRandom(EmbellishmentTypeId type,
                                                               std::uint64_t seed) {
    std::vector<RegisteredVariant> candidates;
    for (const auto& variant : all()) {
        if (variant.type == type && variant.selectable && variant.supportsRandomSelection &&
            variant.state == ImplementationState::ReferenceOnly) {
            candidates.push_back(variant);
        }
    }
    if (candidates.empty()) return std::nullopt;
    std::mt19937_64 generator(seed);
    std::uniform_int_distribution<std::size_t> distribution(0, candidates.size() - 1);
    return VariantSelection{candidates[distribution(generator)], seed};
}

} // namespace etapki::core
