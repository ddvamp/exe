//
// select.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_SYNC_SELECT_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_SELECT_HPP_INCLUDED_ 1

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>
#include <exe/fiber/sync/channel2.hpp>

#include <concurrency/rendezvous.hpp>
#include <util/utility.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <tuple>
#include <variant>

namespace exe::fiber {

template <typename ...Clauses>
using SelectResult =
		::std::variant<::util::unit_t, typename Clauses::ResultType...>;

namespace detail {

// [TODO]: ?util
class SelectorShuffler {
 private:
  ::std::uint64_t state_;

 public:
  explicit SelectorShuffler(::std::uint64_t seed = 0) noexcept
			: state_(seed != 0 ? seed : 88'172'645'463'325'252ull) {}

	inline ::std::uint64_t Next() noexcept {
		state_ ^= state_ >> 12;
		state_ ^= state_ << 25;
		state_ ^= state_ >> 27;
		return state_ * 2'685'821'657'736'338'717ull;
	}

	inline ::std::uint64_t NextBounded(::std::uint64_t bound) noexcept {
		return Next() % bound;
	}
};

class SelectorBase {
 private:
  FiberHandle handle_;
	::std::atomic_bool first_ = true;
	::std::atomic_size_t active_;
	::concurrency::Rendezvous rendezvous_;

 public:
	FiberHandle SetFiber(FiberHandle &&self) noexcept {
		handle_ = ::std::move(self);
		return rendezvous_.Arrive() ? ::std::move(handle_) : FiberHandle::Invalid();
	}

 protected:
  explicit SelectorBase(::std::size_t active) noexcept : active_(active) {}

	[[nodiscard]] bool FirstUse() noexcept {
		return first_.exchange(false, ::std::memory_order_relaxed);
	}

	[[nodiscard]] bool LastLeave() noexcept {
		return active_.fetch_sub(1, ::std::memory_order_relaxed) == 1;
	}

	void Arrive() noexcept {
		if (rendezvous_.Arrive()) {
			::std::move(handle_).Schedule();
		}
	}

  [[nodiscard]] static ::std::uint64_t GetMiddle(::std::uint64_t n) noexcept {
		static thread_local SelectorShuffler generator(::std::random_device{}());
		return generator.NextBounded(n);
	}
};

class SelectorAwaiter final : public IAwaiter {
 private:
	SelectorBase &selector_;

 public:
	explicit SelectorAwaiter(SelectorBase &selector) noexcept
			: selector_(selector) {}

	FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
		return selector_.SetFiber(::std::move(self));
	}
};

template <typename ...Clauses>
class Selector final : private SelectorBase {
 private:
  template <::std::size_t Idx>
	using Clause = Clauses...[Idx];

	template <::std::size_t Idx>
	using ValueType = Clauses...[Idx]::ValueType;

	SelectResult<Clauses...> result_;

 public:
  Selector() noexcept : SelectorBase(sizeof...(Clauses)) {}

	SelectResult<Clauses...> Select(Clauses ...clauses) {
		auto [...waiters] = MakeWaiters(clauses..., GetMiddle(sizeof...(Clauses)));

		auto const ready = (... || waiters()) || (... || waiters());
		if (!ready) [[unlikely]] {
			SelectorAwaiter awaiter(*this);
			self::Suspend(awaiter);
		}

		(..., waiters.Unlink());

		return ::std::move(result_);
	}

 private:
	template <::std::size_t Idx>
	void SetResult(Clause<Idx>::ResultType &&value) noexcept {
		result_.template emplace<Idx + 1>(::std::move(value));
		Arrive();
	}

	template <::std::size_t Idx>
	[[nodiscard]] ::std::optional<ValueType<Idx>> TrySend(ValueType<Idx> &value)
			noexcept {
		::std::optional<ValueType<Idx>> result;

		if (FirstUse()) [[unlikely]] {
			result.emplace(::std::move(value));
			SetResult<Idx>(::util::unit_t{});
		}

		return result;
	}

	template <::std::size_t Idx>
	[[nodiscard]] bool TryReceive(ValueType<Idx> &value) noexcept {
		if (FirstUse()) [[unlikely]] {
			SetResult<Idx>(::std::move(value));
			return true;
		}

		return false;
	}

	void Close() noexcept {
		if (LastLeave()) [[unlikely]] {
			Arrive();
		}
	}

 private:
	template <::std::size_t Idx>
	class Sender final : private ChannelState<ValueType<Idx>>::Sender {
	 private:
		Selector &selector_;
		Clause<Idx> clause_;
		bool active_;
		bool linked_ = false;

	 public:
	  Sender(Selector &selector, Clause<Idx> clause, bool active) noexcept
				: selector_(selector)
				, clause_(clause)
				, active_(active) {}

		[[nodiscard]] bool operator() () noexcept {
			if (!::std::exchange(active_, !active_)) {
				return false;
			}

			auto const status = clause_.chan.Send(*this);
			linked_ = (status == ChannelOpStatus::kWait);
			return status == ChannelOpStatus::kReady;
		}

		void Unlink() noexcept {
			if (!linked_) {
				return;
			}

			auto guard = clause_.chan.Lock();
			if (this->linked()) {
				this->unlink();
			}
		}

		[[nodiscard]] ::std::optional<ValueType<Idx>> TrySend() noexcept override {
			return selector_.TrySend<Idx>(clause_.value);
		}

		void Close() noexcept override {
			selector_.Close();
		}
	};

	template <::std::size_t Idx>
	class Receiver final : private ChannelState<ValueType<Idx>>::Receiver {
	 private:
		Selector &selector_;
		Clause<Idx> clause_;
		bool active_;
		bool linked_ = false;

	 public:
	  Receiver(Selector &selector, Clause<Idx> clause, bool active) noexcept
				: selector_(selector)
				, clause_(clause)
				, active_(active) {}

		[[nodiscard]] bool operator() () noexcept {
			if (!::std::exchange(active_, !active_)) {
				return false;
			}

			auto const status = clause_.chan.Receive(*this);
			linked_ = (status == ChannelOpStatus::kWait);
			return status == ChannelOpStatus::kReady;
		}

		void Unlink() {
			if (!linked_) {
				return;
			}

			auto guard = clause_.chan.Lock();
			if (this->next_) {
				this->unlink();
			}
		}

		[[nodiscard]] bool TryReceive(ValueType<Idx> &value) noexcept override {
			return selector_.TryReceive<Idx>(value);
		}

		void Close() noexcept override {
			selector_.Close();
		}
	};

  template <::std::size_t Idx>
	Sender<Idx> MakeWaiter(SendClause<ValueType<Idx>> clause, bool active)
			noexcept {
		return {*this, clause, active};
	}

	template <::std::size_t Idx>
	Receiver<Idx> MakeWaiter(ReceiveClause<ValueType<Idx>> clause, bool active)
			noexcept {
		return {*this, clause, active};
	}

  auto MakeWaiters(Clauses ...clauses, ::std::size_t middle) noexcept {
		return [this, middle, clauses...]<::std::size_t ...Ids>(
				::std::index_sequence<Ids...>) noexcept {
			return ::std::tuple(MakeWaiter<Ids>(clauses, Ids >= middle)...);
		}(::std::index_sequence_for<Clauses...>{});
	}
};

} // namespace detail

template <SelectClause ...Clauses>
[[nodiscard]] SelectResult<Clauses...> Select(Clauses ...clauses) noexcept {
	return detail::Selector<Clauses...>().Select(clauses...);
}

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_SELECT_HPP_INCLUDED_ */
