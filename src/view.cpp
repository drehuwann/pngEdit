#include "view.h"

View::View(HWND hWnd) : handle(hWnd) {}
View::~View() {}

void View::Render() {}
void View::Resize(int width, int height) {}
