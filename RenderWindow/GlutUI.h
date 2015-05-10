/** Display.h
 * This header defines with the main functionality of windows and displays.
 *
 * 
**/

#include "stdafx.h"
#include "scene.h"

namespace GlutUI
{
#define DEFAULT_XPOS 10
#define DEFAULT_YPOS 10
#define DEFAULT_WINDOWNAME "A Window"

//TODO Come up with a relative coordinate scheme

/** Global variables **/

/** Class Prototypes **/
class Manager;
class UIElement;
class Window;
class Panel;
class Button;

/** Class Definitions **/

/** Manager Class **/
/** Manages and exposes all the UI elements.
 * There should only ever be one manager.
**/
class Manager
{
public:
    /* Constructors */
    Manager() { }

    void init(int argc, char* argv[]);
    void display();
    void drawElements();
    void setShaders(std::string vertfile, std::string fragfile);
    void deleteShaders();

    Window & createWindow(int width, int height);
    Window & createWindow(int width, int height, std::string name);
    Window & createWindow(int width, int height, int xpos, int ypos, std::string name);

    Panel & createPanel(Window & window, int width, int height, std::string name);
    Panel & createPanel(Window & window, int width, int height, int xpos, int ypos, std::string name);

    Button & createButton(Panel & panel, int width, int height, int xpos, int ypos, std::string name);

private:
    std::vector<Window *> _windows;
    std::vector<UIElement *> _elements;
    GLuint v, f, p;
};

/** UIElement Class **/
/** Abstract class that defines the basic properties of an UI element
 *
**/
class UIElement
{
public:
    /* Constructors */
    UIElement(int width, int height)
        : _xpos(DEFAULT_XPOS), _ypos(DEFAULT_YPOS), _width(width), _height(height), _name(NULL)
        { init(); }
    UIElement(int width, int height, std::string name)
        : _xpos(DEFAULT_XPOS), _ypos(DEFAULT_YPOS), _width(width), _height(height), _name(name)
        { init(); }
    UIElement(int xpos, int ypos, int width, int height, std::string name)
        : _xpos(xpos), _ypos(ypos), _width(width), _height(height), _name(name)
        { init(); }

    
    virtual void init();
    virtual void draw() = 0;
    

    /* getters */
    int getXPos() { return _xpos; } const
    int getYPos() { return _ypos; } const
    int getWidth() { return _width; } const 
    int getHeight() { return _height; } const
    std::string getName() { return _name; } const
    int getId() { return _UIElemId; } const

    std::vector<UIElement *> & getChildren() { return _children; } const


    /* setters */
    void setXPos(int xpos) { _xpos=xpos; }
    void setYPos(int ypos) { _ypos=ypos; }
    void setWidth(int width) { _width=width; }
    void setHeight(int height) { _height=height; }
    void setName(std::string name) { _name=name; }

    void addChildren(UIElement * elem) { _children.push_back(elem); }

    /* Single line functions */
    unsigned int nextId() { return NEXTID++; }

private:
    int _xpos, _ypos,
        _width, _height;
    std::string _name;
    bool _ortho;
    float _opacity;
    unsigned int _UIElemId;
    std::vector<UIElement *> _children;

    
    static int NEXTID;
};


/** Window Class **/
/** Abstract class that defines the basic properties of an UI element
 *
**/
class Window : public UIElement
{
public:
    /* Constructors */
    Window(int width, int height) : UIElement(width, height)
    { setName(DEFAULT_WINDOWNAME); init(); }
    Window(int width, int height, std::string name) : UIElement(width, height, name) { init(); }

    void init();
    void draw();
    void reshape(int w, int h);
    
private:
    static void displayFuncWrapper();
    static void reshapeFuncWrapper(int w, int h);

    bool _init;
};


/** Panel Class **/
/** Panel class that defines a panel areas within the 
 *
**/
class Panel : public UIElement
{
public:
    Panel(int width, int height) : UIElement(width, height), _persp(false)
        { setName("Panel " + getId()); }
    Panel(int width, int height, std::string name) : UIElement(width, height, name), _persp(false)
        { setName("Panel " + getId()); }
    Panel(int width, int height, int xpos, int ypos, std::string name) : UIElement(width, height, xpos, ypos, name), _persp(false)
        { }
    void draw();

    void setWorld(Scene::World * world) { _world = world; }
    Scene::World * getWorld() { return _world; }
    void setCamera(Scene::Camera * camera) { _camera = camera; }

    Scene::Camera * getCamera() { return _camera; }

private:
    std::vector<Button> _buttons;
    bool _persp;
    Scene::World * _world;
    Scene::Camera * _camera;
};

/** Button Class **/
/** Abstract class that defines the basic properties of an UI element
 *
**/
//TODO PRIO: Buttons, and button selection
class Button : public UIElement
{
public:
    Button(int width, int height) : UIElement(width, height), _persp(false)
    { setName("Button " + getId()); }
    Button(int width, int height, int xpos, int ypos) : UIElement(width, height), _persp(false)
    { setName("Button " + getId()); }
    Button(int width, int height, int xpos, int ypos, std::string name) : UIElement(width, height, name), _persp(false)
    {  }
    Button(int width, int height, void * callback(void));
    void draw();

private:
    void * _callback;
    bool _persp;
};


namespace Controls
{
    class Mouse
    {
    public:
        //TODO replace with actual camera?
        Mouse(Panel * panel, Scene::Camera * camera) : _panel(panel), _camera(camera) { init(); }
        void init();

    private:
        Panel * _panel;
        Scene::Camera * _camera;
        int _lastx, _lasty;
        bool _buttons[3];

        void _mouse(int button, int state, int x, int y);
        void _motion(int x, int y);
        //TODO Mousewheel!
        //void mouseWheel(int, int, int, int);
        static void _mouseFuncWrapper(int button, int state, int x, int y);
        static void _motionFuncWrapper(int x, int y);
    };

    class Keyboard
    {
    public:
        Keyboard(Panel * panel) : _panel(panel) { init(); }
        void init();

        void register_hotkey(unsigned char, std::function<void (void)>);
        void register_specialkey(int,  std::function<void (void)>);

    private:
        Panel * _panel;
        std::unordered_map<unsigned char, std::function<void (void)>> hotkey_map;
        std::unordered_map<int, std::function<void (void)>> specialkey_map;

        void _keyPress(unsigned char, int, int);
        void _specialPress(int, int, int);
        static void _keyboardFuncWrapper(unsigned char, int, int);
        static void _keyboardSpecialFuncWrapper(int, int, int);

        static void exitFunc() { exit(0); }
    };
}

};
