#pragma once

// TODO need to implement a way to rotue the login through any service provided by the user
#include <string>
class Frontend
{
  public:
    static Frontend& Get();

    void Init(bool inTerminal = false);

    void Host();
    void Join(const std::string& addr);
};