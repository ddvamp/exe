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
#include <util/intrusive/forward_list.hpp>
#include <util/intrusive/stack.hpp>
#include <util/memory/page_allocation.hpp>
#include <util/memory/view.hpp>

#include <cstddef>
#include <utility>

namespace exe::fiber {

namespace {

class StackAllocator {
 private:
  struct Node : ::util::intrusive_forward_list_node<Node> {
    Stack stack;
  };

  ::util::intrusive_stack<Node> nodes_;
  ::concurrency::QSpinlock lock_; // Protects nodes_

 public:
  ~StackAllocator() {
    while (!nodes_.empty()) {
      nodes_.pop().~Node();
    }
  }

  StackAllocator(StackAllocator const &) = delete;
  void operator= (StackAllocator const &) = delete;

  StackAllocator(StackAllocator &&) = delete;
  void operator= (StackAllocator &&) = delete;

 public:
  StackAllocator() = default;

  Stack Allocate() {
    {
      auto guard = lock_.MakeGuard();
      if (!nodes_.empty()) [[likely]] {
        return ::std::move(nodes_.pop()).stack;
      }
    }

    return AllocateNewStack();
  }

  void Deallocate(Stack &&stack) noexcept {
    UTIL_ASSERT(stack.AllocationSize() != 0, "Stack in moved-from state");

    auto node = ::new (stack.Memory()) Node{.stack = ::std::move(stack)};
    auto guard = lock_.MakeGuard();
    nodes_.push(*node);
  }

 private:
  static Stack AllocateNewStack() {
    constexpr ::std::size_t kStackBytes = 1024 * 1024; // 1MB
    return Stack::AllocateBytes(kStackBytes);
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
