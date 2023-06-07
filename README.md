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

В настоящем репозитории код **не тестировался и не предназначен для использования**, но служит всего лишь демонстрацией применения автором знания языка C++

---

## About

Учебный фреймворк для написания многопоточных масштабируемых concurrency приложений, созданный на основе выполнения заданий из курса "Теория и практика многопоточной синхронизации" за авторством [Романа Липовского](https://gitlab.com/Lipovsky), MIPT

**Написан с использованием C++23**

## Structure

- ***[executors](https://github.com/ddvamp/exe/tree/main/exe/executors)***
	- inline (выполняет задачи на месте)
	- blocking static threadpool
	- [ ] fast work-stealing threadpool
	- strand ([сериализует асинхронные задачи без блокировки](https://www.crazygaze.com/blog/2016/03/17/how-strands-work-and-why-you-should-use-them/))
- ***[stackful coroutines](https://github.com/ddvamp/exe/tree/main/exe/coroutine)***
- [ ] ***stackless coroutines***
- ***[fibers](https://github.com/ddvamp/exe/tree/main/exe/fibers)***
    - API & implementation
	- synchronization primitives
		- mutex
		- shared mutex
		- condition variable
		- wait group ([позволяет дождаться окончания задач и синхронизироваться с ними](https://gobyexample.com/waitgroups))
		- wait point (обобщенная wait group)
		- [ ] channels
			- [ ] implementation
			- [ ] select
	- [ ] futures support
- ***[(functional) futures](https://github.com/ddvamp/exe/tree/main/exe/futures/fun)*** (фьючи в функциональном стиле)
	- constructors (пораждают фьючи)
		- contract (канал future-promise)
		- value (создать готовое значение)
		- just (создать готовое событие)
		- failure (создать готовую ошибку)
		- submit (отправить вычисление в executor и получить его будущий результат)
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

**-DUTILS_DISABLE_DEBUG**

## Third-party libraries

- ***[concurrency](https://github.com/ddvamp/exe/tree/main/concurrency)*** - библиотека средств синхронизации потоков
- ***[context](https://github.com/ddvamp/exe/tree/main/context)*** - контекст исполнения для stackful корутин/файберов
- ***[result](https://github.com/ddvamp/exe/tree/main/result)*** - упрощенная реализация C++23 std::expected для представления значения или пойманного исключения (value or std::exception_ptr)
- ***[utils](https://github.com/ddvamp/exe/tree/main/utils)*** - std-like библиотека общих утилит

## License

В настоящем репозитории код лицензирован под [GNU General Public License v3.0](https://github.com/ddvamp/exe/blob/main/LICENSE). Дополнительная информация доступна по адресу https://www.gnu.org/licenses/

## Links

- [Репозиторий курса](https://gitlab.com/Lipovsky/concurrency-course)

<!-- -->

- [Плейлист с лекциями(2023)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvZw9PmwtHjw6nnmgZJmAXNV)
- [Плейлист с семинарами(2023)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvZ-LgrsobwRBki8FV8JIFg_)

<!-- -->

- [Плейлист с лекциями(2022)](https://www.youtube.com/playlist?list=PL4_hYwCyhAva37lNnoMuBcKRELso5nvBm)
- [Плейлист с семинарами(2022)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvYTxm55RBm_HA5Bq5W1Nv-R)

<!-- -->

- [Плейлист с лекциями(2021)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvb7P8guwSTaaUS8EcOaWjxF)
- [Плейлист с семинарами(2021)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvaxKQHe6n8JQcoc7tWxKWRL)

<!-- -->

- [Плейлист с лекциями(2019)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvbW4DHFV3CY5CqupNqPf4jS)
- [Плейлист с семинарами(2019)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvZgIfxf4nLnjXprGGWBs5VO)
