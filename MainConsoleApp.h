#pragma once

// Top-level model selection menu (see docs/design/phase5-foundation.md).
//
// Dispatches to the Sample/Order/ProductionQueueEntry sub console apps and
// returns to the model selection menu when a sub app's run() returns
// ("뒤로가기"). Only exits the program when "0" is chosen at this top level.

#include "ConsoleApp.h"
#include "OrderConsoleApp.h"
#include "ProductionQueueEntryConsoleApp.h"

namespace DataPersistence
{
    class MainConsoleApp
    {
    public:
        MainConsoleApp(ConsoleApp& sampleApp, OrderConsoleApp& orderApp,
            ProductionQueueEntryConsoleApp& entryApp);

        void run();

    private:
        void printMenu() const;

        ConsoleApp& sampleApp_;
        OrderConsoleApp& orderApp_;
        ProductionQueueEntryConsoleApp& entryApp_;
    };
}
