#include "FileDialogue.h"

#include <GLFW\glfw3.h>
#include <Windows.h>
#include <commdlg.h>
#include <filesystem>
#define GLFW_EXPOSE_NATIVE_WIN32
#include "Walnut/Application.h"
#include <GLFW/glfw3native.h>
#include <iostream>
#include <shlobj.h>
#include <sstream>
#include <string>
#include <windows.h>

using std::filesystem::path;
namespace fs = std::filesystem;

namespace ImProc::Utility::FileDialogue
{
std::vector<path> OpenFile(const char *filter, bool multiselect)
{
    OPENFILENAMEA ofn;
    CHAR szFile[1024] = {0};
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = glfwGetWin32Window(
        (GLFWwindow *)Walnut::Application::Get().GetWindow());
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (multiselect)
        ofn.Flags = ofn.Flags | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        std::vector<path> filePaths;
        // return ofn.lpstrFile;
        auto *p = ofn.lpstrFile;
        std::string path = p;
        p += path.size() + 1;
        if (*p == 0)
        {
            // there is only one string, being the full path to the file
            // filePaths.push_back(path.c_str());
            filePaths.emplace_back(path);
        }
        else
        {
            // multiple files follow the directory
            for (; *p != 0;)
            {
                std::string fileName = p;
                // filePaths.push_back((path + L"\\" + fileName).c_str());
                filePaths.emplace_back(path + "\\" + fileName);
                p += fileName.size() + 1;
            }
        }
        return filePaths;
    }
    return {path()};
}

/*
 * Returns the list of folders that are contained in the selcted folder
 */
std::vector<std::filesystem::path> BrowseFolder()
{
    std::vector<fs::path> paths{};
    WCHAR path[MAX_PATH + 1]{};
    BROWSEINFO bi{};

    bi.hwndOwner = glfwGetWin32Window(
        (GLFWwindow *)Walnut::Application::Get().GetWindow());
    bi.pidlRoot = NULL;
    bi.pszDisplayName = (path); // This is just for display: not useful
    bi.lpszTitle = L"Choose Client Directory";
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lpfn = NULL;
    bi.lParam = 0;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != NULL && SHGetPathFromIDList(pidl, path))
    {
        // auto paths = fs::path{path};
        auto dirs = fs::directory_iterator(path);
        for (const auto &dir : dirs)
        {
            if (fs::is_directory(dir))
            {
                paths.push_back(dir);
            }
        }
    }
    return paths;
}

} // namespace ImProc::Utility::FileDialogue
