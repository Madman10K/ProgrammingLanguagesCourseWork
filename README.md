# HotelManager
Терминална програма за управление на резервации в хотел. 

Условие:

> ТЕМА5: Разработване на C++ система за резервации в хотел
> 
> Създайте C++ приложение за автоматизиране и оптимизиране на основните  процеси в хотелска среда. Приложението трябва да осигурява възможност за управление на стаи, клиенти, резервации и финансови услуги (фактуриране) чрез прилагане на основните принципи на обектно-ориентираното програмиране (енкапсулация, наследяване, полиморфизъм).
> 
> Системата трябва да поддържа следните ключови модули и структури от данни:
> 
> Стаи: информация за тип, цена за нощувка и наличност,
> 
> Клиенти: лични и контактни данни на гостите,
> 
> Резервации и фактуриране: управление на престоя, изчисление на крайна сума и генериране на отчети.
> 
> Изисквания:
> 
>> Автоматизация на хотелски операции: Приложението да предоставя възможност за централизирано управление на хотелските операции – проверка на наличността на стаите, правене на резервация, настаняване и изчисление на дължимите суми за престоя .
>> 
>> Генериране на отчети и справки: Да се имплементира функционалност за извеждане на отчети за заетостта на стаите, финансови отчети за периода и справки за клиенти с цел информирани бизнес решения .
>> 
>> ООП реализация, капсулация и полиморфизъм: Класовете (най-малко 3 класа – напр. Room, Guest/Customer, Reservation/HotelManager) трябва да капсулират всички детайли чрез private член-променливи, публични getters/setters и да прилагат наследяване и полиморфизъм, където е необходимо .
>> 
>> Проверка на данни и устойчивост: Необходимо е да се осигури валидация на входните данни (проверка за свободна стая за определен период, правилни дати, коректни суми) и устойчивост спрямо потребителски грешки .
>> 
>> Оператори и файлово съхранение: Да се предефинира операцията <<, която да се използва за извеждане на данните за резервациите и стаите .
>> 
>> Данните за стаите, клиентите и резервациите да се четат и трайно да се съхраняват във файл за осигуряване на непрекъсната работа между отделните сесии.
>> 
>> UML Диаграма: Класовете и връзките между тях да се опишат с UML клас диаграма .

Функционалности:

- Стаи - 3 вида стаи с различен капацитет и цена
- Гости - системата запаметява лична информация за всеки гост(име, номер на личен документ, телефон, имейл)
- Резервации - поддържа проверка на възможни резервации, резервиране, премахване на резервация, настаняване и напускане на стаята
- Фактуриране - генериране на фактури при напускане на стаята. Фактурата включва всички реално използвани нощувки по цената от резервацията и 10% ДДС
- Отчети - заетост на стаите и финансови отчети за различните видове стаи, за даден гост или за всички гости
- Валидация и устойчивост - всички данни се валидират и всяка операция връща променлива, показваща дали операцията е завършила успешно, или с грешка

Програмата идва с примерни данни от папката `Data`, които се копират и използват автоматично при компилиране.

## Компилация
Нужни са CMake 3.21+ и компилатор, който поддържа C++ 20(GCC, Clang или MSVC):
```bash
mkdir build && cd build
cmake ..
make
```
## Инструкции
Употреба:

```bash
cd build
./HotelManager
```

## Файлове
Всеки ред съдържа един запис, като полетата са разделени с `|`. Символът `\` може да се използва за escape-ване на `|` и `\`.

| Файл | Полета |
|---|---|
| `rooms.txt` | `number\|type\|pricePerNightCents\|capacity\|outOfService\|description` |
| `guests.txt` | `id\|firstName\|lastName\|idDocument\|phone\|email` |
| `reservations.txt` | `id\|room\|guestID\|checkIn\|checkOut\|guestCount\|nightlyRateCents\|status\|createdOn` |
| `invoices.txt` | `id\|reservationID\|room\|billedTo\|issueDate\|nights\|roomCost\|tax\|total` |

## UML диаграма
UML диаграмата описва отношенията между всички класове.
Рендерираната версия е качена като файлът `UML.png`.

Легенда:
- `-` private
- `+` public
- `#` protected
- `*` pure virtual
- `$` static.

