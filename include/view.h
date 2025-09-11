#pragma once

#include "utf4win.h"
#include "defs.h"

class View {
public:
    explicit View(HWND hWnd);
    ~View();
    void Render();
    void Resize(int width, int height);

private:
    HWND handle;
};
