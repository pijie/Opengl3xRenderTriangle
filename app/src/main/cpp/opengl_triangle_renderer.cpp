#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <GLES3/gl3.h>

#include <EGL/egl.h>

#define JAVA_CLASS "com/cci/opengl3nativerender/GlTriangleRenderer"

jlong drawTriangle(JNIEnv *env, jclass clazz, jobject jsurface);
jlong drawTriangle1(JNIEnv *env, jclass clazz, jobject jsurface);

const static JNINativeMethod Methods[] = {
        {"nativeDrawTriangle", "(Landroid/view/Surface;)V", reinterpret_cast<void *>(drawTriangle1)}
};

extern "C" {
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *env;
    if (jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    env->RegisterNatives(env->FindClass(JAVA_CLASS), Methods,
                         sizeof(Methods) / sizeof(Methods[0]));
    return JNI_VERSION_1_6;
}
}

namespace {
    float vertices[] = {
            0.5f, 0.5f, 0.0f,  // top right
            0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  // bottom left
            -0.5f, 0.5f, 0.0f   // top left
    };
    // 两个三角形的顶点索引
    unsigned int indices[] = {
            0, 1, 3,  // first Triangle
            1, 2, 3   //
    };

    const char *VERTEX_SHADER_SRC = R"SRC(
        #version 300 es
        layout (location = 0) in vec3 aPos;
        void main()
        {
            gl_Position = vec4(aPos.x,aPos.y,aPos.z,1.0);
        }
    )SRC";

    const char *FRAGMENT_SHADER_SRC = R"SRC(
        #version 300 es
        out vec4 FragColor;
        void main()
        {
            FragColor = vec4(1.0f,0.5f,0.2f,1.0f);
        }
    )SRC";

    const char *FRAGMENT_SHADER_SRC2 = R"SRC(
        #version 300 es
        out vec4 FragColor;
        void main(){
            FragColor = vec4(1.0f,1.0f,0.0f,1.0f);
        }
    )SRC";
}

static GLuint vbo;
static GLuint vao;
static GLuint ebo;

// 理论上此处是初始化代码只调用一次，测试demo1
jlong drawTriangle(JNIEnv *env, jclass clazz, jobject surface) {
    // 获取当前android平台的窗口
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    // 创建显示设备
    EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    // 创建eglsurface
    EGLConfig config;
    EGLint configNum;
    EGLint configAttrs[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };
    eglChooseConfig(eglDisplay, configAttrs, &config, 1, &configNum);
    assert(configNum > 0);
    // 创建surface
    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, config, nativeWindow, nullptr);
    // 创建上下文
    const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    EGLContext eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, ctxAttr);
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // 颜色缓存区，深度缓存区，模板缓存区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glViewport(0, 0, ANativeWindow_getWidth(nativeWindow) / 2,
               ANativeWindow_getHeight(nativeWindow) / 2);

    // 创建VAO对象
    glGenVertexArrays(1, &vao);
    // 创建vbo对象  // 绑定顶点数据，复用数据
    glGenBuffers(1, &vbo);
    // 创建eb0对象; ,移除重复点，的通过indices的顺序使用顶点
    glGenBuffers(1, &ebo);
    // 绑定vao对象 ，将数据批量放入显存，提高顶点数据传输效率
    glBindVertexArray(vao);

    // 绑定vbo对象 顶点数组对象
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // 批量复制顶点数据到显存中,提升顶点数据传输效率
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 绑定ebo对象
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 告诉openGl如何解析顶点数据，从vbo中读取顶点数据  size 顶点属性的大小，由于顶点属性是vec3类型，此处传递3
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    //  使能
    glEnableVertexAttribArray(0);

    // 先解绑，使用时再绑定vbo
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // 先解绑，使用时再绑定vao
    glBindVertexArray(0);

    // 编译顶点着色器
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VERTEX_SHADER_SRC, nullptr);
    glCompileShader(vertexShader);

    // 编译片元着色器
    GLuint fragShader;
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &FRAGMENT_SHADER_SRC, nullptr);
    glCompileShader(fragShader);

    // 创建着色器程序
    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 已经生成了着色器程序、现在可以删除着色器对象了
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);

    // 绑定vbo,开始使用内存数据
//    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    // 绑定vao对象
    glBindVertexArray(vao);

//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // 绘制
//    glDrawArrays(GL_TRIANGLES,0,3);
    glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, nullptr);
    eglSwapBuffers(eglDisplay, eglSurface);
    return 0;
}


// 绘制两个三角形分别使用不同的片元着色器
jlong drawTriangle1(JNIEnv *env, jclass clazz, jobject surface) {
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    EGLConfig config;
    EGLint configNum;
    EGLint configAttrs[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    eglChooseConfig(eglDisplay, configAttrs, &config, 1, &configNum);
    assert(configNum > 0);
    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, config, nativeWindow, nullptr);
    const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    EGLContext eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, ctxAttr);
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

    glViewport(0, 0, ANativeWindow_getWidth(nativeWindow) / 2,
               ANativeWindow_getHeight(nativeWindow) / 2);
//    glEnable(GL_SCISSOR_TEST);
//    glScissor(0, 0, ANativeWindow_getWidth(nativeWindow) / 4,
//              ANativeWindow_getHeight(nativeWindow) / 4);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderOrange = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint fragmentShaderYellow = glCreateShader(GL_FRAGMENT_SHADER);

    GLuint shaderProgramOrange = glCreateProgram();
    GLuint shaderProgramYellow = glCreateProgram();
    glShaderSource(vertexShader, 1, &VERTEX_SHADER_SRC, nullptr);
    glShaderSource(fragmentShaderOrange,1,&FRAGMENT_SHADER_SRC, nullptr);
    glShaderSource(fragmentShaderYellow,1,&FRAGMENT_SHADER_SRC2, nullptr);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShaderOrange);
    glCompileShader(fragmentShaderYellow);

    glAttachShader(shaderProgramOrange,vertexShader);
    glAttachShader(shaderProgramOrange,fragmentShaderOrange);
    glLinkProgram(shaderProgramOrange);

    glAttachShader(shaderProgramYellow,vertexShader);
    glAttachShader(shaderProgramYellow,fragmentShaderYellow);
    glLinkProgram(shaderProgramYellow);

    float firstTriangle[] = {
            -0.9f, -0.5f, 0.0f,  // left
            -0.0f, -0.5f, 0.0f,  // right
            -0.45f, 0.5f, 0.0f,  // top
    };
    float secondTriangle[] = {
            0.0f, -0.5f, 0.0f,  // left
            0.9f, -0.5f, 0.0f,  // right
            0.45f, 0.5f, 0.0f   // top
    };

    GLuint VBOs[2],VAOs[2];
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);

    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(firstTriangle),firstTriangle,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*4, nullptr);
    // 使能位子location =0 处的顶点属性
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(secondTriangle),secondTriangle,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*4, nullptr);
    glEnableVertexAttribArray(0);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(shaderProgramOrange);
    glBindVertexArray(VAOs[0]);
    glDrawArrays(GL_TRIANGLES,0,3);

    glUseProgram(shaderProgramYellow);
    glBindVertexArray(VAOs[1]);

    // 第二个参数顶点数组的起始索引
    glDrawArrays(GL_TRIANGLES,0,3);

    eglSwapBuffers(eglDisplay,eglSurface);
    return 0;
}