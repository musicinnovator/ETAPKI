#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace etapki::core {

enum class EmbellishmentTypeId {
    Appoggiatura,
    CompoundAppoggiatura,
    Mordent,
    Slide,
    Snap,
    Trill,
    Turn
};

enum class ImplementationState {
    Integrated,
    ReferenceOnly,
    Placeholder,
    Blocked,
    Deprecated
};

struct VariantDescriptor {
    EmbellishmentTypeId type;
    std::string id;
    std::string displayName;
    ImplementationState state;
    bool selectable;
    bool supportsRandomSelection;
};

struct EmbellishmentTypeDescriptor {
    EmbellishmentTypeId id;
    std::string stableId;
    std::string displayName;
    std::size_t declaredVariantCount;
    std::size_t directImplementationBranchCount;
};

struct AnalysisLabelDescriptor {
    std::string id;
    ImplementationState state;
    std::string detectionMethodId;
    std::string explanation;
};

const std::vector<EmbellishmentTypeDescriptor>& embellishmentTypes();
const std::vector<VariantDescriptor>& baselineVariantDescriptors();
const std::vector<AnalysisLabelDescriptor>& analysisLabels();
bool isIntegratedAnalysisLabel(const std::string& label) noexcept;

const char* toString(EmbellishmentTypeId type) noexcept;
const char* toString(ImplementationState state) noexcept;

} // namespace etapki::core
