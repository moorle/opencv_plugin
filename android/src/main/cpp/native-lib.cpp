#include <stdint.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <android/log.h>

#define ATTRIBUTES extern "C" __attribute__((visibility("default"))) __attribute__((used))

#define DEBUG_NATIVE true

// decode 图片
ATTRIBUTES cv::Mat *opencv_decodeImage(
        unsigned char *img,
        int32_t *imgLengthBytes) {

    cv::Mat *src = new cv::Mat();
    std::vector<unsigned char> m;

    __android_log_print(ANDROID_LOG_VERBOSE, "NATIVE",
                        "opencv_decodeImage() ---  start imgLengthBytes:%d ",
                        *imgLengthBytes);

    for (int32_t a = *imgLengthBytes; a >= 0; a--) m.push_back(*(img++));

    *src = imdecode(m, cv::IMREAD_COLOR);
    if (src->data == nullptr)
        return nullptr;

    if (DEBUG_NATIVE)
        __android_log_print(ANDROID_LOG_VERBOSE, "NATIVE",
                            "opencv_decodeImage() ---  len before:%d  len after:%d  width:%d  height:%d",
                            *imgLengthBytes, src->step[0] * src->rows,
                            src->cols, src->rows);

    *imgLengthBytes = src->step[0] * src->rows;
    return src;
}

ATTRIBUTES
unsigned char *opencv_blur(
        uint8_t *imgMat,
        int32_t *imgLengthBytes,
        int32_t kernelSize) {
    // 1. decode 图片
    cv::Mat *src = opencv_decodeImage(imgMat, imgLengthBytes);
    if (src == nullptr || src->data == nullptr)
        return nullptr;
    if (DEBUG_NATIVE) {
        __android_log_print(ANDROID_LOG_VERBOSE, "NATIVE",
                            "opencv_blur() ---  width:%d   height:%d",
                            src->cols, src->rows);

        __android_log_print(ANDROID_LOG_VERBOSE, "NATIVE",
                            "opencv_blur() ---  len:%d ",
                            src->step[0] * src->rows);
    }

    // 2. 高斯模糊
    GaussianBlur(*src, *src, cv::Size(kernelSize, kernelSize), 15, 0, 4);
    std::vector<uchar> buf(1); // imencode() will resize it
//    Encoding with b       mp : 20-40ms
//    Encoding with jpg : 50-70 ms
//    Encoding with png: 200-250ms
    // 3. encode 图片
    imencode(".png", *src, buf);

    if (DEBUG_NATIVE) {
        __android_log_print(ANDROID_LOG_VERBOSE, "NATIVE",
                            "opencv_blur()  resulting image  length:%d %d x %d", buf.size(),
                            src->cols, src->rows);
    }

    *imgLengthBytes = buf.size();

    // the return value may be freed by GC before dart receive it??
    // Sometimes in Dart, ImgProc.computeSync() receives all zeros while here buf.data() is filled correctly
    // Returning a new allocated memory.
    // Note: remember to free() the Pointer<> in Dart!

    // 3. 返回data
    return buf.data();
}
