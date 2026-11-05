#pragma once

#include "etapki/core/catalog.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace etapki::core {

struct RegisteredVariant {
    EmbellishmentTypeId type;
    std::string id;
    std::string displayName;
    std::string canonicalName;
    ImplementationState state;
    bool selectable;
    bool supportsRandomSelection;
};

struct VariantSelection {
    RegisteredVariant variant;
    std::uint64_t seed{};
};

class VariantRegistry {
public:
    static const std::vector<RegisteredVariant>& all();
    static std::optional<RegisteredVariant> findById(const std::string& id);
    static std::optional<RegisteredVariant> findByName(EmbellishmentTypeId type,
                                                       const std::string& name);
    static std::vector<RegisteredVariant> forType(EmbellishmentTypeId type,
                                                  bool selectableOnly = false);
    static std::optional<VariantSelection> chooseRandom(EmbellishmentTypeId type,
                                                         std::uint64_t seed);
};

} // namespace etapki::core
