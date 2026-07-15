#include "ConsoleApp.h"

#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>

#include "Model/Sample.h"

namespace DataPersistence
{
    namespace
    {
        // Thrown internally to unwind handleCreate() when the user cancels
        // input mid-flow (see docs/design/phase1-create.md).
        struct CreateCancelled
        {
        };

        std::string readLine(const std::string& prompt)
        {
            std::cout << prompt;
            std::string line;
            std::getline(std::cin, line);
            if (line == "q")
            {
                throw CreateCancelled{};
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

        template <typename T>
        T readNumber(const std::string& fieldName)
        {
            while (true)
            {
                const std::string value = readLine(fieldName + " (취소: q): ");
                std::istringstream iss(value);
                T parsed{};
                iss >> parsed;
                if (iss && iss.eof())
                {
                    return parsed;
                }
                std::cout << fieldName + " 형식이 올바르지 않습니다. 다시 입력하세요.\n";
            }
        }
    }

    ConsoleApp::ConsoleApp(SampleRepository& repository)
        : repository_(repository)
    {
    }

    void ConsoleApp::printMenu() const
    {
        std::cout << "\n=== Sample 관리 ===\n";
        std::cout << "1. Create\n";
        std::cout << "0. 종료\n";
        std::cout << "선택: ";
    }

    void ConsoleApp::handleCreate()
    {
        Model::Sample input;
        try
        {
            input.name = readNonEmptyString("name");
            input.averageProductionTimePerUnit = readNumber<double>("averageProductionTimePerUnit");
            input.yieldRatio = readNumber<double>("yieldRatio");
            input.stockQuantity = readNumber<int>("stockQuantity");
        }
        catch (const CreateCancelled&)
        {
            std::cout << "Create를 취소했습니다.\n";
            return;
        }

        try
        {
            const Model::Sample created = repository_.create(input);
            std::cout << "생성되었습니다. id: " << created.id << '\n';
        }
        catch (const std::exception& e)
        {
            std::cout << "저장 중 오류가 발생했습니다: " << e.what() << '\n';
        }
    }

    void ConsoleApp::run()
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
