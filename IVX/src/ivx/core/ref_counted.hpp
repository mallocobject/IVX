#pragma once

#include <atomic>
#include <cstdint>

namespace ivx {
#if 1
class RefCounted {
  public:
    void add_ref() const noexcept {
        ref_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void release() const noexcept {
        if (ref_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete this;
        }
    }

    uint32_t ref_count() const noexcept {
        return ref_count_.load(std::memory_order_relaxed);
    }

  protected:
    RefCounted() = default;

    virtual ~RefCounted() = default;

    RefCounted(const RefCounted &) = delete;

  private:
    mutable std::atomic<uint32_t> ref_count_{0};
};
#else
class RefCounted {};
#endif
}; // namespace ivx