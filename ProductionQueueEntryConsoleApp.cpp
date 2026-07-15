#include "ProductionQueueEntryConsoleApp.h"

#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>

#include "Model/ProductionQueueEntry.h"

namespace DataPersistence
{
    namespace
    {
        // Thrown internally to unwind an input flow (handleCreate(), ...)
        // when the user cancels input mid-flow (see
        // docs/design/phase1-create.md, docs/design/phase6-create.md).
        struct InputCancelled
        {
        };

        std::string readLine(const std::string& prompt)
        {
            std::cout << prompt;
            std::string line;
            std::getline(std::cin, line);
            if (line == "q")
            {
                throw InputCancelled{};
            }
            return line;
        }

        // Parses `text` as a whole T value. Returns std::nullopt if `text`
        // is not a valid T (extra trailing characters count as invalid).
        template <typename T>
        std::optional<T> tryParseNumber(const std::string& text)
        {
            std::istringstream iss(text);
            T parsed{};
            iss >> parsed;
            if (iss && iss.eof())
            {
                return parsed;
            }
            return std::nullopt;
        }

        template <typename T>
        T readNumber(const std::string& fieldName)
        {
            while (true)
            {
                const std::string value = readLine(fieldName + " (취소: q): ");
                if (const std::optional<T> parsed = tryParseNumber<T>(value))
                {
                    return *parsed;
                }
                std::cout << fieldName + " 형식이 올바르지 않습니다. 다시 입력하세요.\n";
            }
        }

        Model::ProductionState readProductionState()
        {
            std::cout << "state 선택 (기본값: WAITING)\n";
            std::cout << "1. WAITING\n";
            std::cout << "2. PRODUCING\n";
            std::cout << "3. CONFIRMED\n";

            while (true)
            {
                const std::string value = readLine("선택 (빈 값: 기본값, 취소: q): ");
                if (value.empty())
                {
                    return Model::ProductionState::WAITING;
                }

                const std::optional<int> choice = tryParseNumber<int>(value);
                if (choice.has_value())
                {
                    switch (*choice)
                    {
                    case 1:
                        return Model::ProductionState::WAITING;
                    case 2:
                        return Model::ProductionState::PRODUCING;
                    case 3:
                        return Model::ProductionState::CONFIRMED;
                    default:
                        break;
                    }
                }
                std::cout << "선택이 올바르지 않습니다. 다시 입력하세요.\n";
            }
        }

        std::string productionStateToString(Model::ProductionState state)
        {
            switch (state)
            {
            case Model::ProductionState::WAITING:
                return "WAITING";
            case Model::ProductionState::PRODUCING:
                return "PRODUCING";
            case Model::ProductionState::CONFIRMED:
                return "CONFIRMED";
            }
            return "UNKNOWN";
        }

        void printProductionQueueEntry(const Model::ProductionQueueEntry& entry)
        {
            std::cout << "orderId: " << entry.orderId
                       << ", sampleId: " << entry.sampleId
                       << ", orderedQuantity: " << entry.orderedQuantity
                       << ", shortageQuantity: " << entry.shortageQuantity
                       << ", actualProductionQuantity: " << entry.actualProductionQuantity
                       << ", totalProductionTime: " << entry.totalProductionTime
                       << ", state: " << productionStateToString(entry.state) << '\n';
        }
    }

    ProductionQueueEntryConsoleApp::ProductionQueueEntryConsoleApp(ProductionQueueEntryRepository& repository)
        : repository_(repository)
    {
    }

    void ProductionQueueEntryConsoleApp::printMenu() const
    {
        std::cout << "\n=== ProductionQueueEntry 관리 ===\n";
        std::cout << "1. Create\n";
        std::cout << "2. 전체 목록 보기\n";
        std::cout << "3. orderId로 검색\n";
        std::cout << "4. Update\n";
        std::cout << "0. 뒤로가기\n";
        std::cout << "선택: ";
    }

    void ProductionQueueEntryConsoleApp::handleCreate()
    {
        Model::ProductionQueueEntry input;
        try
        {
            input.orderId = readNumber<int>("orderId");
            input.sampleId = readNumber<int>("sampleId");
            input.orderedQuantity = readNumber<int>("orderedQuantity");
            input.shortageQuantity = readNumber<int>("shortageQuantity");
            input.actualProductionQuantity = readNumber<int>("actualProductionQuantity");
            input.totalProductionTime = readNumber<double>("totalProductionTime");
            input.state = readProductionState();
        }
        catch (const InputCancelled&)
        {
            std::cout << "Create를 취소했습니다.\n";
            return;
        }

        try
        {
            const Model::ProductionQueueEntry created = repository_.create(input);
            std::cout << "생성되었습니다. orderId: " << created.orderId << '\n';
        }
        catch (const std::exception& e)
        {
            std::cout << "저장 중 오류가 발생했습니다: " << e.what() << '\n';
        }
    }

