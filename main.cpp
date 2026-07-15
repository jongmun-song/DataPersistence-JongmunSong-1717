// Entry point (see docs/design/phase0-foundation.md,
// docs/design/phase5-foundation.md).

#include <exception>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ConsoleApp.h"
#include "MainConsoleApp.h"
#include "OrderConsoleApp.h"
#include "OrderRepository.h"
#include "ProductionQueueEntryConsoleApp.h"
#include "ProductionQueueEntryRepository.h"
#include "SampleRepository.h"

int main()
{
#ifdef _WIN32
    // Source files are UTF-8; align the console output code page so
    // Korean menu text renders correctly instead of as mojibake.
    SetConsoleOutputCP(CP_UTF8);
#endif

    DataPersistence::SampleRepository sampleRepository("samples.json");
    DataPersistence::OrderRepository orderRepository("orders.json");
    DataPersistence::ProductionQueueEntryRepository entryRepository("production_queue.json");

    try
    {
        sampleRepository.load();
    }
    catch (const std::exception& e)
    {
        std::cerr << "samples.json 파일을 읽는 중 오류가 발생했습니다: " << e.what() << '\n';
        return 1;
    }

    try
    {
        orderRepository.load();
    }
    catch (const std::exception& e)
    {
        std::cerr << "orders.json 파일을 읽는 중 오류가 발생했습니다: " << e.what() << '\n';
        return 1;
    }

    try
    {
        entryRepository.load();
    }
    catch (const std::exception& e)
    {
        std::cerr << "production_queue.json 파일을 읽는 중 오류가 발생했습니다: " << e.what() << '\n';
        return 1;
    }

    DataPersistence::ConsoleApp sampleApp(sampleRepository);
    DataPersistence::OrderConsoleApp orderApp(orderRepository);
    DataPersistence::ProductionQueueEntryConsoleApp entryApp(entryRepository);

    DataPersistence::MainConsoleApp mainApp(sampleApp, orderApp, entryApp);
    mainApp.run();

    return 0;
}
