#pragma once

// Console UI shell (see docs/design/phase0-foundation.md, phase1-create.md,
// phase2-read.md).
//
// Phase 0 scope: a menu with only the "exit" option. Phase 1 adds the
// Create menu item. Phase 2 adds the read-all/find-by-id menu items.
// Remaining CRUD menu items are added incrementally in later phases.

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
        void handleCreate();
        void handleReadAll() const;
        void handleReadById() const;

        SampleRepository& repository_;
    };
}
