#pragma once

namespace ivx {
class FileDialog {
  public:
    static std::filesystem::path open_file(std::string_view filter);
    static std::filesystem::path save_file(std::string_view filter);
};
} // namespace ivx
