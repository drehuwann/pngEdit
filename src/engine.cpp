#include "engine.h"

Engine::Engine() : m_modl(nullptr), m_view(nullptr), m_ctrl(nullptr) {}

Engine::~Engine() {}

Error Engine::Init(Model *modl, void *view, Controller *ctrl) {
    if (TestZtools()) return Error::ZTOOLSTESTFAILED;
    if (! modl || ! view || ! ctrl) return Error::NOTINITIALIZED;
    m_modl = modl;
    m_view = view;
    m_ctrl = ctrl;
    return Error::NONE;
}

Model *Engine::GetModel() {
    return this->m_modl;
}

Controller *Engine::GetController() {
    return this->m_ctrl;
}

void *Engine::GetView() {
    return this->m_view;
}
