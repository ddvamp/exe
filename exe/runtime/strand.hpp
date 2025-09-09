// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_STRAND_H_
#define DDV_EXE_EXECUTORS_STRAND_H_ 1

#include "exe/runtime/task/scheduler.hpp"

#include "util/refer/ref_counted_ptr.hpp"

namespace exe::runtime {

// Adapter for an scheduler that allows to
// serialize asynchronous critical sections without using explicit locks
//
// Instead of moving the "lock" between threads, it moves critical sections,
// thereby allowing data to be in cache all the time
//
// Sending critical sections is wait-free except for
// launching a new critical sections of the strand itself
class Strand final : public ISafeScheduler {
private:
	class Impl;

	::util::RefCountedPtr<Impl> impl_;

public:
	~Strand() noexcept;

	Strand(Strand const &) = delete;
	void operator= (Strand const &) = delete;

	Strand(Strand &&) = delete;
	void operator= (Strand &&) = delete;

public:
	explicit Strand(ISafeScheduler &where);

	[[nodiscard]] ISafeScheduler &getScheduler() const noexcept;

	void submit(TaskBase *critical_section) noexcept override;
};

} // namespace exe::runtime

#endif /* DDV_EXE_EXECUTORS_STRAND_H_ */
