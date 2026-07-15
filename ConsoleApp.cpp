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
        // Thrown internally to unwind an input flow (handleCreate(),
        // handleReadById(), ...) when the user cancels input mid-flow (see
        // docs/design/phase1-create.md).
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
    }

    ConsoleApp::ConsoleApp(SampleRepository& repository)
        : repository_(repository)
    {
    }

    void ConsoleApp::printMenu() const
    {
        std::cout << "\n=== Sample 관리 ===\n";
        std::cout << "1. Create\n";
        std::cout << "2. 전체 목록 보기\n";
        std::cout << "3. ID로 검색\n";
        std::cout << "0. 종료\n";
        std::cout << "선택: ";
    }

    namespace
    {
        void printSample(const Model::Sample& sample)
        {
            std::cout << "id: " << sample.id
                       << ", name: " << sample.name
                       << ", averageProductionTimePerUnit: " << sample.averageProductionTimePerUnit
                       << ", yieldRatio: " << sample.yieldRatio
                       << ", stockQuantity: " << sample.stockQuantity << '\n';
        }
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
        catch (const InputCancelled&)
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

    void ConsoleApp::handleReadAll() const
    {
        const std::vector<Model::Sample>& samples = repository_.all();
        if (samples.empty())
        {
            std::cout << "등록된 시료가 없습니다.\n";
            return;
        }

        for (const Model::Sample& sample : samples)
        {
            printSample(sample);
        }
    }

    void ConsoleApp::handleReadById() const
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

        const std::optional<Model::Sample> found = repository_.findById(*id);
        if (!found.has_value())
        {
            std::cout << "해당 ID의 시료를 찾을 수 없습니다.\n";
            return;
        }

        printSample(*found);
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
            case 2:
                handleReadAll();
                break;
            case 3:
                handleReadById();
                break;
            default:
                std::cout << "알 수 없는 메뉴입니다.\n";
                break;
            }
        }
    }
}
