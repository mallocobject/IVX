// Tests for ivx::Ref, an intrusive smart pointer, and for the ivx::RefCounted
// base class that stores the reference count inside the object.
//
// The instrumentation below reports construction/destruction of the payload
// type, so a leaked, double-deleted or wrongly shared object fails a check
// instead of only showing up as a crash later on.

#include "catch_amalgamated.hpp"

// Included twice on purpose: both headers must survive multiple inclusion.
// clang-format off
#include "ivx/core/ref.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include "ivx/core/ref_counted.hpp"
// clang-format on

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

using namespace ivx;

namespace {

// Payload that tracks how many instances are alive and how often it has been
// constructed/destroyed.
class Tracked : public RefCounted {
  public:
    explicit Tracked(int value = 0) : value_(value) {
        ++alive_;
        ++constructed_;
    }

    ~Tracked() override {
        --alive_;
        ++destroyed_;
    }

    int value() const {
        return value_;
    }

    static Tracked *create(int value = 0) {
        return new Tracked(value);
    }

    static void reset_counters() {
        alive_ = 0;
        constructed_ = 0;
        destroyed_ = 0;
    }

    static int alive() {
        return alive_.load();
    }

    static int constructed() {
        return constructed_.load();
    }

    static int destroyed() {
        return destroyed_.load();
    }

  private:
    int value_{0};

    inline static std::atomic<int> alive_{0};
    inline static std::atomic<int> constructed_{0};
    inline static std::atomic<int> destroyed_{0};
};

// Two-level hierarchy used to check polymorphic conversion and destruction.
class Shape : public RefCounted {
  public:
    ~Shape() override {
        ++destroyed_shapes;
    }

    virtual const char *name() const {
        return "shape";
    }

    inline static int destroyed_shapes = 0;
};

class Circle : public Shape {
  public:
    ~Circle() override {
        ++destroyed_circles;
    }

    const char *name() const override {
        return "circle";
    }

    inline static int destroyed_circles = 0;
};

// Detects whether add_ref()/release() are reachable from the outside: they
// must stay hidden behind access control, with Ref as the only friend.
template <typename T, typename = void>
struct add_ref_is_public : std::false_type {};

template <typename T>
struct add_ref_is_public<
    T,
    std::void_t<decltype(std::declval<const T &>().add_ref())>>
    : std::true_type {};

template <typename T, typename = void>
struct release_is_public : std::false_type {};

template <typename T>
struct release_is_public<
    T,
    std::void_t<decltype(std::declval<const T &>().release())>>
    : std::true_type {};

// Positive control: a type that does expose the pair publicly must be
// detected, otherwise the checks below could pass for the wrong reason.
struct PubliclyCounted {
    void add_ref() const noexcept {
    }

    void release() const noexcept {
    }
};

} // namespace

TEST_CASE("Ref satisfies its static contract", "[ref][traits]") {
    // Moving/copying a handle never throws: it only touches its own pointer
    // and the count stored in the object.
    STATIC_REQUIRE(std::is_nothrow_default_constructible_v<Ref<Tracked>>);
    STATIC_REQUIRE(std::is_nothrow_copy_constructible_v<Ref<Tracked>>);
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<Ref<Tracked>>);
    STATIC_REQUIRE(std::is_nothrow_copy_assignable_v<Ref<Tracked>>);
    STATIC_REQUIRE(std::is_nothrow_move_assignable_v<Ref<Tracked>>);
    STATIC_REQUIRE(std::is_nothrow_destructible_v<Ref<Tracked>>);
    STATIC_REQUIRE(
        std::is_nothrow_constructible_v<Ref<Tracked>, std::nullptr_t>);
    STATIC_REQUIRE(std::is_nothrow_constructible_v<Ref<Tracked>, Tracked *>);

    // Taking ownership of a raw pointer must be explicit ...
    STATIC_REQUIRE_FALSE(std::is_convertible_v<Tracked *, Ref<Tracked>>);
    // ... while adopting a null pointer may be implicit.
    STATIC_REQUIRE(std::is_convertible_v<std::nullptr_t, Ref<Tracked>>);

    // A handle is exactly one pointer wide: no control block, no allocation.
    STATIC_REQUIRE(sizeof(Ref<Tracked>) == sizeof(Tracked *));

    // Upcasts are implicit, downcasts are not.
    STATIC_REQUIRE(std::is_convertible_v<Ref<Circle>, Ref<Shape>>);
    STATIC_REQUIRE_FALSE(std::is_convertible_v<Ref<Shape>, Ref<Circle>>);

    // Derived-to-base conversion also works for assignment, in both the
    // copy and the move flavour.
    STATIC_REQUIRE(std::is_assignable_v<Ref<Shape> &, const Ref<Circle> &>);
    STATIC_REQUIRE(std::is_assignable_v<Ref<Shape> &, Ref<Circle> &&>);
}

