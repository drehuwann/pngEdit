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

const char *colourStr[8] = {
   "Greyscale",
   "UNKNOWN !",
   "Truecolour",
   "Indexed-colour",
   "Greyscale with alpha",
   "UNKNOWN!",
   "Truecolour with alpha",
   "UNKNOWN!"
};

const char *interlaceStr[2] = {"No interlace", "Adam 7"};

// common functions
Engine *InitEngine(HWND hWnd) {
   if (!hWnd) return nullptr;
   Engine *toRet = new Engine();
   if (! toRet) return nullptr;
   Model *modl = new Model(toRet);
   if (!modl) {
      if (toRet) {
         delete toRet;
         toRet = nullptr;
      }
      return nullptr;
   }
   Controller *ctrl = new Controller(toRet);
   if (!ctrl) {
      if (modl) delete modl;
      if (toRet) {
         delete toRet;
         toRet = nullptr;
      }
      return nullptr;
   }
   if (toRet->Init(modl, (void *)hWnd, ctrl) != Error::NONE) {
      if (ctrl) delete ctrl;
      if (modl) delete modl;
      delete toRet;
      toRet = nullptr;         
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
         AppendMenu(hFile, MF_STRING, ID_Info, _T("Show Info"));
         AppendMenu(hFile, MF_STRING, ID_Layo, _T("Show Layout"));
         AppendMenu(hFile, MF_SEPARATOR, 0, 0);
         AppendMenu(hFile, MF_STRING, ID_Exit, _T("Exit"));
         AppendMenu(hEdit, MF_STRING, ID_Undo, _T("Undo"));
         AppendMenu(hEdit, MF_STRING, ID_Redo, _T("Redo"));
         AppendMenu(hHelp, MF_STRING, ID_Abou, _T("About"));
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
         if (LOWORD(wParam) == ID_Info) {
            s_imInfo *inf = engine->GetModel()->GetInfo();
            if (inf == nullptr) {
               MessageBox(NULL, _T("There is no image info available.\nDid you load a valid \
*.png file before querying info ?"), _T("WARNING"), 0);
            } else {
               const char *colstr = colourStr[inf->bitfield.colourType.to_ulong()];
               const char *intstr = interlaceStr[(inf->interlace ? 1 : 0)];
               unsigned long bd = inf->bitfield.bitDepth.to_ulong();
               UINT32 w = inf->width;
               UINT32 h = inf->height;
               int size = std::snprintf(nullptr, 0, infoStr, w, h, colstr, bd, intstr);
               size ++; // +1 for null termination.
               char *str = (char *)(malloc((size_t)size));
               std::sprintf(str, infoStr, w, h, colstr, bd, intstr);
               LPCTSTR tstr = FromCstr((const char *)str);
               MessageBox(NULL, tstr, _T("image Info"), 0);
               if (tstr && tstr != (LPCTSTR)str) {
                  delete tstr;
               }
               if (str) delete str;
            }
         }
         if (LOWORD(wParam) == ID_Layo) {}
         if (LOWORD(wParam) == ID_Exit) {
            PostQuitMessage(0);
            exit(0);
         }
         if (LOWORD(wParam) == ID_Undo) {}
         if (LOWORD(wParam) == ID_Redo) {}
         if (LOWORD(wParam) == ID_Abou) {
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
   void OnInfo(wxCommandEvent& event);
   void OnLayo(wxCommandEvent& event);
   void OnExit(wxCommandEvent& event);
   void OnUndo(wxCommandEvent& event);
   void OnRedo(wxCommandEvent& event);
   void OnAbout(wxCommandEvent& event);
};

wxIMPLEMENT_APP(MyApp);
 
bool MyApp::OnInit() {
   MyFrame *frame = new MyFrame();
   if (!frame) return false;
   engine = InitEngine(frame);
   if (!engine) {
      if (frame) delete frame;
      return false;
   }
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
   menuFile->Append(ID_Info, "&Info\tCtrl-I",
            "Show image Info");
   menuFile->Append(ID_Layo, "&Layout\tCtrl-L", "Show chunk layout");
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
   Bind(wxEVT_MENU, &MyFrame::OnInfo, this, ID_Info);
   Bind(wxEVT_MENU, &MyFrame::OnLayo, this, ID_Layo);
   Bind(wxEVT_MENU, &MyFrame::OnUndo, this, ID_Undo);
   Bind(wxEVT_MENU, &MyFrame::OnRedo, this, ID_Redo);
   Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
   Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}
 
void MyFrame::OnSave(wxCommandEvent &/*event*/) {
   wxLogMessage("TODO");
}

void MyFrame::OnLoad(wxCommandEvent &/*event*/) {
   wxFileDialog ofd(this, _("Open PNG file"), "", "",
         "PNG files (*.png)|*.png", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
   if (ofd.ShowModal() == wxID_CANCEL) return;
   wxString wxstr = ofd.GetPath();
   const char *utfPath = (const char *)(wxstr.fn_str());
   Model *mod = engine->GetModel();
   mod->PickFile(utfPath);
}

void MyFrame::OnInfo(wxCommandEvent &/*event*/) {
   s_imInfo *inf = engine->GetModel()->GetInfo();
   if (inf == nullptr) {
      wxMessageBox("There is no image info available.\nDid you load a valid \
*.png file before querying info ?", "WARNING", wxOK | wxICON_WARNING, this);
   } else {
      const char *colstr = colourStr[inf->bitfield.colourType.to_ulong()];
      const char *intstr = interlaceStr[(inf->interlace ? 1 : 0)];
      ulong bd = inf->bitfield.bitDepth.to_ulong();
      UINT32 w = inf->width;
      UINT32 h = inf->height;
      int size = std::snprintf(nullptr, 0, infoStr, w, h, colstr, bd, intstr);
      char str[size + 1]; // +1 for null termination.
      std::sprintf(str, infoStr, w, h, colstr, bd, intstr);
      wxMessageBox(str, "image Info", wxOK | wxICON_INFORMATION, this);
   }
}

void MyFrame::OnLayo(wxCommandEvent &/*event*/) {
}
 
void MyFrame::OnExit(wxCommandEvent &/*event*/) {
   Close(true);
}
void MyFrame::OnUndo(wxCommandEvent &/*event*/) {
   wxLogMessage("TODO");
}

void MyFrame::OnRedo(wxCommandEvent &/*event*/) {
   wxLogMessage("TODO");
}

void MyFrame::OnAbout(wxCommandEvent &/*event*/) {
   wxMessageBox(aboutStr, "About", wxOK | wxICON_INFORMATION, this);
}

#else  // POSIX
#error nonWin or nonPosix not implemented yet.
#endif  // POSIX
#endif  // WIN32
