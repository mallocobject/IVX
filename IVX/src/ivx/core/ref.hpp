#include <concepts>

namespace ivx {
#if 1

template <typename T>
class Ref {
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
    Ref(Ref<U> &&other) noexcept : ptr_(other.get()) {
        other.detach();
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

    void reset(T *ptr = nullptr) noexcept {
        release();
        if (ptr) {
            ptr_ = ptr;
            add_ref();
        } else {
            ptr_ = nullptr;
        }
    }

    T *detach() noexcept {
        T *tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }

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

  private:
    T *ptr_{nullptr};
};

#else
#include <memory>
template <typename T>
using Ref = std::shared_ptr<T>;

#endif
} // namespace ivx
