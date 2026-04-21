#include <iostream>
#include <print>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

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
    windowAttributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

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

int main(int, char**)
{
    XlibData xorg = initXlibGlx();

    loadOpenGLFunctions();

    std::println("OpenGL Vendor: {}", (const char*)glGetString(GL_VENDOR));
    std::println("OpenGL Renderer: {}", (const char*)glGetString(GL_RENDERER));
    std::println("OpenGL Version: {}", (const char*)glGetString(GL_VERSION));
    std::println("OpenGL Shading Language Version: {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    unsigned int shaderProgram = createProgram({compileShader(GL_VERTEX_SHADER, "default.vert"), compileShader(GL_FRAGMENT_SHADER, "default.frag")});
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "otherTexture"), 1);

    float vertices[] = {
        // positions          // colors           // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3,
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

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
                    {
                        XConfigureEvent* configureNotifyEvent = (XConfigureEvent*)&event;
                        glViewport(0, 0, configureNotifyEvent->width, configureNotifyEvent->height);
                    }
                    break;

                case KeyPress:
                    {
                        XKeyEvent* keyPressEvent = (XKeyEvent*)&event;
                        if(keyPressEvent->keycode == XKeysymToKeycode(xorg.display, XK_Escape))
                            shouldRun = false;
                    }
                    break;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
