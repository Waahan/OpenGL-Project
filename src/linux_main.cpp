#include <iostream>
#include <print>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <chrono>

#include <cstring>
#include <cstdio>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct XlibData
{
    Display* display;
    Window window;
    Atom deleteWindowMessage;
    GLXContext context;

    ~XlibData()
    {
        glXDestroyContext(display, context);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }
};

void disableCursor(XlibData& data)
{
    XGrabPointer(data.display, data.window, False, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

    Cursor invisible;
    {
        char cursorData[1] = {0};
        Pixmap mask = XCreateBitmapFromData(data.display, data.window, cursorData, 1, 1);
        Pixmap pix = XCreateBitmapFromData(data.display, data.window, cursorData, 1, 1);
        XColor dummy;
        invisible = XCreatePixmapCursor(data.display, pix, mask, &dummy, &dummy, 0, 0);
        XFreePixmap(data.display, pix);
        XFreePixmap(data.display, mask);
    }
    XDefineCursor(data.display, data.window, invisible);
}

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
    XVisualInfo* visual = glXChooseVisual(data.display, screen, visualAttributes);

    if(!visual)
    {
        std::println(std::cerr, "Couldn't choose a visual for x11");
        exit(-1);
    }

    Colormap colormap = XCreateColormap(data.display, rootWindow, visual->visual, AllocNone);

    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap = colormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | StructureNotifyMask;

    data.window = XCreateWindow(data.display, rootWindow, 0, 0, 1920, 1080, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &windowAttributes);

    XFreeColormap(data.display, colormap);

    data.deleteWindowMessage = XInternAtom(data.display, "WM_DELETE_WINDOW", False);
    if(!XSetWMProtocols(data.display, data.window, &data.deleteWindowMessage, 1))
        std::println("Warning couldn't register WM_DELETE_WINDOW");

    XMapWindow(data.display, data.window);
    XStoreName(data.display, data.window, "My Game");

    data.context = glXCreateContext(data.display, visual, NULL, GL_TRUE);
    glXMakeCurrent(data.display, data.window, data.context);

    XFree(visual);

    disableCursor(data);

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
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLUNIFORM4FPROC glUniform4f = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
PFNGLBINDTEXTURESPROC glBindTextures = NULL;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = NULL;

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
    LOAD_OPENGL_FUNCTION(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC);
    LOAD_OPENGL_FUNCTION(glUniform4f, PFNGLUNIFORM4FPROC);
    LOAD_OPENGL_FUNCTION(glUniform1i, PFNGLUNIFORM1IPROC);
    LOAD_OPENGL_FUNCTION(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC);
    LOAD_OPENGL_FUNCTION(glBindTextures, PFNGLBINDTEXTURESPROC);
    LOAD_OPENGL_FUNCTION(glGenerateMipmap, PFNGLGENERATEMIPMAPPROC);
}

GLuint compileShader(GLenum shaderType, const char* shaderFile)
{
    std::string fileContent;
    {
        std::ifstream inputFile{shaderFile};
        if(inputFile.fail())
        {
            std::println(std::cerr, "Failed to load shader file('{}'): ", shaderFile, std::strerror(errno));
            exit(-1);
        }

        std::stringstream bufferStream;
        bufferStream << inputFile.rdbuf();
        fileContent = bufferStream.str();
    }
    const char* shaderCode = fileContent.c_str();

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        int logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::unique_ptr<char[]> infoLog = std::make_unique<char[]>(logLength);
        glGetShaderInfoLog(shader, logLength, NULL, infoLog.get());
        std::println(std::cerr, "Failed to compile '{}' shader: {}", shaderFile, static_cast<const char*>(infoLog.get()));
        exit(-1);
    }

    return shader;
}

GLuint createProgram(std::initializer_list<GLuint> shaders)
{
    GLuint program = glCreateProgram();
    for(GLuint shader : shaders)
        glAttachShader(program, shader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::println(std::cerr, "Failed to link program: {}", infoLog);
        exit(-1);
    }

    for(GLuint shader : shaders)
        glDeleteShader(shader);

    return program;
}

template<typename T>
T readLittleEndian32bit(unsigned char* from)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return ((T)from[0] << 24) | ((T)from[1] << 16) | ((T)from[2] << 8) | ((T)from[3]);
#else
    return ((T)from[3] << 24) | ((T)from[2] << 16) | ((T)from[1] << 8) | ((T)from[0]);
#endif
}

