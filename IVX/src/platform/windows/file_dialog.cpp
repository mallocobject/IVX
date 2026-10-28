#include "ivxpch.h"
#include "ivx/utils/file_dialog.h"
#include "ivx/core/application.h"
#include <SDL3/SDL.h>
#include <commdlg.h>
#include <windows.h>

namespace ivx {
namespace {
static HWND GetNativeWindowHandle(SDL_Window *window) {
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    return static_cast<HWND>(SDL_GetPointerProperty(
        props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
}
} // namespace

std::filesystem::path FileDialog::open_file(std::string_view filter) {
    OPENFILENAMEA ofn;
    CHAR szFile[260] = {0};
    CHAR currentDir[256] = {0};
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = GetNativeWindowHandle(reinterpret_cast<SDL_Window *>(
        Application::get().get_window().get_native_window()));
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    if (GetCurrentDirectoryA(256, currentDir)) {
        ofn.lpstrInitialDir = currentDir;
    }
    ofn.lpstrFilter = filter.data();
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return ofn.lpstrFile;
    }

    return {};
}

std::filesystem::path FileDialog::save_file(std::string_view filter) {
    OPENFILENAMEA ofn;
    CHAR szFile[260] = {0};
    CHAR currentDir[256] = {0};
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = GetNativeWindowHandle(reinterpret_cast<SDL_Window *>(
        Application::get().get_window().get_native_window()));
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    if (GetCurrentDirectoryA(256, currentDir))
        ofn.lpstrInitialDir = currentDir;
    ofn.lpstrFilter = filter.data();
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    // format as "Scene Files\0*.ivx\0\0"
    const char *ext = strchr(filter.data(), '\0') + 1; // "*.ivx"
    if (ext[0] == '*' && ext[1] == '.') {
        ext += 2;
    }
    ofn.lpstrDefExt = ext;

    if (GetSaveFileNameA(&ofn) == TRUE) {
        return ofn.lpstrFile;
    }

    return {};
}
} // namespace ivx
