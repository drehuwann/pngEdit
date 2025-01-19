#pragma once

#ifdef WIN32
#else  // WIN32
#ifdef POSIX
#else  // POSIX
#error non win non posix not implemenmted yet.
#endif // POSIX  
#endif  // WIN32

#include "model.h"
#include "controller.h"

//forward declarations
class Model;
class Controller;

class Engine {
public:
    Engine();
    ~Engine();
    
    /// @brief Checks validity of parameters(no nullptr!).
    /// if everything is ok, copy params in Engine private fields.
    /// if private fields were not null, asks for confirmation and free old
    /// objects.
    /// @param modl pointer on the model
    /// @param view pointer on the view
    /// @param ctrl pointer on the controller
    /// @return Error::NONE on success.
    Error Init(Model *modl, void *view, Controller *ctrl);
    Model *GetModel();
    Controller *GetController();
    void *GetView();
    
private:
    Model *m_modl;
    void *m_view; //TODO encapsulate this naked ptr in class View
    Controller *m_ctrl;
};
