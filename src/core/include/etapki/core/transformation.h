#pragma once

#include "etapki/core/catalog.h"
#include "etapki/core/revision.h"
#include "etapki/core/variant_registry.h"

#include <memory>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace etapki::core {

struct AdapterResult {
    bool accepted{};
    bool executable{};
    std::string message;
};

class TransformationAdapter {
public:
    virtual ~TransformationAdapter() = default;
    virtual EmbellishmentTypeId type() const noexcept = 0;
    virtual AdapterResult validate(const TransformationOperation& operation,
                                   const MidiDocument& source) const = 0;
};

class RegistryBackedTransformationAdapter : public TransformationAdapter {
public:
    RegistryBackedTransformationAdapter(EmbellishmentTypeId type,
                                        std::string sourceFile,
                                        std::string functionName)
        : type_(type), sourceFile_(std::move(sourceFile)), functionName_(std::move(functionName)) {}

    EmbellishmentTypeId type() const noexcept override { return type_; }
    AdapterResult validate(const TransformationOperation& operation,
                           const MidiDocument& source) const override;

private:
    EmbellishmentTypeId type_;
    std::string sourceFile_;
    std::string functionName_;
};

class AppoggiaturaAdapter final : public RegistryBackedTransformationAdapter {
public:
    AppoggiaturaAdapter();
};
class CompoundAppoggiaturaAdapter final : public RegistryBackedTransformationAdapter {
public:
    CompoundAppoggiaturaAdapter();
};
class MordentAdapter final : public RegistryBackedTransformationAdapter {
public:
    MordentAdapter();
};
class SlideAdapter final : public RegistryBackedTransformationAdapter {
public:
    SlideAdapter();
};
class SnapAdapter final : public RegistryBackedTransformationAdapter {
public:
    SnapAdapter();
};
class TrillAdapter final : public RegistryBackedTransformationAdapter {
public:
    TrillAdapter();
};
class TurnAdapter final : public RegistryBackedTransformationAdapter {
public:
    TurnAdapter();
};

class TransformationRouter {
public:
    TransformationRouter();

    AdapterResult validate(const TransformationOperation& operation,
                           const MidiDocument& source) const;
    std::optional<VariantSelection> chooseRandom(EmbellishmentTypeId type,
                                                 std::uint64_t seed) const;

private:
    std::vector<std::unique_ptr<TransformationAdapter>> adapters_;
};

} // namespace etapki::core