std::tuple<unsigned char*, int, int> loadBMP(const char* filePath)
{
    std::FILE* file = fopen(filePath, "rb");
    if(!file)
    {
        std::println(std::cerr, "Failed to open image file('{}'): {}", filePath, std::strerror(errno));
        exit(-1);
    }

    const constexpr size_t HEADER_SIZE = 54;
    unsigned char header[HEADER_SIZE];
    if(std::fread(header, sizeof(unsigned char), HEADER_SIZE, file) != HEADER_SIZE)
    {
        std::println(std::cerr, "Bad BMP file '{}' couldn't read header", filePath);
        exit(-1);
    }

    if(header[0] != 'B' || header[1] != 'M')
    {
        std::println(std::cerr, "Missing BM header in '{}'", filePath);
        exit(-1);
    }

    unsigned int dataPosition = readLittleEndian32bit<unsigned int>(&header[10]);
    int width = readLittleEndian32bit<int>(&header[18]);
    int height = readLittleEndian32bit<int>(&header[22]);
    unsigned int imageSize = readLittleEndian32bit<unsigned int>(&header[34]);

    if(imageSize == 0)
        imageSize = width * height * 3;

    if(dataPosition == 0)
        dataPosition = HEADER_SIZE;

    //TODO fix it to work with non 24 bit bmps
    if((int)imageSize != (width * height * 3))
    {
        std::println("Warning only 24 bit BMP images supported");
        exit(-1);
    }

    fseek(file, dataPosition, SEEK_SET);

    unsigned char* data = new unsigned char [imageSize];
    if(std::fread(data, 1, imageSize, file) != imageSize)
        std::println(std::cerr, "Error reading BMP file data for '{}'", filePath);

    std::fclose(file);

    return std::make_tuple(data, width, height);
}

struct
{
    bool shouldRun = true;

    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;

    float lastX = 0.0f;
    float lastY = 0.0f;
    float windowWidth = 1920.0f;
    float windowHeight = 1080.0f;
    float pitch = 0.0f;
    float yaw = -90.0f;

    void(*mouseRelativeMotion)(float, float);
    void(*windowResize)(float, float);
} Input;

void linuxProcessInput(XlibData& xorg)
{
    XEvent event;
    while(XPending(xorg.display))
    {
        XNextEvent(xorg.display, &event);
        switch(event.type)
        {
            case ClientMessage:
                if((Atom)event.xclient.data.l[0] == xorg.deleteWindowMessage)
                    Input.shouldRun = false;
                break;

            case ConfigureNotify:
                {
                    XConfigureEvent* configureNotifyEvent = (XConfigureEvent*)&event;

                    Input.windowWidth = configureNotifyEvent->width;
                    Input.windowHeight = configureNotifyEvent->height;

                    Input.windowResize(configureNotifyEvent->width, configureNotifyEvent->height);
                }
                break;

            case KeyPress:
                {
                    XKeyEvent* keyPressEvent = (XKeyEvent*)&event;
                    if(keyPressEvent->keycode == XKeysymToKeycode(xorg.display, XK_Escape))
                        Input.shouldRun = false;
                    else if(keyPressEvent->keycode == XKeysymToKeycode(xorg.display, XK_W))
                        Input.forward = true;
                    else if(keyPressEvent->keycode == XKeysymToKeycode(xorg.display, XK_S))
                        Input.backward = true;
                    else if(keyPressEvent->keycode == XKeysymToKeycode(xorg.display, XK_A))
                        Input.left = true;
                    else if(keyPressEvent->keycode == XKeysymToKeycode(xorg.display, XK_D))
                        Input.right = true;
                }
                break;

            case KeyRelease:
                {
                    XKeyEvent* keyPressEvent = (XKeyEvent*)&event;
                    if(keyPressEvent->keycode == XKeysymToKeycode(xorg.display, XK_W))
                        Input.forward = false;
                    else if(keyPressEvent->keycode == XKeysymToKeycode(xorg.display, XK_S))
                        Input.backward = false;
                    else if(keyPressEvent->keycode == XKeysymToKeycode(xorg.display, XK_A))
                        Input.left = false;
                    else if(keyPressEvent->keycode == XKeysymToKeycode(xorg.display, XK_D))
                        Input.right = false;
                }
                break;

            case MotionNotify:
                {
                    XMotionEvent* motionEvent = (XMotionEvent*)&event;

                    float xoffset = motionEvent->x - Input.lastX;
                    float yoffset = Input.lastY - motionEvent->y;

                    Input.lastX = motionEvent->x;
                    Input.lastY = motionEvent->y;

                    Input.mouseRelativeMotion(xoffset, yoffset);
                }
                break;
        }
    }
    XWarpPointer(xorg.display, None, xorg.window, 0, 0, 0, 0, Input.windowWidth / 2, Input.windowHeight / 2);
    Input.lastX = Input.windowWidth / 2;
    Input.lastY = Input.windowHeight / 2;
}

struct
{
    std::chrono::time_point<std::chrono::steady_clock> lastTime;
    float deltaTime = 0.0f;
} GameTime;

void updateTime()
{
    std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();
    GameTime.deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - GameTime.lastTime).count() * 0.001f;
    GameTime.lastTime = currentTime;
}

