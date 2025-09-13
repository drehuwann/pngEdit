// pngEditor.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS D_WINSOCKAPI_ /c

#include <memory>
#ifdef WIN32
#include "utf4win.h"
#include <tchar.h>
#include <basetsd.h>
#else  // WIN32
#ifdef POSIX
#include <wx/wx.h>
#include <wx/filedlg.h>
#include <wx/treectrl.h>
#endif  // POSIX
#endif  // WIN32
#include "defs.h"
#include "htonntoh.h"
#include "engine.h"
#include "view.h"
#include <array>

//Common constants
constexpr int SIZE_X = 500;
constexpr int SIZE_Y = 300;
constexpr const char *const aboutStr =
   "\tpng Editor was initially a quick home made tool to work on small 16*16 "
   "icons defined in png files.\r\nIt growed becoming a tool to check eventual"
   " steganography embedded in .png files.\r\nCopyright drehuwann@gmail.com\r"
   "\nPublished under the terms of the General Public License.\r\n(See https:"
   "//gnu.org/licenses/gpl.html)";
constexpr const char *const infoStr = "Dimensions(WxH) : %ux%u\r\nColourType/BitDepth "
": %s/%lu bit(s)\r\nInterlace : %s";

constexpr std::array<const char *const, 8> colourStr = {
   "Greyscale",
   "UNKNOWN !",
   "Truecolour",
   "Indexed-colour",
   "Greyscale with alpha",
   "UNKNOWN!",
   "Truecolour with alpha",
   "UNKNOWN!"
};

constexpr std::array<const char *const, 2> interlaceStr = {
   "No interlace", "Adam 7"
};

// common functions
Engine *InitEngine(HWND hWnd) {
   if (!hWnd) return nullptr;
   auto *toRet = &Engine::Instance();
   if (! toRet) return nullptr;
   auto *modl = new Model(toRet);
   if (!modl) return nullptr;
   auto *ctrl = new Controller(toRet);
   if (!ctrl) {
      if (modl) delete modl;
      return nullptr;
   }
   auto *view = new View(hWnd);
   if (!view) {
      if (ctrl) delete ctrl;
      if (modl) delete modl;
      return nullptr;
   }
   if (toRet->Init(modl, view, ctrl) != Error::NONE) {
      if (ctrl) delete ctrl;
      if (modl) delete modl;
      if (view) delete view;
      toRet = nullptr;         
   }
   return toRet;
}

#ifdef WIN32
/// @brief 
/// @return created treeView HWND on ^Ms, MyFrame* on wxWidgets, nullptr on error. 
HWND CreateATreeView(HWND hwndParent) {
   HWND hwndTV = nullptr;    // handle to tree-view control
   RECT rcClient;  // dimensions of client area
   
   // Ensure that the common control DLL is loaded.
   INITCOMMONCONTROLSEX iccs;
   iccs.dwICC = ICC_TREEVIEW_CLASSES;
   iccs.dwSize = sizeof(INITCOMMONCONTROLSEX);
   InitCommonControlsEx(&iccs);
   
   // Get the dimensions of the parent window's client area, and create
   // the tree-view control.
   GetClientRect(hwndParent, &rcClient);
   hwndTV = CreateWindowEx(
      0, WC_TREEVIEW, TEXT("Tree View"),
      WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES,
      0, 0, rcClient.right, rcClient.bottom, hwndParent,
      nullptr, nullptr, nullptr);
   return hwndTV;
}

/// The main window class name.
static const TCHAR szWindowClass[] = _T("DesktopApp");

/// The string that appears in the application's title bar.
static const TCHAR szTitle[] = _T("png Editor");

/// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void About(HWND hWnd) {
   LPCTSTR tstr = FromCstr(aboutStr);
   MessageBox(hWnd, tstr, _T("About"), MB_OK); 
   if (tstr && tstr != (LPCTSTR)aboutStr) delete tstr;               
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
   wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
   wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
   wcex.lpszMenuName   = nullptr;
   wcex.lpszClassName  = szWindowClass;
   wcex.hIconSm        = LoadIcon(wcex.hInstance, IDI_APPLICATION);

   if (!RegisterClassEx(&wcex)) {
      MessageBox(nullptr, _T("Call to RegisterClassEx failed!"), szTitle, 0);
      return 1;
   }
   if (hPrevInstance) {
      //suppress warning not-used
   }
   if (lpCmdLine) {
      //TODO args parsing
   }

   HWND hWnd = CreateWindowEx(
      WS_EX_OVERLAPPEDWINDOW,
      szWindowClass,
      szTitle,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT,
      SIZE_X, SIZE_Y,
      nullptr,
      nullptr,
      hInstance,
      nullptr
   );

   if (!hWnd) {
      MessageBox(nullptr, _T("Call to CreateWindow failed!"), szTitle, 0);
      return 1;
   }

   // The parameters to ShowWindow explained:
   // hWnd: the value returned from CreateWindow
   // nCmdShow: the fourth parameter from WinMain
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // Main message loop:
   MSG msg;
   while (GetMessage(&msg, nullptr, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
   PAINTSTRUCT ps;
   HDC hdc;
   const TCHAR greeting[] = _T("Hello, png Editor !");
   switch (message) {
      case WM_CREATE: {
         HMENU hMenubar = CreateMenu();
         HMENU hFile = CreateMenu();
         HMENU hEdit = CreateMenu();
         HMENU hHelp = CreateMenu();
         AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hFile, _T("File"));
         AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hEdit, _T("Edit"));
         AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hHelp, _T("Help"));
         AppendMenu(hFile, MF_STRING, static_cast<UINT_PTR>(MenuID::ID_Save), _T("Save"));
         AppendMenu(hFile, MF_STRING, static_cast<UINT_PTR>(MenuID::ID_Load), _T("Open"));
         AppendMenu(hFile, MF_SEPARATOR, 0, nullptr);
         AppendMenu(hFile, MF_STRING, static_cast<UINT_PTR>(MenuID::ID_Info), _T("Show Info"));
         AppendMenu(hFile, MF_STRING, static_cast<UINT_PTR>(MenuID::ID_Layo), _T("Show Layout"));
         AppendMenu(hFile, MF_SEPARATOR, 0, nullptr);
         AppendMenu(hFile, MF_STRING, static_cast<UINT_PTR>(MenuID::ID_Exit), _T("Exit"));
         AppendMenu(hEdit, MF_STRING, static_cast<UINT_PTR>(MenuID::ID_Undo), _T("Undo"));
         AppendMenu(hEdit, MF_STRING, static_cast<UINT_PTR>(MenuID::ID_Redo), _T("Redo"));
         AppendMenu(hHelp, MF_STRING, static_cast<UINT_PTR>(MenuID::ID_Abou), _T("About"));
         SetMenu(hWnd, hMenubar);
         if (!InitEngine(hWnd)) return 1; //TODO catch this!
         break;
      }
      case WM_COMMAND: {
         if (LOWORD(wParam) == static_cast<UINT_PTR>(MenuID::ID_Save)) {
            //TODO 'Save' methods
         }
         if (LOWORD(wParam) == static_cast<UINT_PTR>(MenuID::ID_Load)) {
            OPENFILENAME ofn;       // common dialog box structure. created on the stack ?
            std::array<char, 280> szFile;       // buffer for file name
            // Initialize OPENFILENAME
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = (LPTSTR)&szFile;
            // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
            // use the contents of szFile to initialize itself.
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = (LPCTSTR)(_T("png\0*.PNG;*.png\0"));
            ofn.nFilterIndex = 0;
            ofn.lpstrFileTitle = nullptr;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = nullptr;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            
            // Display the Open dialog box.
            if (GetOpenFileName(&ofn)==TRUE) {
               const char *utfPath = ToCstr(ofn.lpstrFile);
               Model *mod = Engine::Instance().GetModel();
               mod->PickFile(utfPath);
               if (utfPath && utfPath != (const char *)ofn.lpstrFile) {
                  delete utfPath;
               }
            }
            break;
         }
         if (LOWORD(wParam) == static_cast<UINT_PTR>(MenuID::ID_Info)) {
            auto inf = Engine::Instance().GetModel()->GetInfo();
            if (inf == nullptr) {
               MessageBox(hWnd, _T("There is no image info available.\nDid you load a valid \
*.png file before querying info ?"), _T("WARNING"), 0);
            } else {
               const char *colstr = colourStr[inf->bitfield.colourType.to_ulong()];
               const char *intstr = interlaceStr[inf->interlace ? 1 : 0];
               unsigned long bd = inf->bitfield.bitDepth.to_ulong();
               UINT32 w = inf->width;
               UINT32 h = inf->height;
               int size = std::snprintf(nullptr, 0, infoStr, w, h, colstr, bd, intstr);
               size ++; // +1 for null termination.
               auto *str = (char *)(malloc((size_t)size));
               sprintf_s(str, size, infoStr, w, h, colstr, bd, intstr);
               LPCTSTR tstr = FromCstr((const char *)str);
               MessageBox(hWnd, tstr, _T("image Info"), 0);
               if (tstr && tstr != (LPCTSTR)str) {
                  delete tstr;
               }
               if (str) free(str);
            }
         }
         if (LOWORD(wParam) == static_cast<UINT_PTR>(MenuID::ID_Layo)) {
            Engine &eng = Engine::Instance();
            const Controller *ctrl = eng.GetController();
            Chunk *head = eng.GetModel()->GetChunksHead();
            if (ctrl == nullptr) {
               MessageBox(hWnd, _T("There is no controller available."),
                  _T("CRITICAL"), 0);
            } else if (head == nullptr) {
               MessageBox(hWnd, _T("There is no chunk layout available.\n\
Did you load a valid *.png file before querying layout ?"), _T("WARNING"), 0);
            } else {
               HWND MyLayoutView = CreateATreeView(hWnd);
               TVITEMEX root;
               root.mask = TVIF_TEXT;
               root.pszText = _T("Chunks layout");
               TVINSERTSTRUCT node;
               node.hInsertAfter = TVI_ROOT;
               node.hParent = nullptr;
               node.itemex = root;
               auto hPrev = (HTREEITEM)SendMessage(MyLayoutView,
                  TVM_INSERTITEM, 0, (LPARAM)&node);
               while (head) {
                  std::array<TCHAR, 5> tag = {0, 0, 0, 0,0};
                  UINT32 tTag = hton(head->GetTypeTag());
                  tag[0] = (tTag & 0xff000000) >> 24;
                  tag[1] = (tTag & 0xff0000) >> 16;
                  tag[2] = (tTag & 0xff00) >> 8;
                  tag[3] = tTag & 0xff;
                  LPTSTR tstr = tag.data();
                  TVITEMEX current;
                  current.mask = TVIF_TEXT;
                  current.pszText = tstr;
                  node.hParent = hPrev;
                  node.hInsertAfter = TVI_FIRST;
                  node.itemex = current;
                  SendMessage(MyLayoutView, TVM_INSERTITEM, 0,
                     (LPARAM)&node);
                  head = head->GetPrevious();
               }
            }
         }
         if (LOWORD(wParam) == static_cast<UINT_PTR>(MenuID::ID_Exit)) {
            PostQuitMessage(0);
            exit(0);
         }
         if (LOWORD(wParam) == static_cast<UINT_PTR>(MenuID::ID_Undo)) {
            //TODO implement or remove
            break;
         }
         if (LOWORD(wParam) == static_cast<UINT_PTR>(MenuID::ID_Redo)) {
            //TODO implement or remove
            break;
         }
         if (LOWORD(wParam) == static_cast<UINT_PTR>(MenuID::ID_Abou)) {
            About(hWnd);
            break;
         }
         break;
      }
      case WM_PAINT:
         hdc = BeginPaint(hWnd, &ps);
         TextOut(hdc, 5, 5, greeting, (int)(_tcslen(greeting)));
         EndPaint(hWnd, &ps);
         break;
      case WM_DESTROY:
         PostQuitMessage(0);
         break;
      default:
         return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

