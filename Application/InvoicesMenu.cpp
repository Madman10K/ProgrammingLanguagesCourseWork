#include "Menu.hpp"
#include <iomanip>
#include <iostream>

void Hotel::Menu::invoicesMenu() noexcept
{
    runMenu(
        "Invoices",
        {
            {
                "List invoices",
                [this]() -> void
                {
                    listInvoices();
                }
            },
            {
                "View an invoice",
                [this]() -> void
                {
                    viewInvoice();
                }
            }
        }
    );
}

void Hotel::Menu::listInvoices() const noexcept
{
    printList(
        manager.getInvoices(),
        "No invoices have been issued yet",
        [](const Invoice& invoice) noexcept -> void
        {
            std::cout << "  Invoice #" << std::left << std::setw(5) << invoice.getID() << std::right << invoice.getIssueDate()
                      << "  room " << std::setw(4) << invoice.getRoomNumber() << "  " << std::left << std::setw(28)
                      << invoice.getBilledTo() << std::right << std::setw(12) << formatMoney(invoice.getTotal()) << "\n";
        }
    );
}

void Hotel::Menu::viewInvoice() noexcept
{
    listInvoices();
    if (manager.getInvoices().empty())
        return;

    const auto* invoice = readExisting<Invoice>(
        "Invoice ID",
        uint64_t{ 1 },
        static_cast<uint64_t>(INT64_MAX),
        &HotelManager::findInvoice,
        Result::InvoiceNotFound
    );
    if (invoice != nullptr)
        std::cout << *invoice << "\n";
}
