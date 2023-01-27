#ifndef DDV_CONCURRENCY_MPMC_UNBOUNDED_BLOCKING_QUEUE_H_
#define DDV_CONCURRENCY_MPMC_UNBOUNDED_BLOCKING_QUEUE_H_ 1

#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <optional>
#include <utility>

#include "utils/debug.h"
#include "utils/defer.h"

namespace concurrency {

template <typename T>
class MPMCUnboundedBlockingQueue {
private:
	::std::deque<T> queue_;
	::std::mutex m_; // protects the queue
	::std::condition_variable has_elements_;
	::std::uint32_t waiters_on_take_ = 0;
	bool is_closed_ = false;

public:
	~MPMCUnboundedBlockingQueue() = default;

	MPMCUnboundedBlockingQueue(MPMCUnboundedBlockingQueue const &) = delete;
	void operator= (MPMCUnboundedBlockingQueue const &) = delete;

	MPMCUnboundedBlockingQueue(MPMCUnboundedBlockingQueue &&) = delete;
	void operator= (MPMCUnboundedBlockingQueue &&) = delete;

public:
	MPMCUnboundedBlockingQueue() = default;

	template <typename ...Args>
	bool put(Args &&...args)
		requires (::std::constructible_from<T, Args...>)
	{
		::std::lock_guard lock{m_};

		if (is_closed_) {
			return false;
		}

		queue_.emplace_back(::std::forward<Args>(args)...);

		if (waiters_on_take_ > 0) {
			has_elements_.notify_one();
		}

		return true;
	}

	[[nodiscard]] ::std::optional<T> take()
	{
		::std::unique_lock lock{m_};

		auto stop_waiting = ::utils::defer{
			[&w = ++waiters_on_take_]() noexcept { --w; }
		};

		while (true) {
			if (!queue_.empty()) {
				auto defer_pop = ::utils::defer{
					[&q = queue_]() noexcept { q.pop_front(); }
				};

				return ::std::move(queue_.front());
			}

			if (is_closed_) {
				return ::std::nullopt;
			}
			
			has_elements_.wait(lock);
		}
	}

	void close()
	{
		{
			::std::lock_guard lock{m_};

			UTILS_ASSERT(!is_closed_, "queue already closed");

			is_closed_ = true;
		}

		if (waiters_on_take_ > 0) {
			has_elements_.notify_all();
		}
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_MPMC_UNBOUNDED_BLOCKING_QUEUE_H_ */
