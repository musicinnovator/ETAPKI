#include "etapki/core/revision.h"
#include "etapki/core/transformation.h"

#include <algorithm>

namespace etapki::core {

bool MidiRevision::addOperation(TransformationOperation operation, RevisionError& error) {
    if (operation.operationId.empty()) {
        error = {"Transformation operation requires a stable operation ID"};
        return false;
    }
    if (operation.variantId.empty()) {
        error = {"Transformation operation requires a variant ID"};
        return false;
    }
    for (const auto index : operation.targetEventIndexes) {
        if (index >= source_.eventCount()) {
            error = {"Transformation target event index is outside the source model"};
            return false;
        }
    }
    const TransformationRouter router;
    const auto adapterResult = router.validate(operation, source_);
    if (!adapterResult.accepted) {
        error = {adapterResult.message};
        return false;
    }
    operations_.push_back(std::move(operation));
    return true;
}

bool MidiRevision::removeLastOperation() noexcept {
    if (operations_.empty()) return false;
    operations_.pop_back();
    return true;
}

void MidiRevision::clearOperations() noexcept { operations_.clear(); }

std::vector<Byte> MidiRevision::exportBytes(RevisionError& error) const {
    if (!operations_.empty()) {
        error = {"Transformation adapters are not integrated for this revision yet; export was refused"};
        return {};
    }
    return source_.serialize();
}

} // namespace etapki::core
