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
#include "wx/wx.h"

enum class LoginResult {
    LOGIN_SUCCESS,
    LOGIN_NO_ID,
    LOGIN_WRONG_PW,
    LOGIN_FORMAT_ERROR,
    LOGIN_CONNECT_ERROR,
	LOGIN_ALREADY,
	LOGIN_ERROR
};

class ServerAPI
{
public:
    static bool Init();  // ���� Ž�� �� �غ�

    // �α��� ��û - ���� ���� ��ȯ
    static LoginResult Login(const std::string& id, const std::string& pw);

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