#else  // WIN32
#ifdef POSIX
class MyApp : public wxApp {
public:
   bool OnInit() final;
};

class MyFrame : public wxFrame {
public:
   MyFrame(); 
private:
   void OnSave(const wxCommandEvent& event);
   void OnLoad(const wxCommandEvent& event);
   void OnInfo(const wxCommandEvent& event);
   void OnLayo(const wxCommandEvent& event);
   void OnExit(const wxCommandEvent& event);
   void OnUndo(const wxCommandEvent& event);
   void OnRedo(const wxCommandEvent& event);
   void OnAbou(const wxCommandEvent& event);
};

wxIMPLEMENT_APP(MyApp);
 
bool MyApp::OnInit() {
   auto frame = std::make_unique<MyFrame *>(new MyFrame);
   if (!frame) return false;
   if (const auto *engine = InitEngine(*(frame.get())); !engine) {
      if (frame) frame.reset();
      return false;
   }
   (*(frame.get()))->Show(true);
   return true;
}
 
MyFrame::MyFrame() : wxFrame(nullptr, wxID_ANY, "png Editor", wxDefaultPosition,
wxSize(SIZE_X, SIZE_Y)) {
   auto menuFile = std::make_unique<wxMenu *>(new wxMenu);
   (*(menuFile.get()))->Append(static_cast<int>(MenuID::ID_Save), "&Save\tCtrl-S",
            "Save current work as <fileneme>.sav");
   (*(menuFile.get()))->Append(static_cast<int>(MenuID::ID_Load), "&Open\tCtrl-O",
            "Loads a png file to work on");
   (*(menuFile.get()))->AppendSeparator();
   (*(menuFile.get()))->Append(static_cast<int>(MenuID::ID_Info), "&Info\tCtrl-I",
            "Show image Info");
   (*(menuFile.get()))->Append(static_cast<int>(MenuID::ID_Layo), "&Layout\tCtrl-L",
            "Show chunk layout");
   (*(menuFile.get()))->AppendSeparator();
   (*(menuFile.get()))->Append(wxID_EXIT);
   auto menuEdit = std::make_unique<wxMenu *>(new wxMenu);
   (*(menuEdit.get()))->Append(static_cast<int>(MenuID::ID_Undo), "&Undo\tCtrl-Z",
            "Undo last operation on image");
   (*(menuEdit.get()))->Append(static_cast<int>(MenuID::ID_Redo), "&Redo\tCtrl-Y",
            "Redo last undone operation on image");
   auto menuHelp = std::make_unique<wxMenu *>(new wxMenu);
   (*(menuHelp.get()))->Append(wxID_ABOUT);
   auto menuBar = std::make_unique<wxMenuBar *>(new wxMenuBar);
   (*(menuBar.get()))->Append((*(menuFile.get())), "&File");
   (*(menuBar.get()))->Append((*(menuEdit.get())), "&Edit");
   (*(menuBar.get()))->Append((*(menuHelp.get())), "&Help");
   MyFrame::SetMenuBar(*(menuBar.get()));
   MyFrame::CreateStatusBar();
   MyFrame::SetStatusText("Welcome to pngEditor!");
   Bind(wxEVT_MENU, &MyFrame::OnSave, this, static_cast<int>(MenuID::ID_Save));
   Bind(wxEVT_MENU, &MyFrame::OnLoad, this, static_cast<int>(MenuID::ID_Load));
   Bind(wxEVT_MENU, &MyFrame::OnInfo, this, static_cast<int>(MenuID::ID_Info));
   Bind(wxEVT_MENU, &MyFrame::OnLayo, this, static_cast<int>(MenuID::ID_Layo));
   Bind(wxEVT_MENU, &MyFrame::OnUndo, this, static_cast<int>(MenuID::ID_Undo));
   Bind(wxEVT_MENU, &MyFrame::OnRedo, this, static_cast<int>(MenuID::ID_Redo));
   Bind(wxEVT_MENU, &MyFrame::OnAbou, this, wxID_ABOUT);
   Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::OnSave(const wxCommandEvent &/*event*/) {
   wxLogMessage("TODO");
}

void MyFrame::OnLoad(const wxCommandEvent &/*event*/) {
   Model *mod = Engine::Instance().GetModel();
   wxFileDialog ofd(this, _("Open PNG file"), "", "",
         "PNG files (*.png)|*.png", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
   if (ofd.ShowModal() != wxID_CANCEL)
      mod->PickFile(ofd.GetPath().fn_str().data());
}

void MyFrame::OnInfo(const wxCommandEvent &/*event*/) {
   std::shared_ptr<s_imInfo> inf = Engine::Instance().GetModel()->GetInfo();
   if (inf == nullptr) {
      wxMessageBox("There is no image info available.\nDid you load a valid "
            "*.png file before querying info ?", "WARNING", wxOK | wxICON_WARNING,
            this
      );
   } else {
      const char *colstr = colourStr[inf->bitfield.colourType.to_ulong()];
      const char *intstr = interlaceStr[inf->interlace ? 1 : 0];
      ulong bd = inf->bitfield.bitDepth.to_ulong();
      UINT32 w = inf->width;
      UINT32 h = inf->height;
      int size = std::snprintf(nullptr, 0, infoStr, w, h, colstr, bd, intstr);
      const auto str = std::make_unique<char>(size + 1); // +1 for null termination.
      char *p_str = str.get();
      p_str[size] = 0;
      std::sprintf(p_str, infoStr, w, h, colstr, bd, intstr);
      wxMessageBox(p_str, "image Info", wxOK | wxICON_INFORMATION, this);
   }
}

void MyFrame::OnLayo(const wxCommandEvent &/*event*/) {
   Chunk *head = Engine::Instance().GetModel()->GetChunksHead();
   if (head == nullptr) {
      wxMessageBox("There is no chunk layout available.\nDid you load a valid "
"           *.png file before querying layout ?", "WARNING", wxOK | wxICON_WARNING,
            this
      );
   } else {
      auto MyLayoutView = std::make_unique<wxTreeCtrl *>(new wxTreeCtrl());
      wxTreeItemId root = (*(MyLayoutView.get()))->AddRoot("Chunks layout");
      while (head) {
         std::array<char, 5> tag;
         tag[4] = 0x00;
         UINT32 tTag = hton(head->GetTypeTag());
         std::sprintf(&tag[0], "%c%c%c%c",
            (tTag & 0xff000000) >> 24,
            (tTag & 0xff0000) >> 16,
            (tTag & 0xff00) >> 8,
            (tTag & 0xff)
         );
         (*(MyLayoutView.get()))->InsertItem(root, 0, wxString(tag.data()));
         head = head->GetPrevious();
      }
      int W;
      int H;
      this->DoGetClientSize(&W, &H);
      (*(MyLayoutView.get()))->Create(this, wxID_ANY, wxDefaultPosition, wxSize(W/4, H));
      (*(MyLayoutView.get()))->Show(true);
   }
}
 
void MyFrame::OnExit(const wxCommandEvent &/*event*/) {
   Close(true);
}

void MyFrame::OnUndo(const wxCommandEvent &/*event*/) {
   wxLogMessage("TODO");
}

void MyFrame::OnRedo(const wxCommandEvent &/*event*/) {
   wxLogMessage("TODO");
}

void MyFrame::OnAbou(const wxCommandEvent &/*event*/) {
   wxMessageBox(aboutStr, "About", wxOK | wxICON_INFORMATION, this);
}

#else  // POSIX
#error nonWin or nonPosix not implemented yet.
#endif  // POSIX
#endif  // WIN32
