#pragma once


#ifdef WIN32
#else  // WIN32
#ifdef POSIX
#else  // POSIX
#error non win non posix not implemenmted yet.
#endif // POSIX  
#endif  // WIN32
#include "model.h"
class Model; //fwd declaration
#include "controller.h"

class Engine {
public:
    Engine();
    ~Engine();
    Error Init(Model *modl, void *view, Controller *ctrl);
    Model *GetModel();
    Controller *GetController();
    void *GetView();
    
private:
   Model *m_modl;
   void *m_view;
   Controller *m_ctrl;
};