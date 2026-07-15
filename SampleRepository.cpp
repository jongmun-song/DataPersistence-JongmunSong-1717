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
            throw std::runtime_error("Failed to open file for reading: " + jsonPath_.string());
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

    int SampleRepository::nextId() const
    {
        int maxId = 0;
        for (const Model::Sample& sample : sampleList_)
        {
            if (sample.id > maxId)
            {
                maxId = sample.id;
            }
        }
        return maxId + 1;
    }

    Model::Sample SampleRepository::create(const Model::Sample& input)
    {
        if (input.name.empty())
        {
            throw std::invalid_argument("name은 비어 있을 수 없습니다.");
        }

        Model::Sample created = input;
        created.id = nextId();

        sampleList_.push_back(created);

        try
        {
            save();
        }
        catch (...)
        {
            sampleList_.pop_back();
            throw;
        }

        return created;
    }

    std::optional<Model::Sample> SampleRepository::findById(int id) const
    {
        for (const Model::Sample& sample : sampleList_)
        {
            if (sample.id == id)
            {
                return sample;
            }
        }
        return std::nullopt;
    }
}
