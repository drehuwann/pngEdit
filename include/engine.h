#pragma once

#ifdef WIN32
#else  // WIN32
#ifdef POSIX
#else  // POSIX
#error non win non posix not implemenmted yet.
#endif // POSIX  
#endif  // WIN32

#include "singleton.h"
#include "model.h"
#include "view.h"
#include "controller.h"

//forward declarations
class Model;
class Controller;

class Engine final : public Singleton<Engine> {
public:
    ~Engine() override;    
    /// @brief Checks validity of parameters(no nullptr!).
    /// if everything is ok, copy params in Engine private fields.
    /// if private fields were not null, asks for confirmation and free old
    /// objects.
    /// @param modl pointer on the model
    /// @param view pointer on the view
    /// @param ctrl pointer on the controller
    /// @return Error::NONE on success.
    Error Init(Model *modl, View *view, Controller *ctrl);
    Model *GetModel();
    Controller *GetController();
    View *GetView();

private:
    friend class Singleton<Engine>; // Allows Singleton to access private constructor
    Engine();
    Model *m_modl;
    View *m_view;
    Controller *m_ctrl;
};
