#pragma once
#include <winsock2.h>
#include <thread>
#include <string>
#include <functional>

class ChatClient {
public:
    ChatClient();
    ~ChatClient();

    std::string DiscoverServerIP(); // ���� IP ã�� �Լ�, ������ �����ִ��� ���� �Լ�


    bool Connect(const std::string& ip, int port);
    void Disconnect();
    void Send(const std::string& message);
    void StartReceiving();
    void SetNickname(const std::string name);
    std::function<void(const std::string&)> onMessageReceived;  //GUI���� �޽��� �޾��� �� �ؾ� �� �ൿ, �ݹ��Լ�

    std::string GetNickname() const { return nickname; }
private:
    SOCKET sock;
    bool isConnected = false;

    std::string nickname;
};
