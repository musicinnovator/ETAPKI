#pragma once

#include "etapki/core/catalog.h"
#include "etapki/core/revision.h"

#include <memory>
#include <string>
#include <vector>

namespace etapki::core {

struct AdapterResult {
    bool accepted{};
    std::string message;
};

class TransformationAdapter {
public:
    virtual ~TransformationAdapter() = default;
    virtual EmbellishmentTypeId type() const noexcept = 0;
    virtual AdapterResult validate(const TransformationOperation& operation,
                                   const MidiDocument& source) const = 0;
};

class ReferenceOnlyTransformationAdapter final : public TransformationAdapter {
public:
    explicit ReferenceOnlyTransformationAdapter(EmbellishmentTypeId type) : type_(type) {}

    EmbellishmentTypeId type() const noexcept override { return type_; }
    AdapterResult validate(const TransformationOperation& operation,
                           const MidiDocument& source) const override;

private:
    EmbellishmentTypeId type_;
};

class TransformationRouter {
public:
    TransformationRouter();

    AdapterResult validate(const TransformationOperation& operation,
                           const MidiDocument& source) const;

private:
    std::vector<std::unique_ptr<TransformationAdapter>> adapters_;
};

} // namespace etapki::core
