#include "OrderConsoleApp.h"

#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>

#include "Model/Order.h"

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

        std::string readNonEmptyString(const std::string& fieldName)
        {
            while (true)
            {
                const std::string value = readLine(fieldName + " (취소: q): ");
                if (!value.empty())
                {
                    return value;
                }
                std::cout << fieldName + "은(는) 비어 있을 수 없습니다. 다시 입력하세요.\n";
            }
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

        Model::OrderState readOrderState()
        {
            std::cout << "state 선택 (기본값: RESERVED)\n";
            std::cout << "1. RESERVED\n";
            std::cout << "2. CONFIRMED\n";
            std::cout << "3. PRODUCING\n";
            std::cout << "4. RELEASE\n";
            std::cout << "5. REJECTED\n";

            while (true)
            {
                const std::string value = readLine("선택 (빈 값: 기본값, 취소: q): ");
                if (value.empty())
                {
                    return Model::OrderState::RESERVED;
                }

                const std::optional<int> choice = tryParseNumber<int>(value);
                if (choice.has_value())
                {
                    switch (*choice)
                    {
                    case 1:
                        return Model::OrderState::RESERVED;
                    case 2:
                        return Model::OrderState::CONFIRMED;
                    case 3:
                        return Model::OrderState::PRODUCING;
                    case 4:
                        return Model::OrderState::RELEASE;
                    case 5:
                        return Model::OrderState::REJECTED;
                    default:
                        break;
                    }
                }
                std::cout << "선택이 올바르지 않습니다. 다시 입력하세요.\n";
            }
        }
    }

    OrderConsoleApp::OrderConsoleApp(OrderRepository& repository)
        : repository_(repository)
    {
    }

    void OrderConsoleApp::printMenu() const
    {
        std::cout << "\n=== Order 관리 ===\n";
        std::cout << "1. Create\n";
        std::cout << "0. 뒤로가기\n";
        std::cout << "선택: ";
    }

    void OrderConsoleApp::handleCreate()
    {
        Model::Order input;
        try
        {
            input.sampleId = readNumber<int>("sampleId");
            input.customerName = readNonEmptyString("customerName");
            input.orderedQuantity = readNumber<int>("orderedQuantity");
            input.state = readOrderState();
        }
        catch (const InputCancelled&)
        {
            std::cout << "Create를 취소했습니다.\n";
            return;
        }

        try
        {
            const Model::Order created = repository_.create(input);
            std::cout << "생성되었습니다. id: " << created.id << '\n';
        }
        catch (const std::exception& e)
        {
            std::cout << "저장 중 오류가 발생했습니다: " << e.what() << '\n';
        }
    }

    void OrderConsoleApp::run()
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
