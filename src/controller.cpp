#include "controller.h"

Controller::Controller(Engine *engine) : eng(engine) {}

Controller::~Controller() {}

Engine *Controller::GetEngine() {
    return this->eng;
}
