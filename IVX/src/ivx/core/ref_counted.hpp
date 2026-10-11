#pragma once

#include <atomic>
#include <cstdint>

namespace ivx {
#if defined(IVX_INTRUSIVE_REF)
template <typename T>
class Ref;

class RefCounted {
    // Ref<T> is the only handle allowed to manage the count; the friend
    // declaration below grants every Ref<> specialization access to
    // add_ref()/release().
    template <typename U>
    friend class Ref;

  public:
    uint32_t ref_count() const noexcept {
        return ref_count_.load(std::memory_order_relaxed);
    }

  protected:
    RefCounted() = default;

    virtual ~RefCounted() = default;

    RefCounted(const RefCounted &) = delete;

  private:
    void add_ref() const noexcept {
        ref_count_.fetch_add(1, std::memory_order_relaxed);
    }

    void release() const noexcept {
        if (ref_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete this;
        }
    }

    mutable std::atomic<uint32_t> ref_count_{0};
};
#else
class RefCounted {
  public:
    virtual ~RefCounted() = default;
};
#endif
}; // namespace ivx