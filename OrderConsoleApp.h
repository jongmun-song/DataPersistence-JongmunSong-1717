#pragma once

// Console UI shell for Order (see docs/design/phase5-foundation.md,
// docs/design/phase6-create.md, docs/design/phase7-read.md).
//
// Phase 5 scope: a menu with only the "뒤로가기" option. Phase 6 adds the
// Create menu item. Phase 7 adds Read, Phase 8 adds Update, Phase 9 adds
// Delete (see docs/design/phase9-delete.md).

#include "OrderRepository.h"

namespace DataPersistence
{
    class OrderConsoleApp
    {
    public:
        explicit OrderConsoleApp(OrderRepository& repository);

        // Prints the menu, handles input, and repeats until the user goes back.
        void run();

    private:
        void printMenu() const;
        void handleCreate();
        void handleReadAll() const;
        void handleReadById() const;
        void handleUpdate();
        void handleDelete();

        OrderRepository& repository_;
    };
}
