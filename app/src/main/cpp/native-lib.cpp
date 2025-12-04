#include <jni.h>
#include <string>
#include <android/log.h>

#define LOG_TAG "Poliv_JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

 //реализация TCP клиента
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class SimpleTCPClient {
private:
    int sock = -1;

public:
    bool connect() {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return false;

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(8888);

        if (inet_pton(AF_INET, "192.168.4.1", &server.sin_addr) <= 0) {
            close(sock);
            sock = -1;
            return false;
        }

        // Таймаут для полива 3 секунды
        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        if (::connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
            close(sock);
            sock = -1;
            return false;
        }

        return true;
    }

    std::string sendCommand(const std::string& cmd) {
        if (sock < 0) {
            if (!connect()) return "CONNECT_ERROR";
        }

        std::string command = cmd + "\n";
        if (send(sock, command.c_str(), command.length(), 0) < 0) {
            close(sock);
            sock = -1;
            return "SEND_ERROR";
        }

        char buffer[256] = {0};
        int received = recv(sock, buffer, sizeof(buffer)-1, 0);
        if (received <= 0) {
            close(sock);
            sock = -1;
            return "RECV_ERROR";
        }

        buffer[received] = '\0';
        std::string response(buffer);

        // Убираем \n если есть
        if (!response.empty() && response.back() == '\n') {
            response.pop_back();
        }

        return response;
    }

    void disconnect() {
        if (sock >= 0) {
            close(sock);
            sock = -1;
        }
    }

    bool isConnected() {
        return sock >= 0;
    }
};

static SimpleTCPClient g_client;

// функции JNI для подключения к плате esp32
extern "C" {

JNIEXPORT jstring JNICALL
Java_com_example_polivsystem_MainActivity_connectESP(JNIEnv* env, jobject) {
    LOGI("connectESP");

    if (g_client.isConnected()) {
        return env->NewStringUTF("Уже подключено");
    }

    if (g_client.connect()) {
        // Тестовый запрос
        std::string ping = g_client.sendCommand("PING");
        if (ping.find("PONG") != std::string::npos) {
            return env->NewStringUTF("Подключено к ESP32!");
        }
    }

    return env->NewStringUTF("Ошибка подключения");
}

JNIEXPORT void JNICALL
Java_com_example_polivsystem_MainActivity_disconnectESP(JNIEnv*, jobject) {
    LOGI("disconnectESP");
    g_client.disconnect();
}

JNIEXPORT jstring JNICALL
Java_com_example_polivsystem_MainActivity_getStatus(JNIEnv* env, jobject) {
    LOGI("getStatus");

    if (!g_client.isConnected()) {
        return env->NewStringUTF("Не подключено");
    }

    std::string response = g_client.sendCommand("STATUS");
    return env->NewStringUTF(response.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_example_polivsystem_MainActivity_getSoil(JNIEnv* env, jobject) {
    LOGI("getSoil");

    if (!g_client.isConnected()) {
        return env->NewStringUTF("0");
    }

    std::string response = g_client.sendCommand("SOIL");
    return env->NewStringUTF(response.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_example_polivsystem_MainActivity_shortPump(JNIEnv* env, jobject) {
    LOGI("shortPump");

    if (!g_client.isConnected()) {
        return env->NewStringUTF("Не подключено");
    }

    std::string response = g_client.sendCommand("SHORT_PUMP");
    return env->NewStringUTF(response.c_str());
}

JNIEXPORT jstring JNICALL

Java_com_example_polivsystem_MainActivity_pumpOn(JNIEnv* env, jobject) {
    LOGI("pumpOn");

    if (!g_client.isConnected()) {
        return env->NewStringUTF("Не подключено");
    }

    std::string response = g_client.sendCommand("PUMP_ON");
    return env->NewStringUTF(response.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_example_polivsystem_MainActivity_pumpOff(JNIEnv* env, jobject) {
    LOGI("pumpOff");

    if (!g_client.isConnected()) {
        return env->NewStringUTF("Не подключено");
    }

    std::string response = g_client.sendCommand("PUMP_OFF");
    return env->NewStringUTF(response.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_example_polivsystem_MainActivity_autoOn(JNIEnv* env, jobject) {
    LOGI("autoOn");

    if (!g_client.isConnected()) {
        return env->NewStringUTF("Не подключено");
    }

    std::string response = g_client.sendCommand("AUTO_ON");
    return env->NewStringUTF(response.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_example_polivsystem_MainActivity_autoOff(JNIEnv* env, jobject) {
    LOGI("autoOff");

    if (!g_client.isConnected()) {
        return env->NewStringUTF("Не подключено");
    }

    std::string response = g_client.sendCommand("AUTO_OFF");
    return env->NewStringUTF(response.c_str());
}

JNIEXPORT jboolean JNICALL
Java_com_example_polivsystem_MainActivity_isConnected(JNIEnv*, jobject) {
    return g_client.isConnected() ? JNI_TRUE : JNI_FALSE;
}

}