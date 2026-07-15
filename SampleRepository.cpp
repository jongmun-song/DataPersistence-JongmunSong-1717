#include "SampleRepository.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace DataPersistence
{
    SampleRepository::SampleRepository(std::filesystem::path jsonPath)
        : jsonPath_(std::move(jsonPath))
    {
    }

    void SampleRepository::load()
    {
        if (!std::filesystem::exists(jsonPath_))
        {
            sampleList_.clear();
            return;
        }

        std::ifstream input(jsonPath_);
        if (!input.is_open())
        {
            sampleList_.clear();
            return;
        }

        // Parse errors (nlohmann::json::parse_error) propagate to the caller.
        const nlohmann::json json = nlohmann::json::parse(input);
        sampleList_ = json.get<std::vector<Model::Sample>>();
    }

    void SampleRepository::save() const
    {
        const nlohmann::json json = sampleList_;

        std::filesystem::path tempPath = jsonPath_;
        tempPath += ".tmp";

        {
            std::ofstream output(tempPath, std::ios::trunc);
            if (!output.is_open())
            {
                throw std::runtime_error("Failed to open temporary file for writing: " + tempPath.string());
            }

            output << json.dump(4);
            if (!output)
            {
                throw std::runtime_error("Failed to write to temporary file: " + tempPath.string());
            }
        }

        std::error_code errorCode;
        std::filesystem::rename(tempPath, jsonPath_, errorCode);
        if (errorCode)
        {
            throw std::runtime_error("Failed to replace " + jsonPath_.string() + " with temporary file: " + errorCode.message());
        }
    }

    const std::vector<Model::Sample>& SampleRepository::all() const
    {
        return sampleList_;
    }
}
