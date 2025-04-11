#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <algorithm>
#include <wx/tglbtn.h>
#include "ChatClient.h"
#include "ResourceLoader.h"
#include "VoiceChannelManager.h"

class ChatRoomManager; // ���� ����
// ChatRoomManager���� ���� �޽����� �ܼ���UI�� ����ϴ� ���Ҹ� ���
class ChatFrame : public wxFrame
{
public:
    ChatFrame(ChatClient& clientInst, const std::string& roomId, wxWindow* parent, ChatRoomManager* manager);

    void MarkInitialized(); // ���̾ƿ� �ʱ�ȭ �Ϸ�
	bool IsReady() const { return isInitialized; } // ���̾ƿ� �ʱ�ȭ ���� Ȯ��

    void AppendMessage(const std::string& sender, const std::string& text);
	void UpdateUserList(const std::vector<std::string>& users); // ��ü ä�� ������ ����� ������Ʈ�ϴ� �Լ�

    void OnVoiceChannelJoinedByManager();
    void OnVoiceChannelLeftByManager();

    void UpdateVoiceParticipantList(); // ä�� + ���� ä�� ������ ��� ���ſ�

private:
    ChatClient& client;
	ChatRoomManager* roomManager; // �� ���� ��ü
    std::string roomId; // �� ID
	std::vector<std::string> currentUsers; // ���� ������ ���

	bool isInitialized = false; // ä��â ���̾ƿ� �ʱ�ȭ ����(����� ä��â ������ �Ϸ�Ǿ�����)

    wxTextCtrl* chatDisplay; // ä��â
    wxTextCtrl* inputBox;  // �޽��� �ڽ�
    //wxListCtrl* allParticipantList; // ������ ��� â
    //wxStaticText* participantCountText; // ������ �ο� ��
    wxStaticText* chatChannelParticipantNum; // ������ �ο� ��

    wxImageList* participantIcons; // ������ �̹��� ����Ʈ(����, ����ũ)

	wxButton* chatJoinButton; // ä�ù� ���� ��ư
	wxButton* voiceJoinButton;   // ���� ä�� ���� ��ư

	wxStaticText* chatChannelLabel; // ä�� ä�� ��
    wxStaticText* voiceChannelLabel; // ���� ä�� ��


	wxListCtrl* chatChannelList; // ä��â ������ ���
	wxListCtrl* voiceChannelList; // ���� ä�� ������ ���

	wxBitmapToggleButton* micToggle; // ����ũ ��� ��ư
	wxBitmapToggleButton* headsetToggle; // ���� ��� ��ư

    wxStaticBitmap* profilePic;  // ������ �̹���

	bool micStatus = true; // ����ũ ����
	bool headsetStatus = true; // ���� ����

	bool isInVoiceChannel = false; // ���� ä�� ���� ����

	void OnInitLayout(); // ���̾ƿ� �ʱ�ȭ �Ϸ�Ǿ��� ��
    void OnSend(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    wxTextAttr GetStyleForMessage(const std::string& sender, const std::string& myNickname, const std::string& part); // �۾� �� ���� �Լ�
	//wxBitmap CreateCombinedIcon(const wxBitmap& micIcon, const wxBitmap& headsetIcon); // ������ ���� �Լ�

    void UpdateVoiceControlsState(bool isActive); // ���� �������� ���� ä�ο� ������ ������������ ���� ��ư ���� ������Ʈ


    void UpdateJoinButtons();

    void OnVoiceJoinButtonClicked(wxCommandEvent& event);
    void OnVoiceLeaveButtonClicked(wxCommandEvent& event);
    void OnMicToggle(wxCommandEvent& event);
	void OnHeadsetToggle(wxCommandEvent& event);

    void OnParticipantRightClick(wxListEvent& event);

    void OnSetProfilePic(wxCommandEvent& event);
};
