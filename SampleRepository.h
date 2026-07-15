#pragma once

// JSON file-backed repository for Sample (see docs/design/phase0-foundation.md,
// docs/feature/json-parsing.md, docs/feature/json-file-storage.md).
//
// Phase 0 scope: load/save/all only. Create/Read/Update/Delete-specific
// methods (create, findById, update, remove) are added in later phases.

#include <filesystem>
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

    private:
        std::filesystem::path jsonPath_;
        std::vector<Model::Sample> sampleList_;
    };
}
