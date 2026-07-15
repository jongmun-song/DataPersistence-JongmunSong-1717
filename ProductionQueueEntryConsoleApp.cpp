#include "ProductionQueueEntryConsoleApp.h"

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
    }

    ProductionQueueEntryConsoleApp::ProductionQueueEntryConsoleApp(ProductionQueueEntryRepository& repository)
        : repository_(repository)
    {
    }

    void ProductionQueueEntryConsoleApp::printMenu() const
    {
        std::cout << "\n=== ProductionQueueEntry 관리 ===\n";
        std::cout << "1. Create\n";
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
            default:
                std::cout << "알 수 없는 메뉴입니다.\n";
                break;
            }
        }
    }
}
