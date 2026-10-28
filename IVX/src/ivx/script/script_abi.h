#pragma once

// Fingerprint of everything a script module and the engine share *by layout*.
//
// The script module is compiled independently: it includes the engine headers
// and instantiates the same templates (get_component<T>(), the ScriptableEntity
// vtable) against the same types the host uses. Renaming a virtual function is
// binary compatible (virtual calls are slot indices, names never reach the
// binary), but adding a member, reordering the vtable or changing the ScriptApi
// table is not: the module then reads the wrong offsets and the process dies
// somewhere inside the DLL.
//
// Both sides compute the value below from this header, so a mismatch means
// exactly one of them is stale. The host refuses to load such a module instead
// of running code that corrupts memory (see ScriptLibrary::load).

#include "ivx/scene/components.h"
#include "ivx/scene/scriptable_entity.h"
#include "ivx/script/script_module.h"

#include <cstdint>
#include <cstdio>
#include <string>

namespace ivx {

inline std::uint64_t script_abi_hash() {
    std::uint64_t hash = 0xcbf29ce484222325ull; // FNV-1a, 64 bit
    const auto mix = [&hash](std::uint64_t value) {
        hash ^= value;
        hash *= 0x100000001b3ull;
    };
    const auto mix_type = [&mix](std::uint64_t size, std::uint64_t alignment) {
        mix(size);
        mix(alignment);
    };

    mix(kScriptModuleVersion);

    // Types that actually cross the boundary, either as data or through the ABI
    // tables.
    mix_type(sizeof(ScriptableEntity), alignof(ScriptableEntity));
    mix_type(sizeof(ScriptApi), alignof(ScriptApi));
    mix_type(sizeof(ScriptRegistrar), alignof(ScriptRegistrar));
    mix_type(sizeof(Entity), alignof(Entity));
    mix_type(sizeof(entt::entity), alignof(entt::entity));

    // Every component a script may reach through get_component<T>(), from the
    // single list in components.h so a new one cannot be forgotten here
    // (IdComponent was).
#define IVX_ABI_MIX_TYPE(Type) mix_type(sizeof(Type), alignof(Type));
    IVX_ABI_COMPONENT_LIST(IVX_ABI_MIX_TYPE)
#undef IVX_ABI_MIX_TYPE

    // Settings that change layouts without changing any of the sizes above: Ref
    // is 8 bytes with the intrusive counter and 16 with shared_ptr, and the CRT
    // alters every standard container.
#if defined(IVX_INTRUSIVE_REF)
    mix(1);
#else
    mix(2);
#endif
#if defined(IVX_CONFIG_DEBUG)
    mix(0x10);
#elif defined(IVX_CONFIG_RELEASE)
    mix(0x20);
#else
    mix(0x30);
#endif
#if defined(_MSC_VER)
    mix(static_cast<std::uint64_t>(_MSC_VER));
#else
    mix(0);
#endif

    return hash;
}

// Hex string: what the module exports and the host compares against.
inline std::string script_abi_fingerprint() {
    char buffer[32];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "%016llx",
                  static_cast<unsigned long long>(script_abi_hash()));
    return buffer;
}

} // namespace ivx