TEST_CASE("RefCounted has an intrusive, non-copyable interface",
          "[ref_counted][traits]") {
    // Destruction always goes through the virtual destructor, so releasing
    // the last reference to a derived object through a base handle is safe.
    STATIC_REQUIRE(std::has_virtual_destructor_v<RefCounted>);

    // Only derived types may be created, and instances are never copied.
    STATIC_REQUIRE_FALSE(std::is_default_constructible_v<RefCounted>);
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<RefCounted>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<RefCounted>);
    STATIC_REQUIRE_FALSE(std::is_move_constructible_v<RefCounted>);
    STATIC_REQUIRE_FALSE(std::is_move_assignable_v<RefCounted>);

    // The counting interface is const and returns the raw count.
    STATIC_REQUIRE(
        std::is_same_v<decltype(std::declval<const RefCounted &>().ref_count()),
                       std::uint32_t>);
}

TEST_CASE("Ref is empty by default and from nullptr", "[ref][construction]") {
    Tracked::reset_counters();

    Ref<Tracked> empty;
    CHECK(empty.get() == nullptr);
    CHECK_FALSE(empty);

    Ref<Tracked> null(nullptr);
    CHECK(null.get() == nullptr);
    CHECK_FALSE(null);

    Ref<Tracked> assigned = nullptr;
    CHECK_FALSE(assigned);

    // Adopting a null raw pointer must not touch any object.
    Ref<Tracked> from_raw(static_cast<Tracked *>(nullptr));
    CHECK_FALSE(from_raw);

    // Destroying empty handles must not create or destroy anything.
    CHECK(Tracked::constructed() == 0);
    CHECK(Tracked::alive() == 0);
    CHECK(Tracked::destroyed() == 0);
}

TEST_CASE("Ref adopts a freshly created object with count one",
          "[ref][adopt]") {
    Tracked::reset_counters();

    {
        Ref<Tracked> ref(Tracked::create(41));

        CHECK(Tracked::constructed() == 1);
        CHECK(Tracked::alive() == 1);
        CHECK(Tracked::destroyed() == 0);
        CHECK(ref->ref_count() == 1);

        CHECK(ref.get() != nullptr);
        CHECK(ref->value() == 41);
        CHECK((*ref).value() == 41);
        CHECK(static_cast<bool>(ref));
    } // releasing the last reference destroys the object

    CHECK(Tracked::destroyed() == 1);
    CHECK(Tracked::alive() == 0);
}

TEST_CASE("Ref adopts a pointer that is already owned", "[ref][adopt]") {
    Tracked::reset_counters();

    Tracked *raw = Tracked::create(1);
    CHECK(raw->ref_count() == 0); // a fresh object carries no reference yet
    {
        Ref<Tracked> owner(raw);
        CHECK(raw->ref_count() == 1);
        CHECK(owner.get() == raw);

        {
            // Taking a second reference is the only way to add one now that
            // add_ref() is hidden; handles and count grow together.
            Ref<Tracked> adopted(owner.get());
            CHECK(adopted.get() == raw);
            CHECK(owner.get() == raw);
            CHECK(raw->ref_count() == 2);
        }
        CHECK(raw->ref_count() == 1);
        CHECK(Tracked::destroyed() == 0);
    }

    CHECK(Tracked::destroyed() == 1);
    CHECK(Tracked::alive() == 0);
}

