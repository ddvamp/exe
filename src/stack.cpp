//
// stack.cpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/fiber/core/stack.hpp>

#include <concurrency/qspinlock.hpp>
#include <util/debug/assert.hpp>
#include <util/memory/page_allocation.hpp>
#include <util/memory/view.hpp>

#include <cstddef>
#include <utility>

namespace exe::fiber {

namespace {

class StackAllocator {
 private:
  struct Node {
    Node *next;
    Stack stack;
  };

  Node *top_ = nullptr;
  ::concurrency::QSpinlock lock_; // Protects the top_

 public:
  ~StackAllocator() {
    while (top_) {
      ::std::exchange(top_, top_->next)->~Node();
    }
  }

  StackAllocator(StackAllocator const &) = delete;
  void operator= (StackAllocator const &) = delete;

  StackAllocator(StackAllocator &&) = delete;
  void operator= (StackAllocator &&) = delete;

 public:
  StackAllocator() = default;

  Stack Allocate() {
    if (auto node = TryPop()) [[likely]] {
      return ::std::move(node->stack);
    }

    return AllocateNewStack();
  }

  void Deallocate(Stack &&stack) noexcept {
    UTIL_ASSERT(stack.AllocationSize() != 0, "Stack in moved-from state");

    auto node = ::new (stack.Memory()) Node{.stack = ::std::move(stack)};
    Push(node);
  }

 private:
  [[nodiscard]] Node *TryPop() noexcept {
    auto guard = lock_.MakeGuard();
    return top_ ? ::std::exchange(top_, top_->next) : nullptr;
  }

  void Push(Node *node) noexcept {
    auto guard = lock_.MakeGuard();
    node->next = ::std::exchange(top_, node);
  }

  static Stack AllocateNewStack() {
    constexpr ::std::size_t kStackPages = 16;
    return Stack::AllocatePages(kStackPages);
  }
};

StackAllocator allocator;

} // namespace

Stack AllocateStack() {
  return allocator.Allocate();
}

void DeallocateStack(Stack &&stack) noexcept {
  allocator.Deallocate(::std::move(stack));
}

} // namespace exe::fiber
