#include "controller.h"

Controller::Controller(const Engine &engine) : eng(engine) {}

Controller::~Controller() {}

const Engine &Controller::GetEngine() {
    return this->eng;
}
