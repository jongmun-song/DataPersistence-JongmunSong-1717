#pragma once

// JSON file-backed repository for ProductionQueueEntry (see
// docs/design/phase5-foundation.md, docs/feature/json-parsing.md,
// docs/feature/json-file-storage.md).
//
// Phase 5 scope: load/save/all only. Phase 6 adds create() (see
// docs/design/phase6-create.md). Read/Update/Delete are added in later
// phases.

#include <filesystem>
#include <vector>

#include "Model/ProductionQueueEntry.h"

namespace DataPersistence
{
    class ProductionQueueEntryRepository
    {
    public:
        explicit ProductionQueueEntryRepository(std::filesystem::path jsonPath);

        // Loads the entry list from jsonPath_ into memory.
        // If the file does not exist, entryList_ becomes an empty list.
        // If the file exists but fails to parse, the parse exception
        // (nlohmann::json::parse_error) propagates to the caller.
        void load();

        // Serializes entryList_ to jsonPath_ atomically: writes to a
        // temporary file first, then renames it over the original path.
        // Throws on failure, leaving the original file untouched.
        void save() const;

        const std::vector<Model::ProductionQueueEntry>& all() const;

        // Uses input.orderId as a natural key (no auto-assignment). Throws
        // std::invalid_argument if orderId <= 0 or an entry with the same
        // orderId already exists. Appends to the in-memory list and
        // persists via save(). If save() throws, the just-added entry is
        // rolled back from memory and the exception propagates to the
        // caller.
        // Returns the created ProductionQueueEntry.
        Model::ProductionQueueEntry create(const Model::ProductionQueueEntry& input);

    private:
        bool exists(int orderId) const;

        std::filesystem::path jsonPath_;
        std::vector<Model::ProductionQueueEntry> entryList_;
    };
}
