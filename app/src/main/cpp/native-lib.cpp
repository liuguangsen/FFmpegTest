#include <jni.h>
#include <string>
#include <android/log.h>

#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,"liugsffmpeg",__VA_ARGS__)
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <syslog.h>
}

static double r2d(AVRational r) {
    return r.num == 0 || r.den == 0 ? 0 : (double) r.num / (double) r.den;
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
        LOGW("avformat_open_input failed!:%s", av_err2str(re));
        return env->NewStringUTF(hello.c_str());
    } else {
        LOGW("打开视频成功！");
        int64_t int64 = ic->duration;
        unsigned int i = ic->nb_streams;
        LOGW("duration = %lld nb_streams = %d", int64, i);
    }

    re = avformat_find_stream_info(ic, nullptr);
    if (re < 0) {
        LOGW("avformat_find_stream_info failed!:%s", av_err2str(re));
        return env->NewStringUTF(hello.c_str());
    } else {
        LOGW("再次探测视频成功！");
        int64_t int64 = ic->duration;
        unsigned int i = ic->nb_streams;
        LOGW("duration = %lld nb_streams = %d", int64, i);
    }

    int fps;
    int videoIndex;
    int audioIndex;
    for (int i = 0; i < ic->nb_streams; ++i) {
        AVStream *avStream = ic->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGW("找到视频流！获取视频参数信息");
            fps = (int) r2d(avStream->avg_frame_rate);
            videoIndex = i;
            LOGW("fps = %d , width = %d , height = %d , codeId = %d , pixFormat = %d",
                 fps, avStream->codecpar->width, avStream->codecpar->height,
                 avStream->codecpar->codec_id, avStream->codecpar->format
            );

        } else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGW("找到音频流！");
            audioIndex = i;
            LOGW("sample_rate = %d , channels = %d , sampleFormat = %d",
                 avStream->codecpar->sample_rate, avStream->codecpar->channels,
                 avStream->codecpar->format
            );
        }
    }

    videoIndex = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    audioIndex = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    LOGW("av_find_best_stream videoIndex = %d , audioIndex = %d", videoIndex, audioIndex);

    AVPacket *pkt = av_packet_alloc();
    for (;;) {
        re = av_read_frame(ic,pkt);
        if (re!=0){
            int pos = 3 * r2d(ic->streams[videoIndex]->time_base);
            av_seek_frame(ic,videoIndex,pos,AVSEEK_FLAG_BACKWARD|AVSEEK_FLAG_FRAME);
        }
        LOGW("stream = %d size = %d pts = %lld flag = %d",pkt->stream_index,pkt->size,pkt->pts,pkt->flags);
        //TODO 我这里测试模拟器直接卡死了，应该可以看到native内存确实有所减少
        av_packet_unref(pkt);
    }

    avformat_close_input(&ic);
    return env->NewStringUTF(hello.c_str());
}
