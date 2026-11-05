#include "etapki/core/catalog.h"

#include <algorithm>
#include <array>

namespace etapki::core {

const std::vector<EmbellishmentTypeDescriptor>& embellishmentTypes() {
    static const std::vector<EmbellishmentTypeDescriptor> types{
        {EmbellishmentTypeId::Appoggiatura, "appoggiatura", "Appoggiatura", 160, 160},
        {EmbellishmentTypeId::CompoundAppoggiatura, "compound-appoggiatura", "Compound appoggiatura", 106, 120},
        {EmbellishmentTypeId::Mordent, "mordent", "Mordent", 6, 8},
        {EmbellishmentTypeId::Slide, "slide", "Slide", 74, 74},
        {EmbellishmentTypeId::Snap, "snap", "Snap", 5, 5},
        {EmbellishmentTypeId::Trill, "trill", "Trill", 56, 56},
        {EmbellishmentTypeId::Turn, "turn", "Turn", 55, 55}
    };
    return types;
}

const std::vector<VariantDescriptor>& baselineVariantDescriptors() {
    static const std::vector<VariantDescriptor> variants{
        {EmbellishmentTypeId::Mordent, "mordent.M", "M", ImplementationState::ReferenceOnly, false, false},
        {EmbellishmentTypeId::Mordent, "mordent.MR", "MR", ImplementationState::ReferenceOnly, false, false},
        {EmbellishmentTypeId::Snap, "snap.SPvM3", "SPvM3", ImplementationState::ReferenceOnly, true, true}
    };
    return variants;
}

const std::vector<AnalysisLabelDescriptor>& analysisLabels() {
    static const std::array<const char*, 62> labels{
        "RN", "U2R", "FN", "I8", "LP", "RLN", "QN", "CS", "I3", "BM", "MM", "EM",
        "SPU", "SPD", "CH", "CW", "CD", "HT", "FM", "SYP", "SLP", "RDN", "LAD", "DN",
        "DNW", "SN", "LNSN", "SAN", "RTR2", "TNTDN", "SMP", "SPNSA", "DHT", "DBC", "DDN",
        "LNR", "LNSAS", "LNSAL", "DI", "NLN", "TNTLN", "TNTTN", "Mmd7", "MmAug6", "MmAug4",
        "Mmm3", "NSALP", "BNIR", "IPASC", "IPASV", "RTDS", "BRTD", "SPCM", "SPDM", "SSN",
        "SVN", "ANS", "ANL", "FTB", "CDB", "DLP3", "IALD"
    };

    static const std::array<const char*, 24> placeholders{
        "CS", "CH", "CW", "CD", "HT", "FM", "SYP", "SLP", "DNW", "SN", "DHT", "DBC",
        "Mmd7", "MmAug6", "MmAug4", "Mmm3", "SPCM", "SPDM", "SSN", "SVN", "ANS", "ANL", "FTB", "CDB"
    };

    static const std::array<const char*, 14> integrated{
        "RN", "U2R", "FN", "I8", "LP", "RLN", "QN", "I3", "BM", "MM", "EM", "LAD", "DLP3", "DI"
    };

    static const std::vector<AnalysisLabelDescriptor> descriptors = [] {
        std::vector<AnalysisLabelDescriptor> result;
        result.reserve(labels.size());
        for (const auto* label : labels) {
            const bool placeholder = std::find_if(placeholders.begin(), placeholders.end(),
                                                  [label](const auto* candidate) { return std::string(candidate) == label; }) != placeholders.end();
            const bool isIntegrated = std::find_if(integrated.begin(), integrated.end(),
                                                   [label](const auto* candidate) { return std::string(candidate) == label; }) != integrated.end();
            result.push_back({label,
                              placeholder ? ImplementationState::Placeholder : (isIntegrated ? ImplementationState::Integrated : ImplementationState::ReferenceOnly),
                              placeholder ? std::string("unimplemented.") + label : (isIntegrated ? std::string("analysis.") + label : std::string("reference.processLabels.") + label),
                              placeholder ? "Label is catalogued but no detection rule was found in the reference processLabels method."
                                          : (isIntegrated ? "Detection is implemented in the additive Phase 2 analyzer." : "Detection exists in the imported reference analyzer; integration is pending.")});
        }
        return result;
    }();
    return descriptors;
}

bool isIntegratedAnalysisLabel(const std::string& label) noexcept {
    static const std::array<const char*, 14> integrated{
        "RN", "U2R", "FN", "I8", "LP", "RLN", "QN", "I3", "BM", "MM", "EM", "LAD", "DLP3", "DI"
    };
    return std::find_if(integrated.begin(), integrated.end(), [&label](const auto* candidate) {
        return label == candidate;
    }) != integrated.end();
}

const char* toString(EmbellishmentTypeId type) noexcept {
    switch (type) {
    case EmbellishmentTypeId::Appoggiatura: return "appoggiatura";
    case EmbellishmentTypeId::CompoundAppoggiatura: return "compound-appoggiatura";
    case EmbellishmentTypeId::Mordent: return "mordent";
    case EmbellishmentTypeId::Slide: return "slide";
    case EmbellishmentTypeId::Snap: return "snap";
    case EmbellishmentTypeId::Trill: return "trill";
    case EmbellishmentTypeId::Turn: return "turn";
    }
    return "unknown";
}

const char* toString(ImplementationState state) noexcept {
    switch (state) {
    case ImplementationState::Integrated: return "integrated";
    case ImplementationState::ReferenceOnly: return "reference-only";
    case ImplementationState::Placeholder: return "placeholder";
    case ImplementationState::Blocked: return "blocked";
    case ImplementationState::Deprecated: return "deprecated";
    }
    return "unknown";
}

} // namespace etapki::core
