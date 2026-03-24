#pragma once

class WindowBase
{
  public:
    virtual ~WindowBase() = default;

    virtual void Draw() = 0;
};