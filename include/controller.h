#pragma once

#include "engine.h"

// forward declarations

class Engine;


class Controller {
public:
    Controller(Engine *engine);
    ~Controller();

    Engine *GetEngine();

private:
    Engine *eng;
};