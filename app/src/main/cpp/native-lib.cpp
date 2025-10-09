#include <jni.h>
#include <string>
#include <jni.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_edgedetectionapp_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_edgedetectionapp_NativeLib_processFrame(JNIEnv *env, jobject thiz,
                                                         jobject y_buffer, jobject u_buffer,
                                                         jobject v_buffer, jint width, jint height,
                                                         jint pixel_stride_y, jint row_stride_y,
                                                         jint pixel_stride_uv, jint row_stride_uv) {
    uint8_t* y_data = (uint8_t*) env->GetDirectBufferAddress(y_buffer);
    uint8_t* u_data = (uint8_t*) env->GetDirectBufferAddress(u_buffer);
    uint8_t* v_data = (uint8_t*) env->GetDirectBufferAddress(v_buffer);
    cv::Mat y_mat(height, width, CV_8UC1, y_data, row_stride_y);
    // U and V planes need careful handling based on their strides.
    // For simplicity, we'll focus on the grayscale (Y) plane for now.

    cv::Mat gray_mat = y_mat; // The Y plane is the grayscale image
    cv::Mat edges_mat;

    // Apply Canny Edge Detection
    cv::Canny(gray_mat, edges_mat, 50, 150);

    // We need to return an RGBA Mat for OpenGL
    cv::Mat* rgba_mat = new cv::Mat();
    cv::cvtColor(edges_mat, *rgba_mat, cv::COLOR_GRAY2RGBA);

    return (jlong) rgba_mat;
}