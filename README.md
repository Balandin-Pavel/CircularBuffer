# circular_buffer

STL-совместимый шаблонный контейнер «кольцевой буфер» (circular / ring buffer) на C++23. Header-only библиотека (`lib/circular_buffer.h`).

```cpp
template<typename T, bool Extendable = true, typename Allocator = std::allocator<T>>
class circular_buffer;
```

## Параметры шаблона

- **`T`** — тип хранимых элементов.
- **`Extendable`** — поведение при заполнении буфера:
  - `true` (по умолчанию) — при переполнении ёмкость удваивается, как у `std::vector`;
  - `false` — буфер фиксированного размера: новый элемент затирает самый старый (`push_back` вытесняет из `front`, `push_front` — из `back`).
- **`Allocator`** — произвольный аллокатор (по умолчанию `std::allocator<T>`).

## Возможности

- Полностью совместим с именованными требованиями STL: `Container`, `SequenceContainer`, `ReversibleContainer`, `AllocatorAwareContainer`.
- Итератор произвольного доступа (`RandomAccessIterator`) — работает со всеми алгоритмами `<algorithm>` (`std::sort`, `std::find`, `std::accumulate`, `std::reverse` и т.д.) и с range-based `for`.
- Поддержка кастомных аллокаторов и типов без конструктора по умолчанию.
- Корректная работа с "перехлёстом" через границу внутреннего массива (wraparound) при любых операциях — вставке, удалении, копировании, изменении ёмкости.

## Сборка

Требуется CMake ≥ 3.12 и компилятор с поддержкой C++23. Тесты используют Google Test, который подтягивается автоматически через `FetchContent` (нужен доступ к github.com).

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

Собранный демонстрационный бинарник:

```bash
./build/bin/circular_buffer
```

## Тесты

```bash
cd build && ctest --output-on-failure
```

Тесты расширяемого буфера (Extendable-специфичные сценарии) включаются флагом:

```bash
cmake -B build -DRUN_EXT_TESTS=ON
```

### Покрытие тестами

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DWITH_COVERAGE=ON
cmake --build build
cmake --build build --target coverage
```

Отчёт появится в `build/coverage.html`.

## API

### Конструкторы

```cpp
circular_buffer();
explicit circular_buffer(const Allocator& alloc);
explicit circular_buffer(size_type capacity, const Allocator& alloc = Allocator());
circular_buffer(size_type count, const T& value, const Allocator& alloc = Allocator());
circular_buffer(InputIt first, InputIt last, const Allocator& alloc = Allocator());
circular_buffer(std::initializer_list<T> init, const Allocator& alloc = Allocator());
circular_buffer(const circular_buffer& other);
circular_buffer(circular_buffer&& other) noexcept;
circular_buffer(circular_buffer&& other, const Allocator& alloc);
```

### Доступ к элементам

`at(pos)`, `operator[](pos)`, `front()`, `back()`.

### Итераторы

`begin/end`, `cbegin/cend`, `rbegin/rend`, `crbegin/crend`.

### Ёмкость

`empty()`, `full()`, `size()`, `max_size()`, `capacity()`, `reserve(n)`.

### Модификаторы

`clear()`, `push_back`/`push_front`, `pop_back`/`pop_front`, `emplace_back`/`emplace_front`, `insert` (значение, N значений, диапазон итераторов, initializer_list), `emplace(pos, args...)`, `erase`, `assign`, `resize`, `swap`.

### Операторы сравнения

`==`, `!=`, `<`, `>`, `<=`, `>=` (лексикографическое сравнение).

## Примеры использования

Расширяемый буфер (по умолчанию удваивает ёмкость):

```cpp
#include <circular_buffer.h>

circular_buffer<int> cb(3);
cb.push_back(1);
cb.push_back(2);
cb.push_back(3);
cb.push_back(4);   // ёмкость автоматически удвоится → {1, 2, 3, 4}
```

Буфер фиксированного размера (перезаписывает самый старый элемент):

```cpp
circular_buffer<int, false> ring(3);
ring.push_back(1);
ring.push_back(2);
ring.push_back(3);
ring.push_back(4);   // затирает 1 → {2, 3, 4}
```

Move-семантика и вставка в произвольную позицию:

```cpp
circular_buffer<int> a{1, 2, 3};
circular_buffer<int> b(std::move(a));
b.emplace(b.begin() + 1, 42);   // {1, 42, 2, 3}
```

Итерация и стандартные алгоритмы:

```cpp
for (int v : cb) { /* ... */ }
std::sort(cb.begin(), cb.end());
```

## Структура проекта

```
lib/          — заголовок circular_buffer.h (сама библиотека)
bin/          — демонстрационный исполняемый файл
tests/        — модульные тесты на Google Test
```