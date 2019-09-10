#include <jni.h>
#include <string>
#include <sstream>
#include <dlfcn.h>
#include <android/log.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/shm.h>

#include "dlopen.h"

#define TAG "JDWP"
#define ANDROID_N 24

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG , "%s" ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG  , "%s",__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG  , "%s",__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG , "%s" ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG , "%s" ,__VA_ARGS__) // 定义LOGF类型

bool loop = true;
char recvbuf[102400];

std::string bytestohexstring(char *bytes, int bytelength)
{
    std::string str("");
    std::string str2("0123456789abcdef");
    for (int i = 0; i < bytelength; i++)
    {
        int b;
        b = 0x0f & (bytes[i] >> 4);
        char s1 = str2.at(b);
        str.append(1, str2.at(b));
        b = 0x0f & bytes[i];
        str.append(1, str2.at(b));
        char s2 = str2.at(b);
    }
    return str;
}

enum JdwpTransportType {
    kJdwpTransportUnknown = 0,
    kJdwpTransportSocket,       // transport=dt_socket
    kJdwpTransportAndroidAdb,   // transport=dt_android_adb
};

typedef struct JdwpOptions {
    JdwpTransportType transport = kJdwpTransportUnknown;
    bool server = false;
    bool suspend = false;
    std::string host = "";
    uint16_t port = static_cast<uint16_t>(-1);
} OP;

extern "C" JNIEXPORT void JNICALL
Java_com_sahooz_jdwp_MainActivity_end(
        JNIEnv *env,
        jobject thiz) {
    loop = false;
}

extern "C" JNIEXPORT void JNICALL
Java_com_sahooz_jdwp_MainActivity_replaceDebug(
        JNIEnv *env,
        jobject thiz, jint port/*, jobject callback*/) {

    LOGD("Hello JNI!");
    ndk_init(env);
    void *handler = ndk_dlopen("/system/lib64/libart.so", RTLD_NOW);
    if(handler == NULL){
        LOGD(dlerror());
    }

    //对于debuggable false的配置，重新设置为可调试
    void (*allowJdwp)(bool);
    allowJdwp = (void (*)(bool)) ndk_dlsym(handler, "_ZN3art3Dbg14SetJdwpAllowedEb");
    allowJdwp(true);


    void (*pfun)();
    //关闭之前启动的jdwp-thread
    LOGD("关闭之前启动的jdwp-thread");
    pfun = (void (*)()) ndk_dlsym(handler, "_ZN3art3Dbg8StopJdwpEv");
    pfun();

    //重新配置gJdwpOptions
    int level = android_get_device_api_level();
    if(level < ANDROID_N) {
        bool (*parseJdwpOptions)(const std::string&);
        parseJdwpOptions = (bool (*)(const std::string&)) ndk_dlsym(handler,
                                                                    "_ZN3art3Dbg16ParseJdwpOptionsERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE");
        std::ostringstream oss;
        oss << "transport=dt_socket,address=" << port << ",server=y,suspend=n";
        std::string options = oss.str();
        parseJdwpOptions(options);
    } else {
        void (*config)(const OP&);
        config = (void (*)(const OP&))ndk_dlsym(handler, "_ZN3art3Dbg13ConfigureJdwpERKNS_4JDWP11JdwpOptionsE");
        OP op;
        op.transport = kJdwpTransportSocket;
        op.server = true;
        op.suspend = false;
        op.host = "127.0.0.1";
        op.port = static_cast<uint16_t>(port);
        config(op);
    }
    LOGD("重新配置gJdwpOptions");

    //重新startJdwp
    pfun = (void (*)()) ndk_dlsym(handler, "_ZN3art3Dbg9StartJdwpEv");
    pfun();
    LOGD("重新startJdwp");

    ndk_dlclose(handler);

    ///定义sockfd
    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);

    ///定义sockaddr_in
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);  ///服务器端口
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  ///服务器ip

    ///连接服务器，成功返回0，错误返回-1
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        LOGD("Connect failed..................................................");
        return;
    }

    send(sock_cli, "JDWP-Handshake", sizeof("JDWP-Handshake"), 0);
    long size = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); ///接收
    __android_log_print(ANDROID_LOG_DEBUG,TAG , "Handshake reply: %s" ,recvbuf);
    memset(recvbuf, 0, sizeof(recvbuf));

    char version[10] = {
        0, 0, 11,
        0, 0, 1, 1,
        0,
        1,
        1
    };

    send(sock_cli, version, 10, 0);
    LOGD("Sent version cmd");
    size = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); ///接收
    __android_log_print(ANDROID_LOG_DEBUG,TAG , "Version reply: %s", bytestohexstring(recvbuf, size).c_str());
    memset(recvbuf, 0, sizeof(recvbuf));

    close(sock_cli);
}