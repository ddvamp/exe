#ifndef DDV_CONCURRENCY_ONE_TIME_NOTIFICATION_H_
#define DDV_CONCURRENCY_ONE_TIME_NOTIFICATION_H_ 1

#include <condition_variable>
#include <cstdint>
#include <mutex>

#include "utils/debug.h"

namespace concurrency {

// Synchronization primitive for waiting for a one-time notification
// and sharing data with it
class OneTimeNotification {
private:
	inline static constexpr ::std::uint64_t waiting		= 0b01;
	inline static constexpr ::std::uint64_t notified	= 0b10;

	::std::uint64_t state_ = 0;
	::std::mutex m_; // protects the condvar
	::std::condition_variable its_time_;

public:
	void wait()
	{
		::std::unique_lock lock{m_};

		setWaiting();

		while (isWaitNeeded()) {
			its_time_.wait(lock);
		}
	}

	void notify()
	{
		{
			::std::lock_guard lock{m_};

			UTILS_ASSERT(isWaitNeeded(), "One-time notification sent twice");

			setNotified();

			if (areThereNoWaiters()) {
				return;
			}
		}

		its_time_.notify_all();
	}

private:
	[[nodiscard]] bool isWaitNeeded() const noexcept
	{
		return (state_ & notified) == 0;
	}

	[[nodiscard]] bool areThereNoWaiters() const noexcept
	{
		return (state_ & waiting) == 0;
	}

	void setNotified() noexcept
	{
		state_ |= notified;
	}

	void setWaiting() noexcept
	{
		state_ |= waiting;
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_ONE_TIME_NOTIFICATION_H_ */
