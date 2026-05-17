#pragma once

#include <optional>
#include <string>
class WindowBase
{
  public:
    virtual ~WindowBase() = default;

    virtual void Initialize(const std::string& inMenu, bool inVisibility);
    virtual void Draw() = 0;

    void SetVisbility(bool value) { isShowing = value; }

    bool IsShowing() const { return isShowing; }

  protected:
    bool isShowing = true;

  private:
    std::optional<std::string> menu;
};