TEST_CASE("Ref copy shares ownership", "[ref][copy]") {
    Tracked::reset_counters();

    Tracked *raw = Tracked::create(7);
    {
        Ref<Tracked> owner(raw);
        CHECK(raw->ref_count() == 1);

        Ref<Tracked> copy(owner);
        CHECK(copy.get() == raw);
        CHECK(raw->ref_count() == 2);

        Ref<Tracked> copy_assigned;
        copy_assigned = owner;
        CHECK(copy_assigned.get() == raw);
        CHECK(raw->ref_count() == 3);

        // Copying an empty handle changes nothing.
        Ref<Tracked> empty;
        Ref<Tracked> empty_copy(empty);
        CHECK_FALSE(empty_copy);
        empty_copy = empty;
        CHECK_FALSE(empty_copy);
        CHECK(raw->ref_count() == 3);

        // Self copy assignment must leave the count untouched.
        Ref<Tracked> &alias = copy_assigned;
        copy_assigned = alias;
        CHECK(copy_assigned.get() == raw);
        CHECK(raw->ref_count() == 3);

        copy_assigned = empty;
        CHECK_FALSE(copy_assigned);
        CHECK(raw->ref_count() == 2);
    }

    CHECK(Tracked::destroyed() == 1);
    CHECK(Tracked::alive() == 0);
}

TEST_CASE("Ref assignment releases the previous object exactly once",
          "[ref][assignment]") {
    Tracked::reset_counters();

    Tracked *first = Tracked::create(1);
    Tracked *second = Tracked::create(2);

    {
        Ref<Tracked> ref(first);
        Ref<Tracked> other(second);
        CHECK(first->ref_count() == 1);
        CHECK(second->ref_count() == 1);

        ref = other; // releases `first`
        CHECK(Tracked::destroyed() == 1);
        CHECK(Tracked::alive() == 1);
        CHECK(ref.get() == second);
        CHECK(second->ref_count() == 2);

        ref = nullptr; // releases one reference to `second`
        CHECK_FALSE(ref);
        CHECK(second->ref_count() == 1);
        CHECK(Tracked::alive() == 1);
    }

    CHECK(Tracked::destroyed() == 2);
    CHECK(Tracked::alive() == 0);
}

TEST_CASE("Ref move transfers ownership without touching the count",
          "[ref][move]") {
    Tracked::reset_counters();

    Tracked *raw = Tracked::create(9);
    {
        Ref<Tracked> source(raw);
        CHECK(raw->ref_count() == 1);

        Ref<Tracked> moved(std::move(source));
        CHECK_FALSE(source);
        CHECK(source.get() == nullptr);
        CHECK(moved.get() == raw);
        CHECK(raw->ref_count() == 1); // a move never touches the count

        Ref<Tracked> move_assigned;
        move_assigned = std::move(moved);
        CHECK_FALSE(moved);
        CHECK(move_assigned.get() == raw);
        CHECK(raw->ref_count() == 1);

        // Self move assignment must leave the handle usable.
        Ref<Tracked> &alias = move_assigned;
        move_assigned = std::move(alias);
        CHECK(move_assigned.get() == raw);
        CHECK(raw->ref_count() == 1);

        // A move-assignment releases the object it used to own.
        Ref<Tracked> replaced(Tracked::create(10));
        replaced = std::move(move_assigned);
        CHECK(Tracked::destroyed() == 1);
        CHECK(replaced.get() == raw);
        CHECK(raw->ref_count() == 1);

        // Moving out of an empty handle yields another empty handle.
        CHECK_FALSE(move_assigned);
        Ref<Tracked> from_empty(std::move(move_assigned));
        CHECK_FALSE(from_empty);
        Ref<Tracked> still_empty(std::move(from_empty));
        CHECK_FALSE(still_empty);
        CHECK(raw->ref_count() == 1);
    }

    CHECK(Tracked::destroyed() == 2);
    CHECK(Tracked::alive() == 0);
}

TEST_CASE("Ref reset adopts new pointers and releases old ones",
          "[ref][reset]") {
    Tracked::reset_counters();

    Tracked *first = Tracked::create(1);
    Tracked *second = Tracked::create(2);

    Ref<Tracked> ref(first);
    CHECK(first->ref_count() == 1);

    ref.reset(second); // releases `first`
    CHECK(Tracked::destroyed() == 1);
    CHECK(Tracked::alive() == 1);
    CHECK(ref.get() == second);
    CHECK(second->ref_count() == 1);

    ref.reset(); // the default argument clears the handle
    CHECK(Tracked::destroyed() == 2);
    CHECK(Tracked::alive() == 0);
    CHECK_FALSE(ref);

    ref.reset(); // resetting an empty handle is a no-op
    ref.reset(nullptr);
    CHECK(Tracked::destroyed() == 2);
    CHECK(Tracked::alive() == 0);
}

