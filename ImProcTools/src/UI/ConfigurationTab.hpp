#pragma once

namespace ImProc
{

class ConfigurationTab 
{
  public:
    ConfigurationTab(bool* ShowWindow);

    void DrawWindow();
  private:
    bool* bShowWindow = nullptr;
};
}