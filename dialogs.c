#include <windows.h>
#include <shlobj.h>

char* OpenFileDialog(HWND parent, LPSTR filter)
{
    OPENFILENAME ofn;
    char *szFileName = (char*)malloc(MAX_PATH);
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
        return szFileName;
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
