#define _WIN32_WINNT 0x500
#define _WIN32_IE 0x300
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"
#include "boards.h"
#include "dialogs.h"
#include "fileutil.h"
#include "thread.h"
#include "randstring.h"
#include <liblist/list.h>

#ifdef DEBUG
#define VERSION_FULLSTRING "Chandumper Version 0.0.7 - DEBUG BUILD"
#else
#define VERSION_FULLSTRING "Chandumper Version 0.0.7"
#endif
#define VERSION_STRING "0.0.7"
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_REVISION 5

HINSTANCE hInst;
HWND FileList;
HWND BoardSelect;
HWND ThreadNo;
HWND PostLocation;
HWND DumpDirectory;
HWND ImageCountLabel;
HWND UploadCountLabel;
HWND CounterLabel;

HWND ConsoleWindow = NULL;

HWND NameText;
HWND EmailText;
HWND SubjectText;
HWND CommentText;

HWND ShowConsoleButton;

char *RandomPassword = NULL;

int FileCount = 0;
int UploadedCount = 0;

BOOL ListViewAddColumn(HWND listview, int colIndex, char *headerText, int colWidth)
{
    LVCOLUMN lvc;
    memset(&lvc, 0, sizeof(lvc));
    lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.iSubItem = colIndex;
    lvc.pszText = headerText;
    lvc.cx = colWidth;
    if(ListView_InsertColumn(listview, colIndex, &lvc) == -1) return FALSE;
    return TRUE;
}

BOOL FileListAddRow(HWND listview, char *message, char *fileLocation)
{
    LVITEM item;
    memset(&item, 0, sizeof(LVITEM));
    item.mask = LVIF_TEXT;
    item.iItem = 0;
    item.iSubItem = 0;
    item.pszText = "";
    ListView_InsertItem(listview, &item);
    item.iSubItem = 1;
    item.pszText = message;
    ListView_SetItem(listview, &item);
    item.iSubItem = 2;
    item.pszText = fileLocation;
    ListView_SetItem(listview, &item);
    
    return FALSE;
}

void ClearFileList(HWND listview)
{
    SendMessage(listview, LVM_DELETEALLITEMS, 0, 0L);
}

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
    for(i = 0; i < (sizeof(boards) / sizeof(char*)); i++)
    {
        SendMessage(BoardSelect, CB_ADDSTRING, (WPARAM)0, (LPARAM)boards[i]);
    }
    SendMessage(BoardSelect, CB_SETCURSEL, (WPARAM)1, (LPARAM)0L);
    SendMessage(PostLocation, WM_SETTEXT, (WPARAM)0, (LPARAM)postlocations[SendMessage(BoardSelect, CB_GETCURSEL, 0, 0)]);
    return TRUE;
}

DWORD WINAPI DumpThread(void *param)
{
    char itemText[255];
    char pathText[MAX_PATH];
    char fileLocation[MAX_PATH];
    char postURL[255];
    int counter = 0;
    float timeleft;
    char buf[125];
    
    char threadno[128];
    //char imagePath[255];
    char name[255];
    char email[255];
    char subject[255];
    char message[1024];
    
    
    const float TIMEBETWEEN = 60.0f;
    int i = 0;
    
    int numItems = ListView_GetItemCount(FileList);
    while(1)
    {
        if((int)(timeleft * 100.0f) == 0) //if 60 seconds passed...
        {
            //Get the data from the edit controls...
            SendMessage(PostLocation, WM_GETTEXT, 255, (LPARAM)postURL);
            SendMessage(ThreadNo, WM_GETTEXT, 128, (LPARAM)threadno);
            SendMessage(NameText, WM_GETTEXT, 128, (LPARAM)name);
            SendMessage(EmailText, WM_GETTEXT, 128, (LPARAM)email);
            SendMessage(SubjectText, WM_GETTEXT, 128, (LPARAM)subject);
            SendMessage(CommentText, WM_GETTEXT, 128, (LPARAM)message);
            
            
            SendMessage(CounterLabel, WM_SETTEXT, 0, (LPARAM)"Uploading...");
            counter = 0;
            
            LVITEM item;
            memset(&item, 0, sizeof(item));
            item.mask = LVIF_TEXT; //get the text
            item.pszText = itemText;
            item.cchTextMax = 255;
            item.iSubItem = 2;     //from the "file" column
            item.iItem = i;
            ListView_GetItem(FileList, &item);
            SendMessage(DumpDirectory, WM_GETTEXT, MAX_PATH, (LPARAM)pathText);
            snprintf(fileLocation, MAX_PATH, "%s\\%s", pathText, item.pszText);
            printf("%s\n", fileLocation);
            printf("Posting data to: %s thread number %s\n", postURL, threadno);
            if(chan_threadreply(postURL, threadno, fileLocation, name, email, subject, message, RandomPassword) == 0)
            {
                UploadedCount++;
                char upcbuf[12];
                sprintf(upcbuf, "%d", UploadedCount);
                SendMessage(UploadCountLabel, WM_SETTEXT, 0, (LPARAM)upcbuf);
            }
            
            ListView_SetCheckState(FileList, i, 1);
            
            i++;
            if(i == numItems) break;
        }
        counter++;
        timeleft = TIMEBETWEEN - ((float)counter / 100.0f);
        sprintf(buf, "Next Upload In: %2.2f\n Seconds", timeleft);
        SendMessage(CounterLabel, WM_SETTEXT, 0, (LPARAM)buf);
        Sleep(10); //sleep for 1/100th of a second
    }
    SendMessage(CounterLabel, WM_SETTEXT, 0, (LPARAM)"Done");
    return TRUE;
}

