// pngEditor.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS D_WINSOCKAPI_ /c

#include "utf4win.h"
#ifdef WIN32
#include <tchar.h>
#else   // WIN32
//class MyFrame; //Fwd declaration
//#define HWND MyFrame * => moved these 2 lines under defs.h
#endif  // WIN32
//#include "model.h"
//#include "controller.h"
#include "defs.h"
#include "engine.h"

// object to represent running Engine
Engine *engine = nullptr;

//Common constants
#define ID_Save 1
#define ID_Load 2
#define ID_Exit 3
#define ID_Undo 4
#define ID_Redo 5
#define ID_Apro 6

#define SIZE_X 500
#define SIZE_Y 300
#define aboutStr "\tpng Editor was initially a quick home made tool to work on small \
16*16 icons defined in png files.\r\nIt growed becoming a tool to check \
eventual steganography embedded in .png files.\r\nCopyright drehuwann@gmail.com\
\r\nPublished under the terms of the General Public License.\r\n\
(See https://gnu.org/licenses/gpl.html)"

// common functions
Engine *InitEngine(HWND hWnd) {
   if (!hWnd) return nullptr;
   Model *modl = new Model();
   if (!modl) return nullptr;
   Controller *ctrl = new Controller();
   if (!ctrl) {
      if (modl) delete modl;
      return nullptr;
   }
   Engine *toRet = new Engine();
   if (!toRet) {
      if (ctrl) delete ctrl;
      if (modl) delete modl;
   } else {
      if (toRet->Init(modl, (void *)hWnd, ctrl) != Error::NONE) {
         if (ctrl) delete ctrl;
         if (modl) delete modl;
         delete toRet;
         toRet = nullptr;         
      }
   }
   return toRet;
}


#ifdef WIN32
// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("png Editor");

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void About(HWND hWnd) {
   MessageBox(hWnd,
         _T(aboutStr),
         _T("About"),
         MB_OK); 
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
   WNDCLASSEX wcex;
   
   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style          = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc    = WndProc;
   wcex.cbClsExtra     = 0;
   wcex.cbWndExtra     = 0;
   wcex.hInstance      = hInstance;
   wcex.hIcon          = LoadIcon(wcex.hInstance, IDI_APPLICATION);
   wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
   wcex.lpszMenuName   = NULL;
   wcex.lpszClassName  = szWindowClass;
   wcex.hIconSm        = LoadIcon(wcex.hInstance, IDI_APPLICATION);

   if (!RegisterClassEx(&wcex)) {
      MessageBox(NULL, _T("Call to RegisterClassEx failed!"), szTitle, 0);
      return 1;
   }

   // Store instance handle in our global variable
   hInst = hInstance;

   // The parameters to CreateWindowEx explained:
   // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
   // szWindowClass: the name of the application
   // szTitle: the text that appears in the title bar
   // WS_OVERLAPPEDWINDOW: the type of window to create
   // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
   // 500, 100: initial size (width, length)
   // NULL: the parent of this window
   // NULL: this application does not have a menu bar
   // hInstance: the first parameter from WinMain
   // NULL: not used in this application
   HWND hWnd = CreateWindowEx(
      WS_EX_OVERLAPPEDWINDOW,
      szWindowClass,
      szTitle,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT,
      SIZE_X, SIZE_Y,
      NULL,
      NULL,
      hInstance,
      NULL
   );

   if (!hWnd) {
      MessageBox(NULL, _T("Call to CreateWindow failed!"), szTitle, 0);
      return 1;
   }

   // The parameters to ShowWindow explained:
   // hWnd: the value returned from CreateWindow
   // nCmdShow: the fourth parameter from WinMain
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // Main message loop:
   MSG msg;
   while (GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
   PAINTSTRUCT ps;
   HDC hdc;
   TCHAR greeting[] = _T("Hello, png Editor !");
   switch (message) {
      case WM_CREATE: {
         HMENU hMenubar = CreateMenu();
         HMENU hFile = CreateMenu();
         HMENU hEdit = CreateMenu();
         HMENU hHelp = CreateMenu();
         AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hFile, _T("File"));
         AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hEdit, _T("Edit"));
         AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hHelp, _T("Help"));
         AppendMenu(hFile, MF_STRING, ID_Save, _T("Save"));
         AppendMenu(hFile, MF_STRING, ID_Load, _T("Open"));
         AppendMenu(hFile, MF_SEPARATOR, 0, 0);
         AppendMenu(hFile, MF_STRING, ID_Exit, _T("Exit"));
         AppendMenu(hEdit, MF_STRING, ID_Undo, _T("Undo"));
         AppendMenu(hEdit, MF_STRING, ID_Redo, _T("Redo"));
         AppendMenu(hHelp, MF_STRING, ID_Apro, _T("A propos"));
         SetMenu(hWnd, hMenubar);
         engine = InitEngine(hWnd);
         if (!engine) return 1;
         break;
      }
      case WM_COMMAND: {
         if (LOWORD(wParam) == ID_Save) {}
         if (LOWORD(wParam) == ID_Load) {
            OPENFILENAME ofn;       // common dialog box structure
            char szFile[280];       // buffer for file name
            // Initialize OPENFILENAME
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = (LPTSTR)szFile;
            // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
            // use the contents of szFile to initialize itself.
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = (LPCTSTR)(_T("png\0*.PNG;*.png\0"));
            ofn.nFilterIndex = 0;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            
            // Display the Open dialog box.
            if (GetOpenFileName(&ofn)==TRUE) {
               const char *utfPath = ToCstr(ofn.lpstrFile);
               Model *mod = engine->GetModel();
               mod->PickFile(utfPath);
               if (utfPath && utfPath != (const char *)ofn.lpstrFile) {
                  delete utfPath;
               }
            }
            break;
         }
         if (LOWORD(wParam) == ID_Exit) {
            PostQuitMessage(0);
            exit(0);
         }
         if (LOWORD(wParam) == ID_Undo) {}
         if (LOWORD(wParam) == ID_Redo) {}
         if (LOWORD(wParam) == ID_Apro) {
            About(hWnd);
            break;
         }
      }
      case WM_PAINT:
         hdc = BeginPaint(hWnd, &ps);
         TextOut(hdc, 5, 5, greeting, _tcslen(greeting));
         EndPaint(hWnd, &ps);
         break;
      case WM_DESTROY:
         PostQuitMessage(0);
         break;
      default:
         return DefWindowProc(hWnd, message, wParam, lParam);
         break;
   }
   return 0;
}
#else  // WIN32
#ifdef POSIX
#include <wx/wx.h>
#include <wx/filedlg.h>

