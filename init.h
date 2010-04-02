#ifndef INIT_H_INCLUDED
#define INIT_H_INCLUDED

extern HMENU sysMenu;
extern int showconsolepos;
extern HWND PostLocation; /* init.c */
extern HWND FileList;     /* init.c */
extern HWND BoardSelect;  /* init.c */
extern HWND ConsoleWindow;

extern BOOL InitFileList(HWND hwndDlg);
extern BOOL InitBoardSelect(HWND hwndDlg);
extern void InitConsole(HWND hwndDlg);

#endif // INIT_H_INCLUDED
