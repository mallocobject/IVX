#include "ivxpch.h"
#include "input.h"

#if defined(IVX_PLATFORM_WINDOWS)
#include "platform/windows/win_input.h"
#endif

namespace ivx {
Ref<Input> Input::create() {
#ifdef IVX_PLATFORM_WINDOWS
    return make_ref<WinInput>();
#else
    HZ_CORE_ASSERT(false, "Unknown platform!");
    return nullptr;
#endif
}
} // namespace ivx