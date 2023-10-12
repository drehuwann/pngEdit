#include "model.h"

Model::Model(const Engine &engine) : eng(engine), m_info(nullptr),
        pal((Palette)nullptr) {
    m_file = new PngFile();
}

Model::~Model() {
    if (m_file) delete m_file;
    if (m_info) delete m_info;
    m_file = nullptr;
}

const Engine &Model::GetEngine() {
    return this->eng;
}

void Model::SetInfo(s_imInfo *infoPtr) {
    this->m_info = infoPtr;
}

void Model::PickFile(const char *path) {
    m_file->SetModel(this);
    if (m_file->Pick(path) < 0) throw;
}
