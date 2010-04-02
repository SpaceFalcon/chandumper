#include <windows.h>
#include <shlobj.h>
#include <liblist/list.h>

char* OpenFileDialog(HWND parent, LPSTR filter)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH];
    memset(szFileName, 0, MAX_PATH);
    memset(&ofn, 0, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = parent;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "txt";
    if(GetOpenFileName(&ofn))
        return ofn.lpstrFile;
    else
        return NULL;
}

list* MultipleFileSelectDialog(HWND parent, LPSTR filter)
{
    OPENFILENAME ofn;
    char szFileName[131072];
    memset(szFileName, 0, 131072);
    memset(&ofn, 0, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = parent;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = 131072;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;
    ofn.lpstrDefExt = "txt";
    if(GetOpenFileName(&ofn))
    {
        list *files = NULL;
        char buf[MAX_PATH + 1];
        char *p = ofn.lpstrFile + strlen(ofn.lpstrFile) + 1;
        do
        {
            strcpy(buf, ofn.lpstrFile);
            if(*p)
            {
                strcat(buf, "\\");
                strcat(buf, p);
            }
            char *temp = malloc(strlen(buf));
            strcpy(temp, buf);
            files = list_prepend(files, temp);
            
            if(*p)
                p = p + strlen(p) + 1;
        }while(*p);
        files = list_reverse(files);
        return files;
    }
    else
        return NULL;
}


char* FolderBrowserDialog(HWND parent)
{
        BROWSEINFO bi;
        char *szSelectedFolder = (char*)malloc(MAX_PATH);
        ZeroMemory(&bi, sizeof(bi));
        bi.hwndOwner = parent;
        bi.pidlRoot = NULL;
        bi.pszDisplayName = szSelectedFolder;
        bi.lpszTitle = "Select a Folder";
        LPITEMIDLIST idl = SHBrowseForFolder(&bi);
        if(idl != NULL)
        {
            if(!SHGetPathFromIDList(idl, szSelectedFolder)) return NULL;
            return szSelectedFolder;
        }
        else
            return NULL;
}
