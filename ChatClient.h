#pragma once
#include <winsock2.h>
#include <thread>
#include <string>
#include <functional>

class ChatClient {
public:
    ChatClient();
    ~ChatClient();

    std::string DiscoverServerIP(); // 서버 IP 찾는 함수, 서버가 열려있는지 묻는 함수


    bool Connect(const std::string& ip, int port);
    void Disconnect();
    void Send(const std::string& message);
    void StartReceiving();
    void SetNickname(const std::string name);
    std::function<void(const std::string&)> onMessageReceived;  //GUI에서 메시지 받았을 때 해야 할 행동, 콜백함수

    std::string GetNickname() const { return nickname; }
private:
    SOCKET sock;
    bool isConnected = false;

    std::string nickname;
};
