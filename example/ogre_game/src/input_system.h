#ifndef INPUT_SYSTEM_FILE
#define INPUT_SYSTEM_FILE

class InputListner
{
public:
    virtual void keyPressed() = 0;
    virtual void keyReleased() = 0;

    virtual void moved() = 0;
};

class MouseInputListner:public InputListner
{
};

class MouseListener
{
public:
    // Receives SDL_MOUSEMOTION and SDL_MOUSEWHEEL events
    virtual void mouseMoved(const SDL_Event& arg) {}
    virtual void mousePressed(const SDL_MouseButtonEvent& arg, Ogre::uint8 id) {}
    virtual void mouseReleased(const SDL_MouseButtonEvent& arg, Ogre::uint8 id) {}
};

class KeyboardListener
{
public:
    virtual void textEditing(const SDL_TextEditingEvent& arg) {}
    virtual void textInput(const SDL_TextInputEvent& arg) {}
    virtual void keyPressed(const SDL_KeyboardEvent& arg) {}
    virtual void keyReleased(const SDL_KeyboardEvent& arg) {}
};

class JoystickListener
{
public:
    virtual void joyButtonPressed(const SDL_JoyButtonEvent& evt, int button) {}
    virtual void joyButtonReleased(const SDL_JoyButtonEvent& evt, int button) {}
    virtual void joyAxisMoved(const SDL_JoyAxisEvent& arg, int axis) {}
    virtual void joyPovMoved(const SDL_JoyHatEvent& arg, int index) {}
};
#endif