TEST_CASE("Ref reset must not destroy the object it already owns",
          "[ref][reset][regression]") {
    // Regression test: reset() used to release the current reference first,
    // which destroyed the object and then re-adopted a dangling pointer.
    Tracked::reset_counters();

    Ref<Tracked> ref(Tracked::create(5));
    Tracked *raw = ref.get();

    ref.reset(raw);
    CHECK(ref.get() == raw);
    CHECK(raw->ref_count() == 1);
    CHECK(raw->value() == 5);
    CHECK(Tracked::alive() == 1);
    CHECK(Tracked::destroyed() == 0);

    ref.reset(ref.get()); // same through the accessor
    CHECK(ref.get() == raw);
    CHECK(raw->ref_count() == 1);
    CHECK(Tracked::destroyed() == 0);

    ref = nullptr;
    CHECK(Tracked::destroyed() == 1);
    CHECK(Tracked::alive() == 0);
}

TEST_CASE("Ref swaps handles without touching the counts", "[ref][swap]") {
    Tracked::reset_counters();

    Tracked *first = Tracked::create(1);
    Tracked *second = Tracked::create(2);
    {
        Ref<Tracked> a(first);
        Ref<Tracked> b(second);

        a.swap(b);
        CHECK(a.get() == second);
        CHECK(b.get() == first);
        CHECK(first->ref_count() == 1);
        CHECK(second->ref_count() == 1);

        std::swap(a, b);
        CHECK(a.get() == first);
        CHECK(b.get() == second);
        CHECK(first->ref_count() == 1);
        CHECK(second->ref_count() == 1);
    }

    CHECK(Tracked::destroyed() == 2);
    CHECK(Tracked::alive() == 0);
}

TEST_CASE("Ref converts between derived and base handles",
          "[ref][polymorphism]") {
    Shape::destroyed_shapes = 0;
    Circle::destroyed_circles = 0;

    {
        Ref<Circle> circle(new Circle);
        CHECK(circle->ref_count() == 1);
        CHECK(circle->name() == std::string("circle"));

        Ref<Shape> shape(circle); // converting copy construction
        CHECK(shape.get() == circle.get());
        CHECK(shape->ref_count() == 2);
        CHECK(circle->ref_count() == 2);

        Ref<Shape> moved(std::move(circle)); // converting move construction
        CHECK_FALSE(circle);
        CHECK(moved.get() == shape.get());
        CHECK(shape->ref_count() == 2);

        Ref<Shape> assigned;
        assigned = shape; // converting copy assignment
        CHECK(assigned.get() == shape.get());
        CHECK(shape->ref_count() == 3);

        assigned = std::move(moved); // converting move assignment
        CHECK_FALSE(moved);
        CHECK(assigned.get() == shape.get());
        CHECK(shape->ref_count() == 2);
    }

    // The virtual destructor runs the most derived destructor exactly once,
    // no matter which handle type is dropped last.
    CHECK(Circle::destroyed_circles == 1);
    CHECK(Shape::destroyed_shapes == 1);
}

TEST_CASE("Ref behaves inside standard containers", "[ref][container]") {
    Tracked::reset_counters();

    {
        std::vector<Ref<Tracked>> handles;
        for (int i = 0; i < 4; ++i) {
            handles.emplace_back(Tracked::create(i));
        }
        CHECK(Tracked::alive() == 4);
        for (auto &handle : handles) {
            CHECK(handle->ref_count() == 1);
        }

        std::vector<Ref<Tracked>> copies = handles; // every element is shared
        for (auto &handle : handles) {
            CHECK(handle->ref_count() == 2);
        }

        handles.clear(); // dropping one reference keeps the objects alive
        CHECK(Tracked::alive() == 4);
        CHECK(Tracked::destroyed() == 0);
        CHECK(copies.front()->value() == 0);
        CHECK(copies.front()->ref_count() == 1);

        copies.erase(copies.begin());
        CHECK(Tracked::destroyed() == 1);
        CHECK(Tracked::alive() == 3);

        std::vector<Ref<Tracked>> moved = std::move(copies);
        CHECK(moved.size() == 3);
        CHECK(Tracked::alive() == 3);
        CHECK(Tracked::destroyed() == 1);
    }

    CHECK(Tracked::destroyed() == 4);
    CHECK(Tracked::alive() == 0);
}

