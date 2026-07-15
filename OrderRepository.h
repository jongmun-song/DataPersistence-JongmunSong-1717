#pragma once

// JSON file-backed repository for Order (see docs/design/phase5-foundation.md,
// docs/feature/json-parsing.md, docs/feature/json-file-storage.md).
//
// Phase 5 scope: load/save/all only. Create/Read/Update/Delete are added in
// later phases (see docs/design/phase6-create.md and onward).

#include <filesystem>
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

    private:
        std::filesystem::path jsonPath_;
        std::vector<Model::Order> orderList_;
    };
}