```mermaid
classDiagram
    direction TB

    class Date {
        -sys_days value
        +parse(string, Date&)$ Result
        +fromComponents(int, unsigned, unsigned, Date&)$ Result
        +today()$ Date
        +daysUntil(Date) int64_t
        +addDays(int64_t) Date
        +toString() string
        +operator<=>(Date)
    }

    class Room {
        <<abstract>>
        -uint32_t number
        -Cents pricePerNight
        -uint32_t capacity
        -string description
        -bool bOutOfService
        +create(RoomType, ...)$ Result
        +clone()* unique_ptr~Room~
        +getType()* RoomType
        +getMaxCapacity()* uint32_t
        +calculateStayCost(uint32_t nights, Cents rate) Cents
        +print(ostream&) void
        +validate() Result
        +getters / validating setters()
    }

    class StandardRoom {
        +getType() RoomType
        +getMaxCapacity() uint32_t = 2
    }

    class DeluxeRoom {
        +getType() RoomType
        +getMaxCapacity() uint32_t = 3
    }

    class Suite {
        +getType() RoomType
        +getMaxCapacity() uint32_t = 6
    }

    class Guest {
        -uint64_t id
        -string firstName
        -string lastName
        -string idDocument
        -string phone
        -string email
        +validate() Result
        +getFullName() string
        +isValidPhone(string)$ bool
        +isValidEmail(string)$ bool
        +getters / validating setters()
    }

    class Reservation {
        -uint64_t id
        -uint32_t roomNumber
        -uint64_t guestID
        -Date checkInDate
        -Date checkOutDate
        -uint32_t guestCount
        -Cents nightlyRate
        -ReservationStatus status
        -Date createdOn
        +validate() Result
        +getNights() uint32_t
        +checkIn() Result
        +checkOut() Result
        +cancel() Result
        +overlaps(Date from, Date to) bool
        +nightsWithin(Date from, Date to) uint32_t
        +isBlockingRoom() bool
        +isActive() bool
    }

    class Invoice {
        -uint64_t id
        -uint64_t reservationID
        -uint32_t roomNumber
        -string billedTo
        -Date issueDate
        -uint32_t nights
        -Cents roomCost
        -Cents tax
        +calculate(id, Reservation, Room, billedTo, Date)$ Invoice
        +setAmounts(...) Result
        +getTotal() Cents
    }

    class HotelManager {
        -vector~unique_ptr~Room~~ rooms
        -vector~Guest~ guests
        -vector~Reservation~ reservations
        -vector~Invoice~ invoices
        -uint64_t nextGuestID
        -uint64_t nextReservationID
        -uint64_t nextInvoiceID
        -path dataDirectory
        -optional~Date~ fixedToday
        +setDataDirectory(path) HotelManager&
        +load(vector~string~& warnings) Result
        +save() Result
        +getToday() Date
        +addRoom(unique_ptr~Room~) Result
        +updateRoom(number, price, description, bOutOfService) Result
        +removeRoom(uint32_t) Result
        +checkAvailability(number, from, to) Result
        +findAvailableRooms(from, to, guestCount, type) vector~Room*~
        +addGuest(Guest, uint64_t& id) Result
        +updateGuest(Guest) Result
        +removeGuest(uint64_t) Result
        +searchGuests(string) vector~Guest*~
        +makeReservation(room, guest, from, to, count, uint64_t& id) Result
        +cancelReservation(uint64_t) Result
        +checkIn(uint64_t) Result
        +checkOut(uint64_t, uint64_t& invoiceID) Result
        +importX(record) Result
        -commit() Result
    }

    class Storage {
        +load(path, HotelManager&, vector~string~& warnings)$ Result
        +save(path, HotelManager)$ Result
    }

    class Report {
        <<abstract>>
        #const HotelManager& manager
        +generate(ostream&)* void
        +getTitle()* string
        +saveToFile(path, path&) Result
        #printHeading(ostream&, string)$ void
    }

    class PeriodReport {
        <<abstract>>
        #Date from
        #Date to
        #getEndExclusive() Date
        #formatPeriodTitle(prefix) string
    }

    class OccupancyReport {
        +generate(ostream&) void
        +getTitle() string
    }

    class FinancialReport {
        +generate(ostream&) void
        +getTitle() string
    }

    class GuestReport {
        -uint64_t guestID
        +generate(ostream&) void
        +getTitle() string
    }

    class Menu {
        -HotelManager& manager
        -bool bEOF
        +run() void
        -read~T~(prompt, parser) optional~T~
    }

    Room <|-- StandardRoom
    Room <|-- DeluxeRoom
    Room <|-- Suite
    Report <|-- PeriodReport
    PeriodReport <|-- OccupancyReport
    PeriodReport <|-- FinancialReport
    Report <|-- GuestReport

    HotelManager "1" *-- "0..*" Room : owns
    HotelManager "1" *-- "0..*" Guest : owns
    HotelManager "1" *-- "0..*" Reservation : owns
    HotelManager "1" *-- "0..*" Invoice : owns
    Reservation *-- Date : stay period

    Reservation "0..*" --> "1" Room : roomNumber
    Reservation "0..*" --> "1" Guest : guestID
    Invoice "0..1" --> "1" Reservation : reservationID
    Invoice ..> Room : calculate() uses calculateStayCost()

    HotelManager ..> Storage : load / save
    Storage ..> HotelManager : import / read
    Report --> "1" HotelManager : reads
    Menu --> "1" HotelManager : drives
    Menu ..> Report : shows
```
