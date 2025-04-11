#pragma once
#include <winsock2.h>
#include <thread>
#include <string>
#include <functional>
#include <map>

class ChatFrame;

// ������ ���� ����ϴ� Ŭ����(TCP����, Send, recv ��), ������ ���� ���� ��ȭ�ϴ� Ŭ����
// Client �� �̱������� 1���� �����ϰ� ��𼭵� ������ �� �ְ�
class ChatClient {
public:
    ChatClient();
    ~ChatClient();

    std::string DiscoverServerIP(); // ���� IP ã�� �Լ�, ������ �����ִ��� ���� �Լ�


    bool Connect(const std::string& ip, int port);
    void Disconnect();
	bool IsConnected() const { return isConnected; } // ���� ���� Ȯ���ϴ� �Լ�

	void JoinRoom(const std::string& roomId); // �濡 �����ϴ� �Լ�

    void Send(const std::string& message);
    void StartReceiving();
    void SetNickname(const std::string name);
    std::function<void(const std::string&)> onMessageReceived;  //GUI���� �޽��� �޾��� �� �ؾ� �� �ൿ, �ݹ��Լ�

	static ChatClient& GetInstance(); // ��𼭵� ������ �� �ֵ��� �̱��� �������� ����
    std::string GetNickname() const { return nickname; }
	SOCKET GetSocket() const { return sock; } // ������ �ܺο��� ������ �� �ֵ��� getter ����

private:
    SOCKET sock;
    bool isConnected = false;
    //std::map<std::string, ChatFrame*> chatFrames; // �����ִ� ä�ù��
    std::string nickname;
};
