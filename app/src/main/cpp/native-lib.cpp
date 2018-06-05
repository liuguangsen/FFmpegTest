#include <jni.h>
#include <string>
#include <android/log.h>

#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,"liugsffmpeg",__VA_ARGS__)
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <syslog.h>
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_stick_gsliu_ffmpegtest_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    hello += avcodec_configuration();
    // 初始化解封装
    av_register_all();
    avformat_network_init();

    AVFormatContext *ic = NULL;
    char path[] = "/sdcard/Pictures/demo.mp4";
    int re = avformat_open_input(&ic, path, 0, 0);
    if (re != 0) {
        LOGW("avformat_open_input failed!:%s",av_err2str(re));
        return env->NewStringUTF(hello.c_str());
    } else {
        LOGW("打开视频成功！");
        LOGW("duration = %lld nb_streams = %d",ic->duration,ic->nb_streams);
    }

    avformat_close_input(&ic);
    return env->NewStringUTF(hello.c_str());
}