class MyApp : public wxApp {
public:
   virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
   MyFrame(); 
private:
   void OnSave(wxCommandEvent& event);
   void OnLoad(wxCommandEvent& event);
   void OnExit(wxCommandEvent& event);
   void OnUndo(wxCommandEvent& event);
   void OnRedo(wxCommandEvent& event);
   void OnAbout(wxCommandEvent& event);
};

wxIMPLEMENT_APP(MyApp);
 
bool MyApp::OnInit() {
   MyFrame *frame = new MyFrame();
   if (!frame) return false;
   frame->Show(true);
   return true;
}
 
MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, "png Editor", wxDefaultPosition,
wxSize(SIZE_X, SIZE_Y)) {
   wxMenu *menuFile = new wxMenu;
   menuFile->Append(ID_Save, "&Save\tCtrl-S",
            "Save current work as <fileneme>.sav");
   menuFile->Append(ID_Load, "&Open\tCtrl-O", "Loads a png file to work on");
   menuFile->AppendSeparator();
   menuFile->Append(wxID_EXIT);
   wxMenu *menuEdit = new wxMenu;
   menuEdit->Append(ID_Undo, "&Undo\tCtrl-Z", "Undo last operation on image");
   menuEdit->Append(ID_Redo, "&Redo\tCtrl-Y",
            "Redo last undone operation on image");
   wxMenu *menuHelp = new wxMenu;
   menuHelp->Append(wxID_ABOUT);
   wxMenuBar *menuBar = new wxMenuBar;
   menuBar->Append(menuFile, "&File");
   menuBar->Append(menuEdit, "&Edit");
   menuBar->Append(menuHelp, "&Help");
   SetMenuBar( menuBar );
   CreateStatusBar();
   SetStatusText("Welcome to pngEditor!");
   Bind(wxEVT_MENU, &MyFrame::OnSave, this, ID_Save);
   Bind(wxEVT_MENU, &MyFrame::OnLoad, this, ID_Load);
   Bind(wxEVT_MENU, &MyFrame::OnUndo, this, ID_Undo);
   Bind(wxEVT_MENU, &MyFrame::OnRedo, this, ID_Redo);
   Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
   Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
   InitEngine(this);
}
 
void MyFrame::OnExit(wxCommandEvent& event) {
   Close(true);
}
 
void MyFrame::OnAbout(wxCommandEvent& event) {
   wxMessageBox(aboutStr, "About", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnSave(wxCommandEvent& event) {
   wxLogMessage("TODO");
}

void MyFrame::OnUndo(wxCommandEvent& event) {
   wxLogMessage("TODO");
}

void MyFrame::OnRedo(wxCommandEvent& event) {
   wxLogMessage("TODO");
}

void MyFrame::OnLoad(wxCommandEvent& event) {
   wxFileDialog ofd(this, _("Open PNG file"), "", "",
         "PNG files (*.png)|*.png", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
   if (ofd.ShowModal() == wxID_CANCEL) return;
   wxString wxstr = ofd.GetPath();
   const char *utfPath = (const char *)(wxstr.fn_str());
   engine.m_model->PickFile(utfPath);
}

#else  // POSIX
#error nonWin or nonPosix not implemented yet.
#endif  // POSIX
#endif  // WIN32
