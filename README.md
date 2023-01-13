# SimpleVector
Контейнер, хранящий элементы в массиве динамической памяти. Поддерживает основные возможности стандартного std::vector. Используется идиома RAII.

## Конструктор
- SimpleVector(); *по умолчанию*
- SimpleVector(const SimpleVector& other); - *конструктор копирования*
- SimpleVector(SimpleVector&& other); - *конструктор перемещения*
- explicit SimpleVector(size_t size); - *вектор из числа size элементов*
- SimpleVector(size_t size, const Type& value); - *вектор из чиста size элемнтов со значением value*
- SimpleVector(std::initializer_list<Type> init); - *список инициализации*

## Доступ к элементам
- Type& At(size_t index); - *возвращает значение элемента в позиции index или выбрасывает исключение out_of_range*
- Type& operator[](size_t index) noexcept; - *возвращает значение по индексу index, корректность индекса должна быть обеспечена пользователем*
- const Type& operator[](size_t index) const noexcept;

## Итераторы
Тип итератора - итератор произвольного доступа

- begin() noexcept ; - *возвращает итератор на первый элемент массива*
- end() noexcept ; - *возвращает итератор на позицию, за последним элементом массива*

## Методы
- bool IsEmpty() const noexcept; - *true, если вектор пуст, иначе - false*
- size_t GetSize() const noexcept; -  *возвращает количество элементов*
- size_t GetCapacity() const noexcept; - *возвращает количество зарезервированных в памяти элемнтов*
- void PushBack(const Type& value); - *вставка элемента в конец вектора*
- void PushBack(const Type&& value);
- Iterator Insert(ConstIterator pos, const Type& value); - *вставка элемента в позицию перед итератором pos*
- Iterator Insert(ConstIterator pos, const Type&& value);
- void Resize(size_t new_size); - *изменяет размер вектора на new_size*
- void Reserve(size_t new_capacity); - *резервирует в векотре место для new_capacity общего числа элементов*
- void PopBack() noexcept; *если вектор не пустой, удаляет последний элемент вектора*
- Iterator Erase(ConstIterator pos); *удаляет элемент с позицией pos, если такой элемент существует, либо выбрасывает исключение*
- void swap(SimpleVector& other) noexcept; *обменивается данными с вектором other*

## Сложность некоторых методов:
- Произвольный доступ - константа O(1)
- Вставка или удаление элементов в конце - амортизированная O(1)
- Вставка или удаление элементов - O(n)
## Установка
- Добавить array_ptr.h, simple_vector.h в файлы проекта
- Подключить через директиву #include "simple_vector.h"
