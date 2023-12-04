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

    /// @brief Runs ztools tests and checks validity of parameters(no nullptr!).
    /// if everything is ok, copy params in Engine private fields.
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
   void *m_view;
   Controller *m_ctrl;
};