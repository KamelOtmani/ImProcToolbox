#pragma once

#include <vector>
#include <string>
#include <filesystem>

namespace ImProc::Utility::FileDialogue
{
std::vector<std::filesystem::path> OpenFile(const char *filter, bool multiselect = false);

std::vector<std::filesystem::path> BrowseFolder();

} // namespace ImProc::Utility::FileDialogue
