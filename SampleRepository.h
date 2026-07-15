#pragma once

// JSON file-backed repository for Sample (see docs/design/phase0-foundation.md,
// docs/feature/json-parsing.md, docs/feature/json-file-storage.md).
//
// Phase 0 scope: load/save/all only. Phase 1 adds create(). Phase 2 adds
// findById(). Phase 3 adds update(). Remaining Delete-specific methods
// (remove) are added in later phases.

#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

#include "Model/Sample.h"

namespace DataPersistence
{
    class SampleRepository
    {
    public:
        explicit SampleRepository(std::filesystem::path jsonPath);

        // Loads the sample list from jsonPath_ into memory.
        // If the file does not exist, sampleList_ becomes an empty list.
        // If the file exists but fails to parse, the parse exception
        // (nlohmann::json::parse_error) propagates to the caller.
        void load();

        // Serializes sampleList_ to jsonPath_ atomically: writes to a
        // temporary file first, then renames it over the original path.
        // Throws on failure, leaving the original file untouched.
        void save() const;

        const std::vector<Model::Sample>& all() const;

        // Assigns a fresh id (input.id is ignored), validates required
        // fields (throws std::invalid_argument on failure), appends to the
        // in-memory list, and persists via save(). If save() throws, the
        // just-added entry is rolled back from memory and the exception
        // propagates to the caller.
        // Returns the created Sample (with the assigned id).
        Model::Sample create(const Model::Sample& input);

        // Linear-searches sampleList_ for an entry with the given id.
        // Returns std::nullopt if no such entry exists (not an exception -
        // "not found" is normal control flow, see docs/feature/read.md).
        std::optional<Model::Sample> findById(int id) const;

        // Finds the entry with the given id and applies mutator to it.
        // Returns false if no such entry exists (not an exception - see
        // docs/feature/update.md). On success, backs up the current value
        // before applying mutator, then persists via save(). If save()
        // throws, the entry is rolled back to its backed-up value and the
        // exception propagates to the caller. Returns true on success.
        bool update(int id, const std::function<void(Model::Sample&)>& mutator);

    private:
        int nextId() const;

        std::filesystem::path jsonPath_;
        std::vector<Model::Sample> sampleList_;
    };
}
