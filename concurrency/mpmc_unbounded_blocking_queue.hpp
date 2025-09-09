// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

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

#include "util/debug.hpp"
#include "util/defer.hpp"

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
		auto lock = ::std::lock_guard(m_);

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
		auto lock = ::std::unique_lock(m_);

		auto stop_waiting = ::util::defer(
			[&w = ++waiters_on_take_]() noexcept { --w; }
		);

		while (true) {
			if (!queue_.empty()) {
				auto cleanup = ::util::defer(
					[&q = queue_]() noexcept { q.pop_front(); }
				);

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
			auto lock = ::std::lock_guard(m_);

			UTIL_VERIFY(
				!::std::exchange(is_closed_, true),
				"queue already closed"
			);

			if (waiters_on_take_ == 0) {
				return;
			}
		}

		has_elements_.notify_all();
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_MPMC_UNBOUNDED_BLOCKING_QUEUE_H_ */
