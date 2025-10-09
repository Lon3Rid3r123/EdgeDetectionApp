#include <jni.h>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

// Include OpenGL ES 2 headers
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// --- Global Variables for OpenGL ---
GLuint gProgram;
GLuint gTextureId;
GLuint gPositionLoc;
GLuint gTexCoordLoc;

// --- Helper function to compile a shader ---
GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            // Error handling: You can log the error to see what's wrong with the shader
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

// --- Helper function to create the shader program ---
GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) return 0;
    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) return 0;

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, pixelShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

// --- Shader Source Code ---
const char* gVertexShader =
        "attribute vec4 vPosition;\n"
        "attribute vec2 vTexCoord;\n"
        "varying vec2 texCoord;\n"
        "void main() {\n"
        "  gl_Position = vPosition;\n"
        "  texCoord = vTexCoord;\n"
        "}\n";

const char* gFragmentShader =
        "precision mediump float;\n"
        "varying vec2 texCoord;\n"
        "uniform sampler2D sTexture;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(sTexture, texCoord);\n"
        "}\n";


// --- JNI Functions ---

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_edgedetectionapp_NativeLib_processFrame(JNIEnv *env, jclass clazz,
                                                         jobject y_buffer, jobject u_buffer,
                                                         jobject v_buffer, jint width, jint height,
                                                         jint pixel_stride_y, jint row_stride_y,
                                                         jint pixel_stride_uv, jint row_stride_uv) {
    uint8_t* y_data = (uint8_t*) env->GetDirectBufferAddress(y_buffer);
    cv::Mat y_mat(height, width, CV_8UC1, y_data, row_stride_y);
    cv::Mat gray_mat = y_mat;
    cv::Mat edges_mat;

    cv::Canny(gray_mat, edges_mat, 50, 150);

    cv::Mat* rgba_mat = new cv::Mat();
    cv::cvtColor(edges_mat, *rgba_mat, cv::COLOR_GRAY2RGBA);

    return (jlong) rgba_mat;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_edgedetectionapp_NativeLib_onSurfaceCreated(JNIEnv* env, jclass clazz) {
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        return; // Handle error
    }
    gPositionLoc = glGetAttribLocation(gProgram, "vPosition");
    gTexCoordLoc = glGetAttribLocation(gProgram, "vTexCoord");

    glGenTextures(1, &gTextureId);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_edgedetectionapp_NativeLib_onSurfaceChanged(JNIEnv* env, jclass clazz, jint width, jint height) {
    glViewport(0, 0, width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_edgedetectionapp_NativeLib_drawFrame(JNIEnv* env, jclass clazz, jlong mat_pointer) {
    cv::Mat* rgbaMat = (cv::Mat*)mat_pointer;
    if (rgbaMat == nullptr || rgbaMat->empty()) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(gProgram);

    glBindTexture(GL_TEXTURE_2D, gTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgbaMat->cols, rgbaMat->rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaMat->data);

    GLfloat vertices[] = {
            -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
            1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
            1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
    };

    glVertexAttribPointer(gPositionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices);
    glVertexAttribPointer(gTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vertices[3]);

    glEnableVertexAttribArray(gPositionLoc);
    glEnableVertexAttribArray(gTexCoordLoc);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    delete rgbaMat; // Avoid memory leaks
}