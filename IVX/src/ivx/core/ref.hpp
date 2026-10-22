#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>

namespace ivx {

#if defined(IVX_INTRUSIVE_REF)
template <typename T>
class Ref {
    // Ref<U> hands its pointer over to Ref<T> on a converting move, which
    // must not touch the count.
    template <typename U>
    friend class Ref;

  public:
    constexpr Ref() noexcept = default;
    constexpr Ref(std::nullptr_t) noexcept {
    }

    explicit Ref(T *ptr) noexcept : ptr_(ptr) {
        add_ref();
    }

    Ref(const Ref &other) noexcept : ptr_(other.ptr_) {
        add_ref();
    }

    Ref(Ref &&other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    template <typename U>
        requires std::convertible_to<U *, T *>
    Ref(const Ref<U> &other) noexcept : ptr_(other.get()) {
        add_ref();
    }

    template <typename U>
        requires std::convertible_to<U *, T *>
    Ref(Ref<U> &&other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    ~Ref() {
        release();
    }

    Ref &operator=(const Ref &other) noexcept {
        if (this != &other) {
            Ref tmp(other);
            swap(tmp);
        }
        return *this;
    }

    Ref &operator=(Ref &&other) noexcept {
        if (this != &other) {
            Ref tmp(std::move(other));
            swap(tmp);
        }
        return *this;
    }

    Ref &operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    T *get() const noexcept {
        return ptr_;
    }

    T *operator->() const noexcept {
        return ptr_;
    }

    T &operator*() const noexcept {
        return *ptr_;
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    bool operator==(T *ptr) const noexcept {
        return ptr_ == ptr;
    }

    template <typename U>
        requires std::convertible_to<U *, T *>
    bool operator==(const Ref<U> &other) const noexcept {
        return ptr_ == other.ptr_;
    }

    void reset(T *ptr = nullptr) noexcept {
        // Re-adopting the pointer that is already owned must be a no-op:
        // releasing it first could destroy the object and leave ptr_
        // dangling.
        if (ptr == ptr_) {
            return;
        }
        release();
        ptr_ = ptr;
        add_ref();
    }

    // T *detach() noexcept {
    //     T *tmp = ptr_;
    //     ptr_ = nullptr;
    //     return tmp;
    // }

    // // Takes over an existing reference without incrementing the count. Only
    // // valid for a reference that no Ref owns at that moment, such as the one
    // // handed out by detach(); adopting get() would put two handles on a
    // single
    // // reference.
    // static Ref adopt(T *ptr) noexcept {
    //     Ref result;
    //     result.ptr_ = ptr;
    //     return result;
    // }

    void swap(Ref &other) noexcept {
        std::swap(ptr_, other.ptr_);
    }

  private:
    void add_ref() {
        if (ptr_) {
            ptr_->add_ref();
        }
    }

    void release() {
        if (ptr_) {
            ptr_->release();
        }
    }

    T *ptr_{nullptr};
};

template <typename T, typename... Args>
    requires std::constructible_from<T, Args...>
[[nodiscard]] Ref<T> make_ref(Args &&...args) {
    return Ref<T>(new T(std::forward<Args>(args)...));
}
#else
template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T, typename... Args>
    requires std::constructible_from<T, Args...>
[[nodiscard]] Ref<T> make_ref(Args &&...args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}
#endif
} // namespace ivx
