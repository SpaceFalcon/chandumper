#ifndef DIALOGS_H_INCLUDED
#define DIALOGS_H_INCLUDED

extern char* OpenFileDialog(HWND parent, LPSTR filter);
extern char* FolderBrowserDialog(HWND parent);
extern list* MultipleFileSelectDialog(HWND parent, LPSTR filter);

#endif // DIALOGS_H_INCLUDED
