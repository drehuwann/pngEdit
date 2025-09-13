#pragma once

#include "defs.h"

enum class MenuID {
    ID_Save = 1, ID_Load, ID_Info, ID_Layo, ID_Exit, ID_Undo, ID_Redo, ID_Abou
};

class View {
public:
    explicit View(HWND hWnd);
    ~View();
    void Render();
    void Resize(int width, int height);

private:
    HWND handle;
};
