#pragma once

#include <Windows.h>
#include <commdlg.h>
#include <filesystem>
#include <GLFW\glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "Walnut/Application.h"

using std::filesystem::path;

namespace Utility
{
	namespace FileDialogue
	{
		std::vector<path> OpenFile(const char* filter,bool multiselect = false)
		{
			OPENFILENAMEA ofn;
			CHAR szFile[1024] = { 0 };
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Walnut::Application::Get().GetWindow());
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
			if (multiselect )
				ofn.Flags = ofn.Flags | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
			if (GetOpenFileNameA(&ofn) == TRUE)
			{
				std::vector<path> filePaths;
				//return ofn.lpstrFile;
				auto* p = ofn.lpstrFile;
				std::string path = p;
				p += path.size() + 1;
				if (*p == 0)
				{
					// there is only one string, being the full path to the file
					//filePaths.push_back(path.c_str());
					filePaths.push_back(path);
				}
				else
				{
					// multiple files follow the directory
					for (; *p != 0; )
					{
						std::string fileName = p;
						//filePaths.push_back((path + L"\\" + fileName).c_str());
						filePaths.push_back(path + "\\" + fileName);
						p += fileName.size() + 1;
					}
				}
				return filePaths;
			}
			return {path()};
		}

	}
}