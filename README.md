# exe

Учебный фреймворк для написания масштабируемых многопоточных concurrency приложений, созданный на основе выполнения заданий из курса "Теория и практика многопоточной синхронизации" за авторством Романа Липовского, MIPT

Написан с использованием C++23

## Content
- [executors](https://github.com/ddvamp/exe/tree/main/exe/executors)
	- inline (выполняет задачи на месте)
	- blocking static threadpool
	- **[X]** fast work-stealing threadpool
	- strand ([сериализует задачи вместо потоков](https://www.crazygaze.com/blog/2016/03/17/how-strands-work-and-why-you-should-use-them/))
- [stackful coroutines](https://github.com/ddvamp/exe/tree/main/exe/coroutine)
- **[X]** stackless coroutines
- [fibers](https://github.com/ddvamp/exe/tree/main/exe/fibers)
    - API & implementation
	- synchronization primitives
		- mutex
		- shared mutex
		- condition variable
		- wait group ([позволяет дождаться окончания задач и синхронизироваться с ними](https://gobyexample.com/waitgroups))
		- wait point (обобщенная wait group)
		- **[X]** channels
			- **[X]** implementation
			- **[X]** select
	- **[X]** futures support
- [futures & promises](https://github.com/ddvamp/exe/tree/main/exe/futures/)
	- future (будущее значение)
	- promise (обещание будущего значения)
	- **[X]** combinators
		- **[X]** all
		- **[X]** first_of
	- **[X]** await

## Requirements
1. C++23
2. gcc (trunk)
3. x86-64/sysv/elf
4. POSIX

## Configuration
Для отключения отладочных проверок внутри фреймворка необходимо передать флаг -DUTILS_DISABLE_DEBUG

## Third-party libraries
- concurrency - библиотека средств синхронизации потоков
- context - контекст исполнения для stackful корутин/файберов
- result - упрощенная реализация C++23 std::expected для представления значения или пойманного исключения (value or std::exception_ptr)
- utils - std-like библиотека общих утилит

## Links

- [Репозиторий курса](https://gitlab.com/Lipovsky/concurrency-course)

<!-- -->

- [Плейлист с лекциями(2022)](https://www.youtube.com/playlist?list=PL4_hYwCyhAva37lNnoMuBcKRELso5nvBm)
- [Плейлист с семинарами(2022)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvYTxm55RBm_HA5Bq5W1Nv-R)

<!-- -->

- [Плейлист с лекциями(2021)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvb7P8guwSTaaUS8EcOaWjxF)
- [Плейлист с семинарами(2021)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvaxKQHe6n8JQcoc7tWxKWRL)

<!-- -->

- [Плейлист с лекциями(2019)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvbW4DHFV3CY5CqupNqPf4jS)
- [Плейлист с семинарами(2019)](https://www.youtube.com/playlist?list=PL4_hYwCyhAvZgIfxf4nLnjXprGGWBs5VO)
