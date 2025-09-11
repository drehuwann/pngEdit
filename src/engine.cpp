#include "engine.h"

Engine::Engine() : m_modl(nullptr), m_view(nullptr), m_ctrl(nullptr) {}

Engine::~Engine() {
    if (m_modl) delete m_modl;
    if (m_ctrl) delete m_ctrl;
    if (m_view) delete m_view;
}

Error Engine::Init(Model *modl, View *view, Controller *ctrl) {
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

View *Engine::GetView() {
    return this->m_view;
}