TEST_CASE("RefCounted keeps add_ref and release out of the public API",
          "[ref_counted][traits]") {
    // Taking or dropping a reference by hand is not part of the interface:
    // both stay behind access control and friendship with Ref.
    STATIC_REQUIRE_FALSE(add_ref_is_public<RefCounted>::value);
    STATIC_REQUIRE_FALSE(release_is_public<RefCounted>::value);

    // ... and they are out of reach through a derived type as well.
    STATIC_REQUIRE_FALSE(add_ref_is_public<Tracked>::value);
    STATIC_REQUIRE_FALSE(release_is_public<Tracked>::value);

    // Control: the same detector does fire on a public interface, so the
    // negative results above are meaningful.
    STATIC_REQUIRE(add_ref_is_public<PubliclyCounted>::value);
    STATIC_REQUIRE(release_is_public<PubliclyCounted>::value);
}

TEST_CASE("Ref releases through a base-class handle", "[ref_counted]") {
    Shape::destroyed_shapes = 0;
    Circle::destroyed_circles = 0;

    {
        const Ref<Shape> shape(new Circle); // const handle, base-class type
        CHECK(shape->ref_count() == 1);
        {
            Ref<Shape> alias(shape);
            CHECK(shape->ref_count() == 2);
        }
        CHECK(shape->ref_count() == 1);
        CHECK(Circle::destroyed_circles == 0);
    } // the last reference is dropped through the base-class interface

    CHECK(Circle::destroyed_circles == 1);
    CHECK(Shape::destroyed_shapes == 1);
}

TEST_CASE("RefCounted survives concurrent reference traffic",
          "[ref_counted][thread]") {
    constexpr std::uint32_t kThreads = 8;
    constexpr int kIterations = 20000;

    Tracked::reset_counters();

    {
        Ref<Tracked> owner(Tracked::create(1));
        std::atomic<bool> broken{false};

        std::vector<std::thread> workers;
        workers.reserve(kThreads);
        for (std::uint32_t i = 0; i < kThreads; ++i) {
            workers.emplace_back([&owner, &broken] {
                for (int n = 0; n < kIterations; ++n) {
                    Ref<Tracked> copy(owner); // add_ref
                    if (copy->ref_count() < 2) {
                        broken = true;
                    }
                    Ref<Tracked> moved(std::move(copy)); // no count change
                    if (!moved) {
                        broken = true;
                    }
                } // release
            });
        }
        for (auto &worker : workers) {
            worker.join();
        }

        CHECK_FALSE(broken.load());
        CHECK(owner->ref_count() == 1);
        CHECK(Tracked::alive() == 1);
        CHECK(Tracked::destroyed() == 0);
    }

    CHECK(Tracked::destroyed() == 1);
    CHECK(Tracked::alive() == 0);
}

TEST_CASE("RefCounted deletes the object exactly once under contention",
          "[ref_counted][thread]") {
    constexpr std::uint32_t kThreads = 16;

    Tracked::reset_counters();

    Tracked *raw = Tracked::create(1);
    std::vector<Ref<Tracked>> handles;
    handles.reserve(kThreads);
    for (std::uint32_t i = 0; i < kThreads; ++i) {
        handles.emplace_back(raw);
    }
    CHECK(raw->ref_count() == kThreads);

    // Every thread drops its own reference; exactly one of them must run the
    // destructor, and no reference may be lost on the way.
    std::vector<std::thread> workers;
    workers.reserve(kThreads);
    for (auto &handle : handles) {
        workers.emplace_back(
            [ref = std::move(handle)]() mutable { ref = nullptr; });
    }
    for (auto &worker : workers) {
        worker.join();
    }
    handles.clear();

    CHECK(Tracked::destroyed() == 1);
    CHECK(Tracked::alive() == 0);
}
