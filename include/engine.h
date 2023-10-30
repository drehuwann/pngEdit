#pragma once

#ifdef WIN32
#else  // WIN32
#ifdef POSIX
#else  // POSIX
#error non win non posix not implemenmted yet.
#endif // POSIX  
#endif  // WIN32

#include "ztools.h"
#include "model.h"
#include "controller.h"

//forward declarations

class Model;
class Controller;


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