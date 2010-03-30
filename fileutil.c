#include <windows.h>
#include <stdio.h>
#include <liblist/list.h>

BOOL isFile(char *path)
{
    FILE *file = fopen(path, "rb");
    BOOL exists = (file != NULL);
    fclose(file);
    return exists;
}

BOOL isDirectory(char *path)
{
    DWORD fileAttributes = GetFileAttributes(path);
    if(fileAttributes != INVALID_FILE_ATTRIBUTES)
        return (fileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    else
        return FALSE;
}

list* GetDirectoryFiles(const char *dir)
{
    WIN32_FIND_DATA ffd;
    HANDLE find = INVALID_HANDLE_VALUE;
    DWORD error = 0;
    list *returnList = NULL;
    
    char findDir[MAX_PATH];
    strcat(findDir, dir);   //findDir += dir
    strcat(findDir, "\\*"); //findDir += "\\*"
    
    find = FindFirstFile(findDir, &ffd);
    if(find == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }
    
    do
    {
        if((strcmp(ffd.cFileName, ".") != 0) && (strcmp(ffd.cFileName, "..") != 0)) // Exclude "." and ".."
        {
            if((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                char *string = malloc(strlen(ffd.cFileName) + 1);
                strcpy(string, ffd.cFileName);
                returnList = list_append(returnList, string);
            }
        }
    }while(FindNextFile(find, &ffd) != 0);
    
    error = GetLastError();
    FindClose(find);
    if(error != ERROR_NO_MORE_FILES)
        return NULL;
    return returnList;
}

/**
  * @param filename A string of the basename of a filename to retrieve the extension of.
  * @return -1: File has no extension, -2: Not enough memory, 0: Success
  */
int get_file_extension(char *filename, char *buffer)
{
    int i;
    int start_substr = 0;
    int filename_length = strlen(filename);
    
    for(i=0;i<filename_length;i++)
    {
        if(filename[i] == '.') start_substr = i + 1;
    }
    
    //If no file extension is found, return -1
    if(start_substr == 0) return -1;
    char *file_ext = (char*)malloc(filename_length - start_substr);
    if(file_ext == NULL) return -2; //Not enough memory.
    strcpy(buffer, filename + start_substr);
    
    return 0;
}
