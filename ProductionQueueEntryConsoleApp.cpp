#include "ProductionQueueEntryConsoleApp.h"

#include <iostream>
#include <limits>

namespace DataPersistence
{
    ProductionQueueEntryConsoleApp::ProductionQueueEntryConsoleApp(ProductionQueueEntryRepository& repository)
        : repository_(repository)
    {
    }

    void ProductionQueueEntryConsoleApp::printMenu() const
    {
        std::cout << "\n=== ProductionQueueEntry 관리 ===\n";
        std::cout << "0. 뒤로가기\n";
        std::cout << "선택: ";
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
            default:
                std::cout << "알 수 없는 메뉴입니다.\n";
                break;
            }
        }
    }
}
