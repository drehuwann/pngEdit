#pragma once

#include "engine.h"
class Engine; // fwd decl
class Controller {
public:
    Controller(const Engine &engine);
    ~Controller();

    const Engine &GetEngine();

private:
    const Engine &eng;
};