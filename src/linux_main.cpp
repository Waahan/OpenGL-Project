#include <iostream>
#include <print>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glext.h>

struct XlibData
{
    Display* display;
    XVisualInfo* visual;
    Colormap colormap;
    Window window;
    Atom deleteWindowMessage;
    GLXContext context;

    ~XlibData()
    {
        glXDestroyContext(display, context);
        XDestroyWindow(display, window);
        XFreeColormap(display, colormap);
        XFree(visual);
        XCloseDisplay(display);
    }
};

XlibData initXlibGlx()
{
    XlibData data;
    data.display = XOpenDisplay(NULL);

    if(!data.display)
    {
        std::println(std::cerr, "Couldn't connect to x11 server, display name: {}", XDisplayName(NULL));
        exit(-1);
    }

    std::println("Xorg Library: xlib");
    std::println("Xorg Protocol: {}.{}", XProtocolVersion(data.display), ProtocolRevision(data.display));

    int screen = DefaultScreen(data.display);

    Window rootWindow = DefaultRootWindow(data.display);

    {
        int GLXMajorVersion;
        int GLXMinorVersion;
        glXQueryVersion(data.display, &GLXMajorVersion, &GLXMinorVersion);
        std::println("GLX version: {}.{}", GLXMajorVersion, GLXMinorVersion);
    }

    GLint visualAttributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
    data.visual = glXChooseVisual(data.display, screen, visualAttributes);

    if(!data.visual)
    {
        std::println(std::cerr, "Couldn't choose a visual for x11");
        exit(-1);
    }

    data.colormap = XCreateColormap(data.display, rootWindow, data.visual->visual, AllocNone);

    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap = data.colormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    data.window = XCreateWindow(data.display, rootWindow, 0, 0, 1920, 1080, 0, data.visual->depth, InputOutput, data.visual->visual, CWColormap | CWEventMask, &windowAttributes);

    data.deleteWindowMessage = XInternAtom(data.display, "WM_DELETE_WINDOW", False);
    if(!XSetWMProtocols(data.display, data.window, &data.deleteWindowMessage, 1))
        std::println("Warning couldn't register WM_DELETE_WINDOW");

    XMapWindow(data.display, data.window);
    XStoreName(data.display, data.window, "My Game");

    data.context = glXCreateContext(data.display, data.visual, NULL, GL_TRUE);
    glXMakeCurrent(data.display, data.window, data.context);

    return data;
}

#define LOAD_OPENGL_FUNCTION(functionName, functionType) functionName = (functionType)glXGetProcAddress((const GLubyte*)#functionName)

PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;

void loadOpenGLFunctions()
{
    LOAD_OPENGL_FUNCTION(glGenBuffers, PFNGLGENBUFFERSPROC);
    LOAD_OPENGL_FUNCTION(glBindBuffer, PFNGLBINDBUFFERPROC);
    LOAD_OPENGL_FUNCTION(glBufferData, PFNGLBUFFERDATAPROC);
    LOAD_OPENGL_FUNCTION(glCreateShader, PFNGLCREATESHADERPROC);
    LOAD_OPENGL_FUNCTION(glShaderSource, PFNGLSHADERSOURCEPROC);
    LOAD_OPENGL_FUNCTION(glCompileShader, PFNGLCOMPILESHADERPROC);
    LOAD_OPENGL_FUNCTION(glCreateProgram, PFNGLCREATEPROGRAMPROC);
    LOAD_OPENGL_FUNCTION(glAttachShader, PFNGLATTACHSHADERPROC);
    LOAD_OPENGL_FUNCTION(glLinkProgram, PFNGLLINKPROGRAMPROC);
    LOAD_OPENGL_FUNCTION(glDeleteShader, PFNGLDELETESHADERPROC);
    LOAD_OPENGL_FUNCTION(glUseProgram, PFNGLUSEPROGRAMPROC);
    LOAD_OPENGL_FUNCTION(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC);
    LOAD_OPENGL_FUNCTION(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC);
    LOAD_OPENGL_FUNCTION(glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC);
    LOAD_OPENGL_FUNCTION(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC);
    LOAD_OPENGL_FUNCTION(glGetShaderiv, PFNGLGETSHADERIVPROC);
    LOAD_OPENGL_FUNCTION(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC);
    LOAD_OPENGL_FUNCTION(glGetProgramiv, PFNGLGETPROGRAMIVPROC);
    LOAD_OPENGL_FUNCTION(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC);
    LOAD_OPENGL_FUNCTION(glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC);
    LOAD_OPENGL_FUNCTION(glDeleteBuffers, PFNGLDELETEBUFFERSPROC);
    LOAD_OPENGL_FUNCTION(glDeleteProgram, PFNGLDELETEPROGRAMPROC);
}

int main(int, char**)
{
    XlibData xorg = initXlibGlx();

    loadOpenGLFunctions();

    std::println("OpenGL Vendor: {}", (const char*)glGetString(GL_VENDOR));
    std::println("OpenGL Renderer: {}", (const char*)glGetString(GL_RENDERER));
    std::println("OpenGL Version: {}", (const char*)glGetString(GL_VERSION));
    std::println("OpenGL Shading Language Version: {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    //TEST CODE modified from learnopengl.com:
    const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(0.356862745f, 0.807843137f, 0.980392157f, 1.0f);\n"
        "}\n\0";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::println(std::cerr, "Failed to compile vertex shader: {}", infoLog);
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::println(std::cerr, "Failed to compile fragment shader: {}", infoLog);
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::println(std::cerr, "Failed to link program: {}", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    bool shouldRun = true;
    while(shouldRun)
    {
        XEvent event;
        while(XPending(xorg.display))
        {
            XNextEvent(xorg.display, &event);
            switch(event.type)
            {
                case ClientMessage:
                    if((Atom)event.xclient.data.l[0] == xorg.deleteWindowMessage)
                        shouldRun = false;
                    break;

                case ConfigureNotify:
                    XConfigureEvent* configureNotifyEvent = (XConfigureEvent*)&event;
                    glViewport(0, 0, configureNotifyEvent->width, configureNotifyEvent->height);
                    break;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glXSwapBuffers(xorg.display, xorg.window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    return 0;
}