    void ProductionQueueEntryConsoleApp::handleReadAll() const
    {
        const std::vector<Model::ProductionQueueEntry>& entries = repository_.all();
        if (entries.empty())
        {
            std::cout << "등록된 생산 큐 엔트리가 없습니다.\n";
            return;
        }

        for (const Model::ProductionQueueEntry& entry : entries)
        {
            printProductionQueueEntry(entry);
        }
    }

    void ProductionQueueEntryConsoleApp::handleReadByOrderId() const
    {
        std::string value;
        try
        {
            value = readLine("검색할 orderId (취소: q): ");
        }
        catch (const InputCancelled&)
        {
            std::cout << "검색을 취소했습니다.\n";
            return;
        }

        const std::optional<int> orderId = tryParseNumber<int>(value);
        if (!orderId.has_value())
        {
            std::cout << "orderId 형식이 올바르지 않습니다.\n";
            return;
        }

        const std::optional<Model::ProductionQueueEntry> found = repository_.findByOrderId(*orderId);
        if (!found.has_value())
        {
            std::cout << "해당 orderId의 생산 큐 엔트리를 찾을 수 없습니다.\n";
            return;
        }

        printProductionQueueEntry(*found);
    }

    void ProductionQueueEntryConsoleApp::handleUpdate()
    {
        std::string orderIdText;
        try
        {
            orderIdText = readLine("수정할 orderId (취소: q): ");
        }
        catch (const InputCancelled&)
        {
            std::cout << "Update를 취소했습니다.\n";
            return;
        }

        const std::optional<int> orderId = tryParseNumber<int>(orderIdText);
        if (!orderId.has_value())
        {
            std::cout << "orderId 형식이 올바르지 않습니다.\n";
            return;
        }

        const std::optional<Model::ProductionQueueEntry> found = repository_.findByOrderId(*orderId);
        if (!found.has_value())
        {
            std::cout << "해당 orderId의 생산 큐 엔트리를 찾을 수 없습니다.\n";
            return;
        }

        std::cout << "현재 값: ";
        printProductionQueueEntry(*found);

        std::cout << "수정할 필드를 선택하세요.\n";
        std::cout << "1. shortageQuantity\n";
        std::cout << "2. actualProductionQuantity\n";
        std::cout << "3. totalProductionTime\n";
        std::cout << "4. state\n";

        std::string fieldChoiceText;
        try
        {
            fieldChoiceText = readLine("선택 (취소: q): ");
        }
        catch (const InputCancelled&)
        {
            std::cout << "Update를 취소했습니다.\n";
            return;
        }

        const std::optional<int> fieldChoice = tryParseNumber<int>(fieldChoiceText);
        if (!fieldChoice.has_value())
        {
            std::cout << "선택 형식이 올바르지 않습니다.\n";
            return;
        }

        std::function<void(Model::ProductionQueueEntry&)> mutator;
        try
        {
            switch (*fieldChoice)
            {
            case 1:
            {
                const int value = readNumber<int>("shortageQuantity");
                mutator = [value](Model::ProductionQueueEntry& entry) { entry.shortageQuantity = value; };
                break;
            }
            case 2:
            {
                const int value = readNumber<int>("actualProductionQuantity");
                mutator = [value](Model::ProductionQueueEntry& entry) { entry.actualProductionQuantity = value; };
                break;
            }
            case 3:
            {
                const double value = readNumber<double>("totalProductionTime");
                mutator = [value](Model::ProductionQueueEntry& entry) { entry.totalProductionTime = value; };
                break;
            }
            case 4:
            {
                const Model::ProductionState value = readProductionState();
                mutator = [value](Model::ProductionQueueEntry& entry) { entry.state = value; };
                break;
            }
            default:
                std::cout << "알 수 없는 필드입니다.\n";
                return;
            }
        }
        catch (const InputCancelled&)
        {
            std::cout << "Update를 취소했습니다.\n";
            return;
        }

        try
        {
            const bool updated = repository_.update(*orderId, mutator);
            if (updated)
            {
                std::cout << "수정되었습니다.\n";
            }
            else
            {
                std::cout << "해당 orderId의 생산 큐 엔트리를 찾을 수 없습니다.\n";
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "저장 중 오류가 발생했습니다: " << e.what() << '\n';
        }
    }

    void ProductionQueueEntryConsoleApp::run()
    {
        bool running = true;
        while (running)
        {
            printMenu();

            int choice = -1;
            if (!(std::cin >> choice))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "잘못된 입력입니다. 숫자를 입력하세요.\n";
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            switch (choice)
            {
            case 0:
                running = false;
                break;
            case 1:
                handleCreate();
                break;
            case 2:
                handleReadAll();
                break;
            case 3:
                handleReadByOrderId();
                break;
            case 4:
                handleUpdate();
                break;
            default:
                std::cout << "알 수 없는 메뉴입니다.\n";
                break;
            }
        }
    }
}
