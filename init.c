#define _WIN32_WINNT 0x500
#define _WIN32_IE 0x300
#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "boards.h"

/* forward declarations */
extern BOOL ListViewAddColumn(HWND listview, int colIndex, char *headerText, int colWidth);

HWND FileList;
HWND BoardSelect;
HWND PostLocation;
HMENU sysMenu;
int showconsolepos = 0;
HWND ConsoleWindow = NULL;

BOOL InitFileList(HWND hwndDlg)
{
    FileList = GetDlgItem(hwndDlg, IDC_LIST1);
    SendMessage(FileList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
    ListViewAddColumn(FileList, 0, "", 23);
    ListViewAddColumn(FileList, 1, "Message", 200);
    ListViewAddColumn(FileList, 2, "File", 274);
    return TRUE;
}

BOOL InitBoardSelect(HWND hwndDlg)
{
    BoardSelect = GetDlgItem(hwndDlg, IDC_BOARDSELECT);
    unsigned int i = 0;
    for(i = 0; i < NUM_BOARDS; i++)
    {
        SendMessage(BoardSelect, CB_ADDSTRING, (WPARAM)0, (LPARAM)boards[i]);
    }
    SendMessage(BoardSelect, CB_SETCURSEL, (WPARAM)1, (LPARAM)0L);
    SendMessage(PostLocation, WM_SETTEXT, (WPARAM)0, (LPARAM)postlocations[SendMessage(BoardSelect, CB_GETCURSEL, 0, 0)]);
    return TRUE;
}

void InitConsole(HWND hwndDlg)
{
    sysMenu = GetSystemMenu(hwndDlg, FALSE);
    int numItems = GetMenuItemCount(sysMenu);
    showconsolepos = numItems - 1;
    InsertMenu(sysMenu, showconsolepos, MF_BYPOSITION | MF_STRING, IDM_SHOW_CONSOLE, "Show Console");
    InsertMenu(sysMenu, showconsolepos + 1, MF_BYPOSITION | MF_STRING, IDM_GO_TO_WEBSITE, "Go to Website...");
    InsertMenu(sysMenu, showconsolepos + 2, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
    
    ConsoleWindow = GetConsoleWindow();
    /* Don't allow user to close the console when it's visible, as it terminates the program */
    
    HMENU winmenu = GetSystemMenu(ConsoleWindow, FALSE);
    DeleteMenu(winmenu, 6, 1024);
    
    ShowWindow(ConsoleWindow, SW_HIDE);
}
