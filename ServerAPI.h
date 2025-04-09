#pragma once

// ������ Ŭ���̾�Ʈ ���� ����� ���� API Ŭ����(���� ����� Client�� ������ �޽��� ���� ����?�ϴ� ����)
// API : Application Programming Interface � �ý���(����)�� �����ϱ� ���� �������̽�
// API�� ������ Ŭ���̾�Ʈ ���� ����� ���� ��Ģ�� ������ �����ϴ� ����

//�α��� ��û �޽��� ����
//������ �α��� ��û ����
//���� ���� �� �Ľ�
//ȸ������ ��û ó���� ���� ����
#include <memory>
#include <string>
#include <iostream>
#include "ChatClient.h"

class ServerAPI
{
public:
    static bool Init();  // ���� Ž�� �� �غ�

    // �α��� ��û - ���� ���� ��ȯ
    static bool Login(const std::string& id, const std::string& pw);

    // ȸ������ ��û - ���� ���� ��ȯ
    static bool Register(const std::string& id, const std::string& pw);

private:
    //ChatClient& client;

    static std::string BuildLoginMessage(const std::string& id, const std::string& pw);
    static std::string BuildRegisterMessage(const std::string& id, const std::string& pw);
    static bool SendAndReceive(const std::string& message, std::string& response);

    static std::string serverIP;
    static int serverPort;
};

