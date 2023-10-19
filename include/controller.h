#pragma once

#include "engine.h"
class Engine; // fwd decl
class Controller {
public:
    Controller(Engine *engine);
    ~Controller();

    Engine *GetEngine();

private:
    Engine *eng;
};