#ifndef FILEUTIL_H_INCLUDED
#define FILEUTIL_H_INCLUDED

#include <liblist/list.h>

extern BOOL isFile(char *path);
extern BOOL isDirectory(char *path);
extern list* GetDirectoryFiles(char *dir);
extern char* get_file_extension(char *filename, char *buffer);

#endif // FILEUTIL_H_INCLUDED
