#include "RoomListDialog.h"

wxBEGIN_EVENT_TABLE(RoomListDialog, wxFrame)
EVT_BUTTON(wxID_OK, RoomListDialog::OnJoinClicked)
EVT_CLOSE(RoomListDialog::OnClose)
wxEND_EVENT_TABLE()


RoomListDialog::RoomListDialog(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, "채팅방 선택", wxDefaultPosition, wxSize(300, 200))
{
    wxPanel* panel = new wxPanel(this);  // ✅ 패널을 만들어서 그 위에 컨트롤 배치

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    roomListBox = new wxListBox(panel, wxID_ANY);
    joinButton = new wxButton(panel, wxID_OK, "입장");

    // ✅ 방 목록 생성 및 서버에 CREATE_ROOM 전송
    wxArrayString rooms = { "방1", "방2", "방3" };
    ChatClient& client = ChatClient::GetInstance();

    for (const auto& room : rooms)
    {
        roomListBox->Append(room);

        // 서버에 방 생성 요청 전송
        std::string msg = "CREATE_ROOM:" + std::string(room.mb_str());
        client.Send(msg);
    }

    sizer->Add(roomListBox, 1, wxALL | wxEXPAND, 10);
    sizer->Add(joinButton, 0, wxALL | wxALIGN_CENTER, 10);

    panel->SetSizer(sizer);

    this->SetSize(wxSize(300, 250));
    this->Centre(); // 화면 중앙 정렬
    this->Show();
}

void RoomListDialog::OnJoinClicked(wxCommandEvent& event)
{
    int sel = roomListBox->GetSelection();
    if (sel != wxNOT_FOUND) 
    {
        selectedRoom = roomListBox->GetString(sel);
        OutputDebugStringA("방 선택 완료\n");

        ChatRoomManager& roomManager = ChatRoomManager::GetInstance();

        roomManager.OpenRoom(selectedRoom.ToStdString());
        
    }
    else 
    {
        wxMessageBox("채팅방을 선택해주세요.", "알림", wxOK | wxICON_INFORMATION);
    }
}


wxString RoomListDialog::GetSelectedRoom() const
{
    return selectedRoom;
}

void RoomListDialog::OnClose(wxCloseEvent& event)
{
    ChatClient::GetInstance().Disconnect(); // 프로그램 종료 시에만

    event.Skip();
}
