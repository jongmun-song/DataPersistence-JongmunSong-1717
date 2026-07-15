#include "MainConsoleApp.h"

#include <iostream>
#include <limits>

namespace DataPersistence
{
    MainConsoleApp::MainConsoleApp(ConsoleApp& sampleApp, OrderConsoleApp& orderApp,
        ProductionQueueEntryConsoleApp& entryApp)
        : sampleApp_(sampleApp)
        , orderApp_(orderApp)
        , entryApp_(entryApp)
    {
    }

    void MainConsoleApp::printMenu() const
    {
        std::cout << "\n=== 모델 선택 ===\n";
        std::cout << "1. Sample 관리\n";
        std::cout << "2. Order 관리\n";
        std::cout << "3. ProductionQueueEntry 관리\n";
        std::cout << "0. 종료\n";
        std::cout << "선택: ";
    }

    void MainConsoleApp::run()
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
                sampleApp_.run();
                break;
            case 2:
                orderApp_.run();
                break;
            case 3:
                entryApp_.run();
                break;
            default:
                std::cout << "알 수 없는 메뉴입니다.\n";
                break;
            }
        }
    }
}
