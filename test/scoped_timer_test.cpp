#include "ivx/core/scoped_timer.h"
#include <iostream>
#include <vector>

using namespace ivx;

int main() {
    std::vector<ProfileResult> prs_;

    {
        PROFILE_SCOPE("for");

        for (int i = 0; i < 100000; i++) {
            int k = i;
            k++;
        }
    }

    {
        PROFILE_SCOPE("while");

        int i = 100000;
        while (i-- > 0) {
            int k = i;
            k++;
        }
    }

    for (auto &&pr : prs_) {
        std::cout << pr.duration << ' ' << pr.label << std::endl;
    }
}