int consoleShown = 0;
void ToggleConsole()
{
    if(ConsoleWindow)
    {
        if(consoleShown == 0)
        {
            SendMessage(ShowConsoleButton, WM_SETTEXT, 0, (LPARAM)"Hide Console");
            ShowWindow(ConsoleWindow, SW_SHOW);
            consoleShown = 1;
        }
        else
        {
            SendMessage(ShowConsoleButton, WM_SETTEXT, 0, (LPARAM)"Show Console");
            ShowWindow(ConsoleWindow, SW_HIDE);
            consoleShown = 0;
        }
    }
}

void InitConsole()
{
    ConsoleWindow = GetConsoleWindow();
    //Disable "x" on console
    
    HMENU winmenu = GetSystemMenu(ConsoleWindow, FALSE);
    DeleteMenu(winmenu, 6, 1024); // no more "x"
    
    ShowWindow(ConsoleWindow, SW_HIDE);
}

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            PostLocation = GetDlgItem(hwndDlg, IDC_POSTLOCATION);
            ThreadNo = GetDlgItem(hwndDlg, IDC_THREADNO);
            InitFileList(hwndDlg);
            InitBoardSelect(hwndDlg);
            DumpDirectory = GetDlgItem(hwndDlg, IDC_DUMPDIR);
            ImageCountLabel = GetDlgItem(hwndDlg, IDC_IMGCOUNT);
            UploadCountLabel = GetDlgItem(hwndDlg, IDC_UPLOADCOUNT);
            CounterLabel = GetDlgItem(hwndDlg, IDC_COUNTER);
            
            NameText = GetDlgItem(hwndDlg, IDC_NAMETEXT);
            EmailText = GetDlgItem(hwndDlg, IDC_EMAILTEXT);
            SubjectText = GetDlgItem(hwndDlg, IDC_SUBJECTTEXT);
            CommentText = GetDlgItem(hwndDlg, IDC_COMMENTTEXT);
            
            ShowConsoleButton = GetDlgItem(hwndDlg, IDC_SHOWCONSOLE);
            
            InitConsole();
            
            SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)VERSION_FULLSTRING);
            RandomPassword = getRandomString(12);
            return TRUE;
        case WM_CLOSE:
            EndDialog(hwndDlg, 0);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_BOARDSELECT:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        SendMessage(PostLocation, WM_SETTEXT, (WPARAM)0, (LPARAM)postlocations[SendMessage(BoardSelect, CB_GETCURSEL, 0, 0)]);
                    }
                return TRUE;
                
                case IDC_DUMPDIRBROWSE:
                    if(HIWORD(wParam) == BN_CLICKED)
                    {
                        SendMessage(DumpDirectory, WM_SETTEXT, (WPARAM)0, (LPARAM)FolderBrowserDialog(hwndDlg));
                    }
                return TRUE;
                
                case IDC_BTNDUMP:
                    if(HIWORD(wParam) == BN_CLICKED)
                    {
                        CreateThread(NULL, 0, &DumpThread, NULL, 0, NULL);
                    }
                return TRUE;
                
                case IDC_SHOWCONSOLE:
                    if(HIWORD(wParam) == BN_CLICKED)
                    {
                        ToggleConsole();
                    }
                return TRUE;
                
                case IDC_DUMPDIR:
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        char dumpdirPath[MAX_PATH];
                        GetWindowText(DumpDirectory, dumpdirPath, MAX_PATH);
                        if(isDirectory(dumpdirPath))
                        {
                            ListView_DeleteAllItems(FileList);
                            FileCount = 0;
                            list *filesList = GetDirectoryFiles(dumpdirPath);
                            list *temp = list_reverse(filesList);
                            char buffer[MAX_PATH];
                            while(temp)
                            {
                                if(get_file_extension(temp->data, buffer) == 0)
                                {
                                    if(strncasecmp(buffer, "jpg", 3) == 0 || strncasecmp(buffer, "png", 3) == 0 || strncasecmp(buffer, "gif", 3) == 0)
                                    {
                                        FileListAddRow(FileList, "", temp->data);
                                        FileCount++;
                                        char buf[10];
                                        sprintf(buf, "%d", FileCount);
                                        SendMessage(ImageCountLabel, WM_SETTEXT, 0, (LPARAM)buf);
                                        SendMessage(UploadCountLabel, WM_SETTEXT, 0, (LPARAM)"0");
                                    }
                                }
                                free(temp->data);
                                temp = temp->next;
                            }
                            delete_list(filesList);
                        }
                    }
                return TRUE;
            }
    }
    return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInst = hInstance;
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwICC = ICC_LISTVIEW_CLASSES;
    InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCommonControlsEx(&InitCtrls);
    return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DialogProc);
}
