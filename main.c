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
#include <liblist/list.h>

HINSTANCE hInst;
HWND FileList;
HWND BoardSelect;
HWND ThreadNo;
HWND PostLocation;
HWND DumpDirectory;
HWND ImageCountLabel;
HWND UploadCountLabel;

int FileCount = 0;

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
    char threadno[128];
    char itemText[255];
    char pathText[MAX_PATH];
    char fileLocation[MAX_PATH];
    char postURL[255];
    
    SendMessage(PostLocation, WM_GETTEXT, 255, (LPARAM)postURL);
    SendMessage(ThreadNo, WM_GETTEXT, 128, (LPARAM)threadno);
    
    int i = 0;
    
    LVITEM item;
    memset(&item, 0, sizeof(item));
    item.mask = LVIF_TEXT; //get the text
    item.pszText = itemText;
    item.cchTextMax = 255;
    item.iSubItem = 2;     //from the "file" column
    for(i = 0; i < ListView_GetItemCount(FileList); i++)
    {
        item.iItem = i;
        ListView_GetItem(FileList, &item);
        SendMessage(DumpDirectory, WM_GETTEXT, MAX_PATH, (LPARAM)pathText);
        snprintf(fileLocation, MAX_PATH, "%s\\%s", pathText, item.pszText);
        printf("%s\n", fileLocation);
        printf("Posting data to: %s thread number: %s\n", postURL, threadno);
        printf("%d\n", chan_threadreply(postURL, threadno, fileLocation, "testdump", "", "", "", "FUCKTHEJEWS"));
        Sleep(60000);
    }
    return TRUE;
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
                            list *temp = filesList;
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
