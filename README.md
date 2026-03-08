# exe

## Contents

- [Disclaimer](#disclaimer)
- [About](#about)
- [Structure](#structure)
- [Requirements](#requirements)
- [Configuration](#configuration)
- [Third-party libraries](#third-party-libraries)
- [License](#license)
- [Links](#links)

## Disclaimer

---

В настоящем репозитории код **не тестировался, определённо содержит ошибки и не предназначен для использования**, но пишется для улучшения и отработки автором знания языка C++, а также служит демонстрацией применения этого знания

Репозиторий находится в активной разработке (WIP), поэтому структура и содержимое могут со временем изменяться. Наиболее интересный с точки зрения применения возможностей языка C++ код можно найти [здесь](https://github.com/ddvamp/exe/tree/main/include/exe/future2)

---

## About

Учебный фреймворк для написания многопоточных масштабируемых concurrency приложений, созданный на основе выполнения заданий из курса "Теория и практика многопоточной синхронизации" за авторством [Романа Липовского](https://gitlab.com/Lipovsky), MIPT

**Написан с использованием C++26**

## Structure

- ***[runtime](https://github.com/ddvamp/exe/tree/main/include/exe/runtime)*** - среда исполнения
  - schedulers - управление задачами
    - inline - выполняет задачи на месте
    - blocking static threadpool
    - [ ] fast work-stealing threadpool
    - strand - [сериализует асинхронные задачи без блокировки](https://www.crazygaze.com/blog/2016/03/17/how-strands-work-and-why-you-should-use-them/)
    - manual loop - ручной запуск задач
  - [] timers - управление таймерами
- ***[fiber](https://github.com/ddvamp/exe/tree/main/include/exe/fiber)***
  - API & implementation
  - synchronization primitives
  - [ ] future support
- ***[(go) channels for fiber](https://github.com/ddvamp/exe/blob/main/exe/fiber/sync/channel.hpp)*** - имплементация каналов из языка go
  - implementation
  - select
- ***[(functional) future](https://github.com/ddvamp/exe/tree/main/exe/future/fun)*** - фьючи в функциональном стиле
  - constructors - создают фьючи
  - combinators - преобразуют одни фьючи в другие
    - seq - последовательные комбинаторы
    - par - параллельные комбинаторы
  - terminators - поглощают фьючи
- ***[lazy future](https://github.com/ddvamp/exe/tree/main/exe/future2)*** - ленивые фьючи (осуществляют выделение памяти и синхронизацию потоков лишь при необходимости)

## Requirements

1. C++26
2. clang++-21/libstdc++
3. x86-64/sysv/elf
4. Linux

## Configuration

Для отключения отладочных проверок внутри фреймворка необходимо передать флаг

<!-- -->

**-DUTIL_DISABLE_DEBUG**

## Third-party libraries

- ***[concurrency](https://github.com/ddvamp/exe/tree/main/concurrency)*** - библиотека средств синхронизации потоков
- ***[context](https://github.com/ddvamp/exe/tree/main/context)*** - контекст исполнения для stackful корутин/файберов
- ***[util](https://github.com/ddvamp/exe/tree/main/util)*** - std-like библиотека общих утилит

## License

В настоящем репозитории код лицензирован под [GNU General Public License v3.0](https://github.com/ddvamp/exe/blob/main/LICENSE). Дополнительная информация доступна по адресу https://www.gnu.org/licenses/

## Links

- [Репозиторий курса](https://gitlab.com/Lipovsky/concurrency-course)
