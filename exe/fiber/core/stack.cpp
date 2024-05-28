// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include "stack.hpp"

#include <utility>

#include <concurrency/qspinlock.hpp>
#include <util/debug/assert.hpp>

namespace exe::fiber {

namespace {

struct Node {
  Node *next_;
  ::util::memory_view self_;
};

class StackAllocator {
 private:
  Node *top_ = nullptr;
  ::concurrency::QSpinlock lock_; // protects the top_

 public:
  Stack Allocate() {
    auto token = lock_.GetToken();
    token.Lock();
    if (top_) [[likely]] {
      auto node = top_;
      top_ = node->next_;

      token.Unlock();
      return Stack::Acquire(node->self_);
    }

    token.Unlock();
    return AllocateNewStack();
  }

  void Deallocate(Stack &stack) noexcept {
    UTIL_ASSERT(stack.AllocationSize() != 0, "Stack in moved-from state");

    auto mem = ::std::move(stack).Release();
    auto node = ::new (mem.data()) Node{.self_ = mem};
    auto guard = lock_.MakeGuard();
    node->next_ = top_;
    top_ = node;
  }

 private:
  static Stack AllocateNewStack() {
    constexpr ::std::size_t kStackPages = 16;
    return Stack::AllocatePages(kStackPages);
  }
};

StackAllocator allocator;

}  // namespace

Stack AllocateStack() {
  return allocator.Allocate();
}

void DeallocateStack(Stack &&stack) noexcept {
  allocator.Deallocate(stack);
}

}  // namespace exe::fiber
