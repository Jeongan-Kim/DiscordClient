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
    long pos; //이모지 삽입 위치
    long length; // placeholder 로 쓴 문자 수(" ")
    std::string name; // 이모지 이름
};

class ChatRoomManager; // 전방 선언
// ChatRoomManager에서 받은 메시지를 단순히UI에 출력하는 역할만 담당
class ChatFrame : public wxFrame
{
public:
    ChatFrame(ChatClient& clientInst, const std::string& roomId, wxWindow* parent, ChatRoomManager* manager);

    void MarkInitialized(); // 레이아웃 초기화 완료
	bool IsReady() const { return isInitialized; } // 레이아웃 초기화 여부 확인

    void AppendMessage(const std::string& hour, const std::string& minute, const std::string& sender, const wxString& text);
	void UpdateUserList(const std::vector<std::string>& users); // 전체 채팅 참여자 목록을 업데이트하는 함수

    void OnVoiceChannelJoinedByManager();
    void OnVoiceChannelLeftByManager();

    void UpdateVoiceParticipantList(); // 채팅 + 음성 채널 참여자 목록 갱신용

    bool GetMicStatus() { return micStatus; }
    bool GetHeadsetStatus() { return headsetStatus; }

    void HighlightVoiceUser(std::string talker);
    void OnExpireTimer(wxTimerEvent& evt);

    int FindItemIndex(const std::string& clientId) const;

private:
    ChatClient& client;
	ChatRoomManager* roomManager; // 방 관리 객체
    std::string roomId; // 방 ID
	std::vector<std::string> currentUsers; // 현재 참여자 목록

	bool isInitialized = false; // 채팅창 레이아웃 초기화 여부(제대로 채팅창 생성이 완료되었는지)

    wxRichTextCtrl* chatDisplay; // 채팅창
    //wxTextCtrl* inputBox;  // 메시지 박스
    wxTextCtrl* inputBox;
    wxButton* emoticonButton; // 이모티콘 버튼

    wxStaticText* chatChannelParticipantNum; // 참여자 인원 수

    wxImageList* participantIcons; // 아이콘 이미지 리스트(헤드셋, 마이크)

	wxButton* chatJoinButton; // 채팅방 참가 버튼
	wxButton* voiceJoinButton;   // 음성 채널 참가 버튼

	wxStaticText* chatChannelLabel; // 채팅 채널 라벨
    wxStaticText* voiceChannelLabel; // 음성 채널 라벨


	wxListCtrl* chatChannelList; // 채팅창 참여자 목록
	wxListCtrl* voiceChannelList; // 음성 채널 참여자 목록

    std::unordered_map<std::string, wxTimer*> _clientTimers; // talker 회색 됐다가 다시 검정으로 돌아오는 시간    clientId->wxTimer*
    std::unordered_map<int, std::string> _timerToClient;     // timerID->clientID
    int _nextTimerId = 1;                                    // 타이머에 아이디 부여용 카운터

	wxBitmapToggleButton* micToggle; // 마이크 토글 버튼
	wxBitmapToggleButton* headsetToggle; // 헤드셋 토글 버튼

    wxStaticBitmap* profilePic;  // 프로필 이미지

    std::vector<EmojiInfo> insertEmojis; // inputBox에 삽입한 이모지 정보
    std::unordered_map<std::string, int> emojiRes = { { "angry", 114 }, { "confused", 115 }, { "poop", 116 }, { "kiss", 117 }, { "frowning", 118 },
                                                        { "grinning", 119 }, { "grinningsweat", 120 }, { "grinningwink", 121 }, { "rollsmile", 122 },
                                                        { "smile", 123 }, { "smileheart", 124 }, { "tired", 125 } };


	bool micStatus = true; // 마이크 상태
	bool headsetStatus = true; // 헤드셋 상태

	bool isInVoiceChannel = false; // 음성 채널 참가 여부

    bool isActive = true; // 채팅창이 활성화 상태인지 

	void OnInitLayout(); // 레이아웃 초기화 완료되었을 때
    void OnSend(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    wxTextAttr GetStyleForMessage(const std::string& sender, const std::string& myNickname, const std::string& part); // 글씨 색 설정 함수

    void UpdateVoiceControlsState(bool isActive); // 현재 프레임이 음성 채널에 참가한 프레임인지에 따른 버튼 상태 업데이트


    void UpdateJoinButtons();

    void OnURLClick(wxTextUrlEvent& event);
    void OnEmoticonButtonClick(wxCommandEvent& event);
    void OnVoiceJoinButtonClicked(wxCommandEvent& event);
    void OnVoiceLeaveButtonClicked(wxCommandEvent& event);
    void OnMicToggle(wxCommandEvent& event);
	void OnHeadsetToggle(wxCommandEvent& event);

    void OnParticipantRightClick(wxListEvent& event);

    void OnSetProfilePic(wxCommandEvent& event);	

    void OnActivate(wxActivateEvent& evt); //채팅방 활성화
    void ShowNotification(const wxString& roomId, const wxString& sender, const wxString& msg); // 알림 띄우기


    wxDECLARE_EVENT_TABLE();
};
