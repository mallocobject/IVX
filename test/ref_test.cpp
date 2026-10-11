#include "catch_amalgamated.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include <iostream>
#include <utility>

using namespace ivx;

class Test : public RefCounted {
  public:
    Test() {
        std::cout << "Test()" << std::endl;
    }

    ~Test() {
        std::cout << "~Test()" << std::endl;
    }

    static Test *create() {
        return new Test;
    }
};

TEST_CASE("ref", "construct") {
    Ref r1(Test::create());
    Ref r2 = r1;
    CHECK(r1->ref_count() == 2);
    r2 = nullptr;
    CHECK(r1->ref_count() == 1);
    Ref r3 = std::move(r1);
    CHECK_FALSE(r1);
    CHECK(r3->ref_count() == 1);
}