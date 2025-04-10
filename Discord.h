#pragma once
#include <wx/wx.h>
#include "ChatClient.h"
#include "LoginDialog.h"
#include "RoomListDialog.h"
#include "RegisterDialog.h"
#include "ServerAPI.h"
#include <windows.h>

// 메인 어플리케이션
class Discord : public wxApp {
public:
    virtual bool OnInit() override;
};

wxDECLARE_APP(Discord);