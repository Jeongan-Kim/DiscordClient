#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/richtext/richtextbuffer.h>
//#include <wx/richtext/richtextobject.h>
//#include <wx/richtext/richtextimageblock.h>
//#include <wx/richtext/richtexttext.h>
#include <algorithm>
#include <wx/tglbtn.h>
#include "ChatClient.h"
#include "ResourceLoader.h"
#include "VoiceChannelManager.h"
//#include "IMERichTextCtrl.h"

struct EmojiInfo
{
    long pos; //�̸��� ���� ��ġ
    long length; // placeholder �� �� ���� ��(" ")
    std::string name; // �̸��� �̸�
};

class ChatRoomManager; // ���� ����
// ChatRoomManager���� ���� �޽����� �ܼ���UI�� ����ϴ� ���Ҹ� ���
class ChatFrame : public wxFrame
{
public:
    ChatFrame(ChatClient& clientInst, const std::string& roomId, wxWindow* parent, ChatRoomManager* manager);

    void MarkInitialized(); // ���̾ƿ� �ʱ�ȭ �Ϸ�
	bool IsReady() const { return isInitialized; } // ���̾ƿ� �ʱ�ȭ ���� Ȯ��

    void AppendMessage(const std::string& hour, const std::string& minute, const std::string& sender, const wxString& text);
	void UpdateUserList(const std::vector<std::string>& users); // ��ü ä�� ������ ����� ������Ʈ�ϴ� �Լ�

    void OnVoiceChannelJoinedByManager();
    void OnVoiceChannelLeftByManager();

    void UpdateVoiceParticipantList(); // ä�� + ���� ä�� ������ ��� ���ſ�

    bool GetMicStatus() { return micStatus; }
    bool GetHeadsetStatus() { return headsetStatus; }

    void HighlightVoiceUser(std::string talker);
    void OnExpireTimer(wxTimerEvent& evt);

    int FindItemIndex(const std::string& clientId) const;

private:
    ChatClient& client;
	ChatRoomManager* roomManager; // �� ���� ��ü
    std::string roomId; // �� ID
	std::vector<std::string> currentUsers; // ���� ������ ���

	bool isInitialized = false; // ä��â ���̾ƿ� �ʱ�ȭ ����(����� ä��â ������ �Ϸ�Ǿ�����)

    wxRichTextCtrl* chatDisplay; // ä��â
    //wxTextCtrl* inputBox;  // �޽��� �ڽ�
    wxTextCtrl* inputBox;
    wxButton* emoticonButton; // �̸�Ƽ�� ��ư

    wxStaticText* chatChannelParticipantNum; // ������ �ο� ��

    wxImageList* participantIcons; // ������ �̹��� ����Ʈ(����, ����ũ)

	wxButton* chatJoinButton; // ä�ù� ���� ��ư
	wxButton* voiceJoinButton;   // ���� ä�� ���� ��ư

	wxStaticText* chatChannelLabel; // ä�� ä�� ��
    wxStaticText* voiceChannelLabel; // ���� ä�� ��


	wxListCtrl* chatChannelList; // ä��â ������ ���
	wxListCtrl* voiceChannelList; // ���� ä�� ������ ���

    std::unordered_map<std::string, wxTimer*> _clientTimers; // talker ȸ�� �ƴٰ� �ٽ� �������� ���ƿ��� �ð�    clientId->wxTimer*
    std::unordered_map<int, std::string> _timerToClient;     // timerID->clientID
    int _nextTimerId = 1;                                    // Ÿ�̸ӿ� ���̵� �ο��� ī����

	wxBitmapToggleButton* micToggle; // ����ũ ��� ��ư
	wxBitmapToggleButton* headsetToggle; // ���� ��� ��ư

    wxStaticBitmap* profilePic;  // ������ �̹���

    std::vector<EmojiInfo> insertEmojis; // inputBox�� ������ �̸��� ����
    std::unordered_map<std::string, int> emojiRes = { { "angry", 114 }, { "confused", 115 }, { "poop", 116 }, { "kiss", 117 }, { "frowning", 118 },
                                                        { "grinning", 119 }, { "grinningsweat", 120 }, { "grinningwink", 121 }, { "rollsmile", 122 },
                                                        { "smile", 123 }, { "smileheart", 124 }, { "tired", 125 } };


	bool micStatus = true; // ����ũ ����
	bool headsetStatus = true; // ���� ����

	bool isInVoiceChannel = false; // ���� ä�� ���� ����

    bool isActive = true; // ä��â�� Ȱ��ȭ �������� 

	void OnInitLayout(); // ���̾ƿ� �ʱ�ȭ �Ϸ�Ǿ��� ��
    void OnSend(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    wxTextAttr GetStyleForMessage(const std::string& sender, const std::string& myNickname, const std::string& part); // �۾� �� ���� �Լ�

    void UpdateVoiceControlsState(bool isActive); // ���� �������� ���� ä�ο� ������ ������������ ���� ��ư ���� ������Ʈ


    void UpdateJoinButtons();

    void OnURLClick(wxTextUrlEvent& event);
    void OnEmoticonButtonClick(wxCommandEvent& event);
    void OnVoiceJoinButtonClicked(wxCommandEvent& event);
    void OnVoiceLeaveButtonClicked(wxCommandEvent& event);
    void OnMicToggle(wxCommandEvent& event);
	void OnHeadsetToggle(wxCommandEvent& event);

    void OnParticipantRightClick(wxListEvent& event);

    void OnSetProfilePic(wxCommandEvent& event);	

    void OnActivate(wxActivateEvent& evt); //ä�ù� Ȱ��ȭ
    void ShowNotification(const wxString& roomId, const wxString& sender, const wxString& msg); // �˸� ����


    wxDECLARE_EVENT_TABLE();
};
