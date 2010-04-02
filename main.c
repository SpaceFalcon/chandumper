/* Standard includes */
#define _WIN32_WINNT 0x500
#define _WIN32_IE 0x300
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

/* liblist : http://github.com/adamlamers/liblist */
#include <liblist/list.h>

/* project includes */
#include "version.h"
#include "resource.h"
#include "dialogs.h"
#include "fileutil.h"
#include "thread.h"
#include "randstring.h"
#include "init.h"
#include "boards.h"

HINSTANCE hInst;
HWND ThreadNo;
HWND DumpDirectory;
HWND ImageCountLabel;
HWND UploadCountLabel;
HWND CounterLabel;

HWND NameText;
HWND EmailText;
HWND SubjectText;
HWND CommentText;

HWND ShowConsoleButton;

char *RandomPassword = NULL;

static int FileCount = 0;
static int UploadedCount = 0;
static int consoleShown = 0;

int dumpActive = FALSE;

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

/* This thread handles collecting all the information, posting and printing all info to stdout */
DWORD WINAPI DumpThread(void *param)
{
    char itemText[255];
    char fileLocation[MAX_PATH];
    char postURL[255];
    int counter = 0;
    char buf[125];
    float timeleft = 0;
    
    char threadno[128];
    char name[255];
    char email[255];
    char subject[255];
    char message[1024];
    
    
    const float TIMEBETWEEN = 60.0f;
    int i = 0;
    
    int numItems = ListView_GetItemCount(FileList);
    while(1 && dumpActive)
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
            
            //Set status display to uploading
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
            printf("%s\n", fileLocation);
            printf("Posting data to: %s thread number %s\n", postURL, threadno);
            if(chan_threadreply(postURL , threadno, item.pszText, name, email, subject, message, RandomPassword) == 0)
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

void ToggleConsole()
{
    if(ConsoleWindow)
    {
        if(consoleShown == 0)
        {
            ModifyMenu(sysMenu, showconsolepos, MF_BYPOSITION | MF_STRING, IDM_SHOW_CONSOLE, "Hide Console");
            ShowWindow(ConsoleWindow, SW_SHOW);
            consoleShown = 1;
        }
        else
        {
            ModifyMenu(sysMenu, showconsolepos, MF_BYPOSITION | MF_STRING, IDM_SHOW_CONSOLE, "Show Console");
            ShowWindow(ConsoleWindow, SW_HIDE);
            consoleShown = 0;
        }
    }
}

void DumpDir_OnChange(HWND hwndDlg)
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
                    char fullfilepath[1024];
                    snprintf(fullfilepath, 1024, "%s\\%s", dumpdirPath, (char*)temp->data);
                    FileListAddRow(FileList, "", fullfilepath);
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

void AddFile_OnClick(HWND hwndDlg)
{
    list *files = MultipleFileSelectDialog(hwndDlg, "Image Files (*.jpg, *.png, *.gif)\0*.jpg;*.png;*.gif\0\0");
    char buffer[MAX_PATH];
    list *temp = files;
    while(temp)
    {
        if(get_file_extension(temp->data, buffer) == 0)
            {
                if(strncasecmp(buffer, "jpg", 3) == 0 || strncasecmp(buffer, "png", 3) == 0 || strncasecmp(buffer, "gif", 3) == 0)
                {
                    FileListAddRow(FileList, "", temp->data);
                }
            }
        free(temp->data);
        temp = temp->next;
    }
    delete_list(files);
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
            InitConsole(hwndDlg);
            DumpDirectory = GetDlgItem(hwndDlg, IDC_DUMPDIR);
            ImageCountLabel = GetDlgItem(hwndDlg, IDC_IMGCOUNT);
            UploadCountLabel = GetDlgItem(hwndDlg, IDC_UPLOADCOUNT);
            CounterLabel = GetDlgItem(hwndDlg, IDC_COUNTER);
            
            NameText = GetDlgItem(hwndDlg, IDC_NAMETEXT);
            EmailText = GetDlgItem(hwndDlg, IDC_EMAILTEXT);
            SubjectText = GetDlgItem(hwndDlg, IDC_SUBJECTTEXT);
            CommentText = GetDlgItem(hwndDlg, IDC_COMMENTTEXT);
            
            SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)VERSION_FULLSTRING);
            RandomPassword = getRandomString(12);
            return TRUE;
        case WM_CLOSE:
            FreeConsole();
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
                        char *selectedfolder = FolderBrowserDialog(hwndDlg);
                        SendMessage(DumpDirectory, WM_SETTEXT, (WPARAM)0, (LPARAM)selectedfolder);
                        free(selectedfolder);
                    }
                return TRUE;
                
                case IDC_ADDFILE:
                    if(HIWORD(wParam) == BN_CLICKED)
                    {
                        AddFile_OnClick(hwndDlg);
                    }
                return TRUE;
                
                case IDC_BTNDUMP:
                    if(HIWORD(wParam) == BN_CLICKED)
                    {
                        if(!dumpActive)
                        {
                            dumpActive = TRUE;
                            CreateThread(NULL, 0, &DumpThread, NULL, 0, NULL);
                            SendMessage(CounterLabel, WM_SETTEXT, 0, (LPARAM)"Starting...");
                            SendMessage(GetDlgItem(hwndDlg, IDC_BTNDUMP), WM_SETTEXT, 0, "Cancel");
                        }
                        else
                        {
                            dumpActive = FALSE;
                            SendMessage(GetDlgItem(hwndDlg, IDC_BTNDUMP), WM_SETTEXT, 0, "Dump");
                            SendMessage(CounterLabel, WM_SETTEXT, 0, (LPARAM)"Cancelled");
                        }
                    }
                return TRUE;
                
                case IDC_DUMPDIR:
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        DumpDir_OnChange(hwndDlg);
                    }
                return TRUE;
            }
        break;
        
        case WM_SYSCOMMAND:
            switch(LOWORD(wParam))
            {
                case IDM_SHOW_CONSOLE:
                    ToggleConsole();
                return TRUE;
                case IDM_GO_TO_WEBSITE:
                    ShellExecute(NULL, "open", "http://github.com/spacefalcon/chandumper", NULL, NULL, SW_SHOWNORMAL);
                return TRUE;
            }
        break;
        
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
