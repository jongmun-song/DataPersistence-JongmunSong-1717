#include "OrderConsoleApp.h"

#include <functional>
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

        std::string orderStateToString(Model::OrderState state)
        {
            switch (state)
            {
            case Model::OrderState::RESERVED:
                return "RESERVED";
            case Model::OrderState::CONFIRMED:
                return "CONFIRMED";
            case Model::OrderState::PRODUCING:
                return "PRODUCING";
            case Model::OrderState::RELEASE:
                return "RELEASE";
            case Model::OrderState::REJECTED:
                return "REJECTED";
            }
            return "UNKNOWN";
        }

        void printOrder(const Model::Order& order)
        {
            std::cout << "id: " << order.id
                       << ", sampleId: " << order.sampleId
                       << ", customerName: " << order.customerName
                       << ", orderedQuantity: " << order.orderedQuantity
                       << ", state: " << orderStateToString(order.state) << '\n';
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
        std::cout << "2. 전체 목록 보기\n";
        std::cout << "3. ID로 검색\n";
        std::cout << "4. Update\n";
        std::cout << "5. Delete\n";
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

    void OrderConsoleApp::handleReadAll() const
    {
        const std::vector<Model::Order>& orders = repository_.all();
        if (orders.empty())
        {
            std::cout << "등록된 주문이 없습니다.\n";
            return;
        }

        for (const Model::Order& order : orders)
        {
            printOrder(order);
        }
    }

    void OrderConsoleApp::handleReadById() const
    {
        std::string value;
        try
        {
            value = readLine("검색할 id (취소: q): ");
        }
        catch (const InputCancelled&)
        {
            std::cout << "검색을 취소했습니다.\n";
            return;
        }

        const std::optional<int> id = tryParseNumber<int>(value);
        if (!id.has_value())
        {
            std::cout << "id 형식이 올바르지 않습니다.\n";
            return;
        }

        const std::optional<Model::Order> found = repository_.findById(*id);
        if (!found.has_value())
        {
            std::cout << "해당 ID의 주문을 찾을 수 없습니다.\n";
            return;
        }

        printOrder(*found);
    }

    void OrderConsoleApp::handleUpdate()
    {
        std::string idText;
        try
        {
            idText = readLine("수정할 id (취소: q): ");
        }
        catch (const InputCancelled&)
        {
            std::cout << "Update를 취소했습니다.\n";
            return;
        }

        const std::optional<int> id = tryParseNumber<int>(idText);
        if (!id.has_value())
        {
            std::cout << "id 형식이 올바르지 않습니다.\n";
            return;
        }

        const std::optional<Model::Order> found = repository_.findById(*id);
        if (!found.has_value())
        {
            std::cout << "해당 ID의 주문을 찾을 수 없습니다.\n";
            return;
        }

        std::cout << "현재 값: ";
        printOrder(*found);

        std::cout << "수정할 필드를 선택하세요.\n";
        std::cout << "1. customerName\n";
        std::cout << "2. orderedQuantity\n";
        std::cout << "3. state\n";

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

        std::function<void(Model::Order&)> mutator;
        try
        {
            switch (*fieldChoice)
            {
            case 1:
            {
                const std::string value = readNonEmptyString("customerName");
                mutator = [value](Model::Order& order) { order.customerName = value; };
                break;
            }
            case 2:
            {
                const int value = readNumber<int>("orderedQuantity");
                mutator = [value](Model::Order& order) { order.orderedQuantity = value; };
                break;
            }
            case 3:
            {
                const Model::OrderState value = readOrderState();
                mutator = [value](Model::Order& order) { order.state = value; };
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
            const bool updated = repository_.update(*id, mutator);
            if (updated)
            {
                std::cout << "수정되었습니다.\n";
            }
            else
            {
                std::cout << "해당 ID의 주문을 찾을 수 없습니다.\n";
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "저장 중 오류가 발생했습니다: " << e.what() << '\n';
        }
    }

    void OrderConsoleApp::handleDelete()
    {
        std::string idText;
        try
        {
            idText = readLine("삭제할 id (취소: q): ");
        }
        catch (const InputCancelled&)
        {
            std::cout << "Delete를 취소했습니다.\n";
            return;
        }

        const std::optional<int> id = tryParseNumber<int>(idText);
        if (!id.has_value())
        {
            std::cout << "id 형식이 올바르지 않습니다.\n";
            return;
        }

        const std::optional<Model::Order> found = repository_.findById(*id);
        if (!found.has_value())
        {
            std::cout << "해당 ID의 주문을 찾을 수 없습니다.\n";
            return;
        }

        std::cout << "삭제 대상: ";
        printOrder(*found);

        std::string confirm;
        try
        {
            confirm = readLine("정말 삭제하시겠습니까? (Y/N): ");
        }
        catch (const InputCancelled&)
        {
            std::cout << "Delete를 취소했습니다.\n";
            return;
        }

        if (confirm != "Y" && confirm != "y")
        {
            std::cout << "삭제를 취소했습니다.\n";
            return;
        }

        try
        {
            const bool removed = repository_.remove(*id);
            if (removed)
            {
                std::cout << "삭제되었습니다.\n";
            }
            else
            {
                std::cout << "해당 ID의 주문을 찾을 수 없습니다.\n";
            }
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
            case 2:
                handleReadAll();
                break;
            case 3:
                handleReadById();
                break;
            case 4:
                handleUpdate();
                break;
            case 5:
                handleDelete();
                break;
            default:
                std::cout << "알 수 없는 메뉴입니다.\n";
                break;
            }
        }
    }
}
