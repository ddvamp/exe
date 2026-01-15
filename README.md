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

---

## About

Учебный фреймворк для написания многопоточных масштабируемых concurrency приложений, созданный на основе выполнения заданий из курса "Теория и практика многопоточной синхронизации" за авторством [Романа Липовского](https://gitlab.com/Lipovsky), MIPT

**Написан с использованием C++23**

## Structure

- ***[scheduler](https://github.com/ddvamp/exe/tree/main/exe/runtime)***
  - inline (выполняет задачи на месте)
  - blocking static threadpool
  - [ ] fast work-stealing threadpool
  - strand ([сериализует асинхронные задачи без блокировки](https://www.crazygaze.com/blog/2016/03/17/how-strands-work-and-why-you-should-use-them/))
- ***[fiber](https://github.com/ddvamp/exe/tree/main/exe/fiber)***
    - API & implementation
  - synchronization primitives
    - mutex
    - shared mutex
    - condition variable
    - wait group ([позволяет дождаться окончания задач и синхронизироваться с ними](https://gobyexample.com/waitgroups))
    - wait point (обобщенная wait group)
  - [ ] future support
- ***[(go) channels for fiber](https://github.com/ddvamp/exe/blob/main/exe/fiber/sync/channel.hpp)*** (имплементация каналов из языка go)
  - implementation
  - [ ] select
- ***[(functional) future](https://github.com/ddvamp/exe/tree/main/exe/future/fun)*** (фьючи в функциональном стиле)
  - constructors (пораждают фьючи)
    - contract (канал future-promise)
    - value (создать готовое значение)
    - just (создать готовое событие)
    - failure (создать готовую ошибку)
    - submit (отправить вычисление в scheduler и получить его будущий результат)
  - combinators (преобразуют одни фьючи в другие)
    - seq
      - via (установить, где будет значение будет потреблено)
      - inLine (использовать значение на месте)
      - map (преобразовать будущее значение)
      - flatten (получить фьючу, которая сама представлена будущим значением)
      - flatMap (map + flatten)
      - andThen (асинхронный try)
      - orElse (асинхронный catch)
    - par
      - first (получить первое значение или последнюю ошибку)
      - all (получить все значения или первую ошибку)
  - terminators (поглощают фьючи)
    - apply (установить способ использования будущего значения)
    - get (синхронно дождаться будущего значения)
    - detach (сбросить будущее значение)

## Requirements

1. C++23
2. gcc (trunk)
3. x86-64/sysv/elf
4. POSIX

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
