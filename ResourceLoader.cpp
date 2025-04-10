#include "ResourceLoader.h"
#include "resource.h"
#include <wx/mstream.h>
#include <windows.h>

// LoadPngFromResource �Լ��� ���ҽ����� PNG �̹����� �ε��ϴ� �Լ�
wxImage LoadPngFromResource(int resID)
{
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resID), _T("PNG"));

    std::string debugMsg = "���ҽ� ID: " + std::to_string(resID);
    OutputDebugStringA((debugMsg + "\n").c_str());

    if (!hRes)
    {
        OutputDebugStringA("���ҽ� ã�� �� ����\n");
        return wxImage();
    }

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) {
        OutputDebugStringA("���ҽ� �����͸� �ε��� �� ����\n");
        return wxImage();
    }

    void* pData = LockResource(hData);
    if (!pData) {
        OutputDebugStringA("���ҽ� �����͸� ��� �� ����\n");
        return wxImage();
    }

    DWORD size = SizeofResource(NULL, hRes);
    if (size == 0) {
        OutputDebugStringA("���ҽ� ũ�Ⱑ 0�Դϴ�\n");
        return wxImage();
    }

    wxMemoryInputStream stream(pData, size);
    wxImage img(stream, wxBITMAP_TYPE_PNG);
    if (!img.IsOk())
    {
        OutputDebugStringA("�̹��� �ε����\n");
		return wxImage();
    }
    return img;
}
