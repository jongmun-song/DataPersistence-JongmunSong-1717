#include "OrderRepository.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace DataPersistence
{
    OrderRepository::OrderRepository(std::filesystem::path jsonPath)
        : jsonPath_(std::move(jsonPath))
    {
    }

    void OrderRepository::load()
    {
        if (!std::filesystem::exists(jsonPath_))
        {
            orderList_.clear();
            return;
        }

        std::ifstream input(jsonPath_);
        if (!input.is_open())
        {
            throw std::runtime_error("Failed to open file for reading: " + jsonPath_.string());
        }

        // Parse errors (nlohmann::json::parse_error) propagate to the caller.
        const nlohmann::json json = nlohmann::json::parse(input);
        orderList_ = json.get<std::vector<Model::Order>>();
    }

    void OrderRepository::save() const
    {
        const nlohmann::json json = orderList_;

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

    const std::vector<Model::Order>& OrderRepository::all() const
    {
        return orderList_;
    }

    int OrderRepository::nextId() const
    {
        int maxId = 0;
        for (const Model::Order& order : orderList_)
        {
            if (order.id > maxId)
            {
                maxId = order.id;
            }
        }
        return maxId + 1;
    }

    Model::Order OrderRepository::create(const Model::Order& input)
    {
        if (input.customerName.empty())
        {
            throw std::invalid_argument("customerName은 비어 있을 수 없습니다.");
        }
        if (input.orderedQuantity <= 0)
        {
            throw std::invalid_argument("orderedQuantity는 0보다 커야 합니다.");
        }

        Model::Order created = input;
        created.id = nextId();

        orderList_.push_back(created);

        try
        {
            save();
        }
        catch (...)
        {
            orderList_.pop_back();
            throw;
        }

        return created;
    }

    std::optional<Model::Order> OrderRepository::findById(int id) const
    {
        for (const Model::Order& order : orderList_)
        {
            if (order.id == id)
            {
                return order;
            }
        }
        return std::nullopt;
    }

    bool OrderRepository::update(int id, const std::function<void(Model::Order&)>& mutator)
    {
        for (Model::Order& order : orderList_)
        {
            if (order.id == id)
            {
                const Model::Order backup = order;
                mutator(order);

                try
                {
                    save();
                }
                catch (...)
                {
                    order = backup;
                    throw;
                }

                return true;
            }
        }
        return false;
    }
}
