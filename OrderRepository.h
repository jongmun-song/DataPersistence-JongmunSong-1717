#pragma once

// JSON file-backed repository for Order (see docs/design/phase5-foundation.md,
// docs/feature/json-parsing.md, docs/feature/json-file-storage.md).
//
// Phase 5 scope: load/save/all only. Phase 6 adds create() (see
// docs/design/phase6-create.md). Phase 7 adds findById() (see
// docs/design/phase7-read.md). Phase 8 adds update() (see
// docs/design/phase8-update.md). Delete is added in a later phase.

#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

#include "Model/Order.h"

namespace DataPersistence
{
    class OrderRepository
    {
    public:
        explicit OrderRepository(std::filesystem::path jsonPath);

        // Loads the order list from jsonPath_ into memory.
        // If the file does not exist, orderList_ becomes an empty list.
        // If the file exists but fails to parse, the parse exception
        // (nlohmann::json::parse_error) propagates to the caller.
        void load();

        // Serializes orderList_ to jsonPath_ atomically: writes to a
        // temporary file first, then renames it over the original path.
        // Throws on failure, leaving the original file untouched.
        void save() const;

        const std::vector<Model::Order>& all() const;

        // Assigns a fresh id (input.id is ignored), validates required
        // fields (throws std::invalid_argument on failure), defaults state
        // to OrderState::RESERVED, appends to the in-memory list, and
        // persists via save(). If save() throws, the just-added entry is
        // rolled back from memory and the exception propagates to the
        // caller.
        // Returns the created Order (with the assigned id).
        Model::Order create(const Model::Order& input);

        // Linear-searches orderList_ for an entry with the given id.
        // Returns std::nullopt if no such entry exists (not an exception -
        // "not found" is normal control flow, see docs/feature/read.md).
        std::optional<Model::Order> findById(int id) const;

        // Finds the entry with the given id and applies mutator to it.
        // Returns false if no such entry exists (not an exception - see
        // docs/feature/update.md). On success, backs up the current value
        // before applying mutator, then persists via save(). If save()
        // throws, the entry is rolled back to its backed-up value and the
        // exception propagates to the caller. Returns true on success.
        bool update(int id, const std::function<void(Model::Order&)>& mutator);

    private:
        int nextId() const;

        std::filesystem::path jsonPath_;
        std::vector<Model::Order> orderList_;
    };
}
