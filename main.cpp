#include <windows.h>
#include <commdlg.h>
#include <string>

#define IDM_NEW 1001
#define IDM_OPEN 1002
#define IDM_SAVE 1003
#define IDM_EXIT 1004
#define IDM_FIND 1005
#define IDM_REPLACE 1006
#define ID_TEXTAREA 1007

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND hTextArea;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
    wc.lpszClassName = L"SimpleWindowClass";

    if (!RegisterClassW(&wc))
    {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowExW(0, L"SimpleWindowClass", L"Simple Window", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
                                NULL, NULL, hInstance, NULL);
    if (hwnd == NULL)
    {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hFileMenu), L"File");
    AppendMenuW(hFileMenu, MF_STRING, IDM_NEW, L"New File");
    AppendMenuW(hFileMenu, MF_STRING, IDM_OPEN, L"Open File...");
    AppendMenuW(hFileMenu, MF_STRING, IDM_SAVE, L"Save File");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, IDM_EXIT, L"Exit");

    HMENU hEditMenu = CreateMenu();
    AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hEditMenu), L"Edit");
    AppendMenuW(hEditMenu, MF_STRING, IDM_FIND, L"Find");
    AppendMenuW(hEditMenu, MF_STRING, IDM_REPLACE, L"Replace");

    SetMenu(hwnd, hMenu);

    hTextArea = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL,
                                0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(ID_TEXTAREA), hInstance, NULL);
    if (hTextArea == NULL)
    {
        MessageBoxW(NULL, L"Failed to create text area!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

void OpenFile(HWND hwnd)
{
    OPENFILENAMEW ofn;
    WCHAR szFileName[MAX_PATH] = L"";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.TXT\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName) / sizeof(*szFileName);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileNameW(&ofn) == TRUE)
    {
        HANDLE hFile = CreateFileW(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwFileSize = GetFileSize(hFile, NULL);
            if (dwFileSize != INVALID_FILE_SIZE)
            {
                DWORD dwRead;
                wchar_t *buffer = new wchar_t[dwFileSize / sizeof(wchar_t) + 1];
                if (ReadFile(hFile, buffer, dwFileSize, &dwRead, NULL))
                {
                    buffer[dwFileSize / sizeof(wchar_t)] = L'\0'; // Null-terminate the string
                    SetWindowTextW(hTextArea, buffer);
                }
                delete[] buffer;
            }
            CloseHandle(hFile);
        }
    }
}

void SaveFile(HWND hwnd)
{
    OPENFILENAMEW ofn;
    WCHAR szFileName[MAX_PATH] = L"";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.TXT\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName) / sizeof(*szFileName);
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn) == TRUE)
    {
        HANDLE hFile = CreateFileW(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwBytesWritten;
            int nLength = GetWindowTextLengthW(hTextArea);
            wchar_t *buffer = new wchar_t[nLength + 1];
            GetWindowTextW(hTextArea, buffer, nLength + 1);
            WriteFile(hFile, buffer, nLength * sizeof(wchar_t), &dwBytesWritten, NULL);
            delete[] buffer;
            CloseHandle(hFile);
        }
    }
}

void FindText(HWND hwnd)
{
    int nLength = GetWindowTextLengthW(hTextArea);
    if (nLength == 0)
    {
        MessageBoxW(hwnd, L"No text in the text area!", L"Find", MB_OK | MB_ICONINFORMATION);
        return;
    }

    wchar_t *buffer = new wchar_t[nLength + 1];
    GetWindowTextW(hTextArea, buffer, nLength + 1);

    std::wstring text(buffer);
    delete[] buffer;

    int start, end;
    SendMessageW(hTextArea, EM_GETSEL, reinterpret_cast<WPARAM>(&start), reinterpret_cast<LPARAM>(&end));
    int pos = text.find(L"TextToFind", end);

    if (pos != std::wstring::npos)
    {
        SendMessageW(hTextArea, EM_SETSEL, pos, pos + lstrlenW(L"TextToFind"));
        SendMessageW(hTextArea, EM_SCROLLCARET, 0, 0);
    }
    else
    {
        MessageBoxW(hwnd, L"Text not found!", L"Find", MB_OK | MB_ICONINFORMATION);
    }
}

void ReplaceText(HWND hwnd)
{
    int nLength = GetWindowTextLengthW(hTextArea);
    if (nLength == 0)
    {
        MessageBoxW(hwnd, L"No text in the text area!", L"Replace", MB_OK | MB_ICONINFORMATION);
        return;
    }

    wchar_t *buffer = new wchar_t[nLength + 1];
    GetWindowTextW(hTextArea, buffer, nLength + 1);

    std::wstring text(buffer);
    delete[] buffer;

    int start, end;
    SendMessageW(hTextArea, EM_GETSEL, reinterpret_cast<WPARAM>(&start), reinterpret_cast<LPARAM>(&end));
    int pos = text.find(L"TextToFind", end);

    if (pos != std::wstring::npos)
    {
        SendMessageW(hTextArea, EM_SETSEL, pos, pos + lstrlenW(L"TextToFind"));
        SendMessageW(hTextArea, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(L"ReplacementText"));
    }
    else
    {
        MessageBoxW(hwnd, L"Text not found!", L"Replace", MB_OK | MB_ICONINFORMATION);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_NEW:
            MessageBoxW(hwnd, L"New File Selected!", L"Info", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_OPEN:
            OpenFile(hwnd);
            break;
        case IDM_SAVE:
            SaveFile(hwnd);
            break;
        case IDM_EXIT:
            DestroyWindow(hwnd);
            break;
        case IDM_FIND:
            FindText(hwnd);
            break;
        case IDM_REPLACE:
            ReplaceText(hwnd);
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        MoveWindow(hTextArea, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}
