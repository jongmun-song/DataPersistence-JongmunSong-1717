#pragma once

// Console UI shell (see docs/design/phase0-foundation.md).
//
// Phase 0 scope: a menu with only the "exit" option. CRUD menu items are
// added incrementally in later phases.

#include "SampleRepository.h"

namespace DataPersistence
{
    class ConsoleApp
    {
    public:
        explicit ConsoleApp(SampleRepository& repository);

        // Prints the menu, handles input, and repeats until the user exits.
        void run();

    private:
        void printMenu() const;

        SampleRepository& repository_;
    };
}
