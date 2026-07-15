#pragma once

// Console UI shell for ProductionQueueEntry (see
// docs/design/phase5-foundation.md).
//
// Phase 5 scope: a menu with only the "뒤로가기" option. Phase 6 adds the
// Create menu item, Phase 7 adds Read, Phase 8 adds Update, Phase 9 adds
// Delete.

#include "ProductionQueueEntryRepository.h"

namespace DataPersistence
{
    class ProductionQueueEntryConsoleApp
    {
    public:
        explicit ProductionQueueEntryConsoleApp(ProductionQueueEntryRepository& repository);

        // Prints the menu, handles input, and repeats until the user goes back.
        void run();

    private:
        void printMenu() const;

        ProductionQueueEntryRepository& repository_;
    };
}
