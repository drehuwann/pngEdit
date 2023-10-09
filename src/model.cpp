#include "model.h"

Model::Model() {
    m_file = new PngFile();
}

Model::~Model() {}

void Model::PickFile(const char *path) {
    if (m_file->Pick(path) <= 0) throw;
}