struct
{
    glm::vec3 position = glm::vec3{0.0f, 0.0f, 3.0f};
    glm::vec3 front = glm::vec3{0.0f, 0.0f, -1.0f};
    glm::vec3 up = glm::vec3{0.0f, 1.0f, 0.0f};
    const float FOV = 45.0f;

    glm::mat4 view;
    glm::mat4 projection;
} Camera;

void processInput()
{
    float cameraSpeed = 5.0f * GameTime.deltaTime;

    if(Input.forward)
        Camera.position += cameraSpeed * Camera.front;

    if(Input.backward)
        Camera.position -= cameraSpeed * Camera.front;

    if(Input.left)
        Camera.position -= glm::normalize(glm::cross(Camera.front, Camera.up)) * cameraSpeed;

    if(Input.right)
        Camera.position += glm::normalize(glm::cross(Camera.front, Camera.up)) * cameraSpeed;
}

void inputMouseCamera(float xoffset, float yoffset)
{
    const constexpr float sensitivity = 0.05f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    Input.yaw += xoffset;
    Input.pitch += yoffset;

    Input.pitch = std::clamp(Input.pitch, -89.0f, 89.0f);

    glm::vec3 direction;
    direction.x = cos(glm::radians(Input.yaw)) * cos(glm::radians(Input.pitch));
    direction.y = sin(glm::radians(Input.pitch));
    direction.z = sin(glm::radians(Input.yaw)) * cos(glm::radians(Input.pitch));
    Camera.front = glm::normalize(direction);
}

void windowResizeEvent(float width, float height)
{
    glViewport(0, 0, width, height);
    Camera.projection = glm::perspective(glm::radians(Camera.FOV), width / height, 0.1f, 100.0f);
}

// For testing purposes only
void fpsCounter()
{
    static std::chrono::time_point<std::chrono::steady_clock> lastTime = std::chrono::steady_clock::now();
    static double frames = 0;

    frames++;
    if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastTime).count() >= 1.0f)
    {
        std::println("FPS: {}, {} milliseconds per frame", frames, 1000.0/frames);
        frames = 0;
        lastTime = std::chrono::steady_clock::now();
    }
}

int main(int, char**)
{
    XlibData xorg = initXlibGlx();

    loadOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);

    std::println("OpenGL Vendor: {}", (const char*)glGetString(GL_VENDOR));
    std::println("OpenGL Renderer: {}", (const char*)glGetString(GL_RENDERER));
    std::println("OpenGL Version: {}", (const char*)glGetString(GL_VERSION));
    std::println("OpenGL Shading Language Version: {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    unsigned int shaderProgram = createProgram({compileShader(GL_VERTEX_SHADER, "default.vert"), compileShader(GL_FRAGMENT_SHADER, "default.frag")});
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "otherTexture"), 1);

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // 0
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // 1
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // 2
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // 3
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // 4
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // 5
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // 6
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // 7
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // 8
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // 9
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // 10
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // 11
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // 12
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // 13
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, // 14
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // 15
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        8, 15, 9,
        9, 4, 8,
        10, 2, 11,
        11, 12, 10,
        9, 13, 5,
        5, 4, 9,
        3, 2, 10,
        10, 14, 3,
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    {
        auto [data, width, height] = loadBMP("bridget.bmp");

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        delete data;
    }

    unsigned int texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    {
        auto [data, width, height] = loadBMP("fun.bmp");

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        delete data;
    }

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    Camera.view = glm::lookAt(Camera.position, Camera.position + Camera.front, Camera.up);
    Camera.projection = glm::perspective(glm::radians(Camera.FOV), Input.windowWidth / Input.windowHeight, 0.1f, 100.0f);

    Input.mouseRelativeMotion = inputMouseCamera;
    Input.windowResize = windowResizeEvent;

    float angle = 0.0f;

    while(Input.shouldRun)
    {
        //fpsCounter();

        //std::chrono::time_point<std::chrono::high_resolution_clock> frameBeginTime = std::chrono::high_resolution_clock::now();

        updateTime();

        linuxProcessInput(xorg);
        processInput();

        Camera.view = glm::lookAt(Camera.position, Camera.position + Camera.front, Camera.up);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(Camera.view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(Camera.projection));

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        angle += 20 * GameTime.deltaTime;

        for(int i = 0; i < 10; i++)
        {
            glm::mat4 currentModel = glm::mat4(1.0f);
            currentModel = glm::translate(currentModel, cubePositions[i]);
            currentModel = glm::rotate(currentModel, glm::radians(5.0f * angle), glm::vec3(1.0f, 1.0f, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(currentModel));
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        //std::println("Frame took {} milliseconds", std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - frameBeginTime).count() * 0.000001f);

        glXSwapBuffers(xorg.display, xorg.window);
    }

    glDeleteTextures(1, &texture2);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    return 0;
}
