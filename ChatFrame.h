#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <algorithm>
#include <wx/tglbtn.h>
#include "ChatClient.h"
#include "ResourceLoader.h"
#include "VoiceChannelManager.h"

class ChatRoomManager; // 전방 선언
// ChatRoomManager에서 받은 메시지를 단순히UI에 출력하는 역할만 담당
class ChatFrame : public wxFrame
{
public:
    ChatFrame(ChatClient& clientInst, const std::string& roomId, wxWindow* parent, ChatRoomManager* manager);

    void MarkInitialized(); // 레이아웃 초기화 완료
	bool IsReady() const { return isInitialized; } // 레이아웃 초기화 여부 확인

    void AppendMessage(const std::string& sender, const std::string& text);
	void UpdateUserList(const std::vector<std::string>& users); // 전체 채팅 참여자 목록을 업데이트하는 함수

    void OnVoiceChannelJoinedByManager();
    void OnVoiceChannelLeftByManager();

    void UpdateVoiceParticipantList(); // 채팅 + 음성 채널 참여자 목록 갱신용

private:
    ChatClient& client;
	ChatRoomManager* roomManager; // 방 관리 객체
    std::string roomId; // 방 ID
	std::vector<std::string> currentUsers; // 현재 참여자 목록

	bool isInitialized = false; // 채팅창 레이아웃 초기화 여부(제대로 채팅창 생성이 완료되었는지)

    wxTextCtrl* chatDisplay; // 채팅창
    wxTextCtrl* inputBox;  // 메시지 박스
    //wxListCtrl* allParticipantList; // 참여자 목록 창
    //wxStaticText* participantCountText; // 참여자 인원 수
    wxStaticText* chatChannelParticipantNum; // 참여자 인원 수

    wxImageList* participantIcons; // 아이콘 이미지 리스트(헤드셋, 마이크)

	wxButton* chatJoinButton; // 채팅방 참가 버튼
	wxButton* voiceJoinButton;   // 음성 채널 참가 버튼

	wxStaticText* chatChannelLabel; // 채팅 채널 라벨
    wxStaticText* voiceChannelLabel; // 음성 채널 라벨


	wxListCtrl* chatChannelList; // 채팅창 참여자 목록
	wxListCtrl* voiceChannelList; // 음성 채널 참여자 목록

	wxBitmapToggleButton* micToggle; // 마이크 토글 버튼
	wxBitmapToggleButton* headsetToggle; // 헤드셋 토글 버튼

    wxStaticBitmap* profilePic;  // 프로필 이미지

	bool micStatus = true; // 마이크 상태
	bool headsetStatus = true; // 헤드셋 상태

	bool isInVoiceChannel = false; // 음성 채널 참가 여부

	void OnInitLayout(); // 레이아웃 초기화 완료되었을 때
    void OnSend(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    wxTextAttr GetStyleForMessage(const std::string& sender, const std::string& myNickname, const std::string& part); // 글씨 색 설정 함수
	//wxBitmap CreateCombinedIcon(const wxBitmap& micIcon, const wxBitmap& headsetIcon); // 아이콘 조합 함수

    void UpdateVoiceControlsState(bool isActive); // 현재 프레임이 음성 채널에 참가한 프레임인지에 따른 버튼 상태 업데이트


    void UpdateJoinButtons();

    void OnVoiceJoinButtonClicked(wxCommandEvent& event);
    void OnVoiceLeaveButtonClicked(wxCommandEvent& event);
    void OnMicToggle(wxCommandEvent& event);
	void OnHeadsetToggle(wxCommandEvent& event);

    void OnParticipantRightClick(wxListEvent& event);

    void OnSetProfilePic(wxCommandEvent& event);
};
