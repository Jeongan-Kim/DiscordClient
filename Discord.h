#pragma once
#include <wx/wx.h>
#include "ChatClient.h"
#include "LoginDialog.h"
#include "RoomListDialog.h"
#include "RegisterDialog.h"
#include "ServerAPI.h"
#include <windows.h>

// ���� ���ø����̼�
class Discord : public wxApp {
public:
    virtual bool OnInit() override;
};

wxDECLARE_APP(Discord);