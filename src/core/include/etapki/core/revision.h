#pragma once

#include "etapki/core/catalog.h"
#include "etapki/core/midi.h"

#include <cstdint>
#include <string>
#include <vector>

namespace etapki::core {

struct TransformationOperation {
    std::string operationId;
    EmbellishmentTypeId type;
    std::string variantId;
    std::vector<std::size_t> targetEventIndexes;
    std::uint64_t randomSeed{};
};

struct RevisionError {
    std::string message;
};

class MidiRevision {
public:
    explicit MidiRevision(const MidiDocument& source) : source_(source) {}

    bool addOperation(TransformationOperation operation, RevisionError& error);
    bool removeLastOperation() noexcept;
    void clearOperations() noexcept;

    const MidiDocument& source() const noexcept { return source_; }
    const std::vector<TransformationOperation>& operations() const noexcept { return operations_; }

    // A non-empty plan is deliberately not exported until an adapter has
    // rendered it. This prevents placeholder behavior from changing a file.
    std::vector<Byte> exportBytes(RevisionError& error) const;

private:
    const MidiDocument& source_;
    std::vector<TransformationOperation> operations_;
};

} // namespace etapki::core
