

//class TDMLClApp : public TApplication
class PM_APPL : public TApplication {
	public:

    // Constructor
		PM_APPL() : TApplication(),CallBackProc((FARPROC)CallBack) {
			InstId = 0;
		}

		void InitInstance();
    		// Performs each instance initialization necessary for the application.
        // Unlike InitApplication, which is called for the first instance of an
        // application, InitInstance is called whether or not there are other
        // executing instances of the application.
        // InitInstance calls InitMainWindow, and then creates and shows the
        // main window element by TWindow::Create and TWindow::Show.
        // If the main window cannot be created, a TXInvalidMainWindow exception is thrown.
				// If you redefine this member function, be sure to explicitly
        // call TApplication::InitInstance.

		void InitMainWindow();
        // By default, InitMainWindow constructs a generic TFrameWindow object
        // with the name of the application as its caption. You can redefine
        // InitMainWindow to construct a useful main window object of
        // TFrameWindow (or a class derived from TFrameWindow) and store it in
        // MainWindow. The main window must be a top-level window; that is, it
        // must be derived from TFrameWindow.

		int TermInstance(int status);
     		// Handles the termination of each executing instance of an ObjectWindows application.

		static HDDEDATA FAR PASCAL _export CallBack(WORD, WORD, HCONV, HSZ, HSZ,
																								HDDEDATA, DWORD, DWORD);
		TProcInstance CallBackProc;
    		// Designed for Win16 applications, TProcInstance handles creating and
        // freeing an instance thunk, a piece of code created for use with
        // exported callback functions. (A callback function is a function that
        // exists within a program but is called from outside the program by a
        // Windows library routine, for example, a dialog box function.)
				// For Win32 applications, TProcInstance is non-functional.
        // The address returned from TProcInstance can be passed as a parameter
        // to callback functions, window subclassing functions, or Windows dialog
        // box functions.

    // Variables
		DWORD   InstId;

};

class PM_GLOBAL {
	public:
    // Other global varialbles
    bool SoundEffects;
    bool ShowDataOnly;
};


#define FRAME_UNKNOWN 0x00
#define FRAME_UI   		0x01
#define FRAME_UA   		0x02
#define FRAME_RR   		0x03
#define FRAME_REJ  		0x04
#define FRAME_DM   		0x05
#define FRAME_DISC 		0x06
#define FRAME_I		 		0x07
#define FRAME_SABM 		0x08

class FRAME {
  public:
    int  frametype;
 		void set(char *s);
		char *getstr(void);
    int  get() { return(frametype); };
};

class PACKMONWnd : public TFrameWindow, PM_GLOBAL {
	public:

  	// Constructor and Destructor
		PACKMONWnd(TWindow *parent, const char *Window_title);
		virtual ~PACKMONWnd();

		virtual void SetupWindow();

    // DDE functions
		int PM_Init_DDE();
		int PM_Exit_DDE();
    int DDE_Connected;
    void TXDATA(char *str,int count);

		void SetScrollDim();
		LRESULT WMFONTS(WPARAM wp,LPARAM);

		void EvPaint();
		void pmWinPaint(TDC& dc, bool erase, TRect& rect);
    char *GetLine(int start);

    void DispMonText(char *s);
    void get_PEheard(void);			// Get heard list from packet engine.

    void EvInitMenu(HMENU menuHandle);
    void CmConnect(void);
    void CmDisConnect(void);
    void CmFileOpen(void);
    void CmToggleSoundEffects(void);
    void CmToggleDataOnly(void);
    void CmHelpAbout(void);

    // Dummies
    void CmRequest(void);
    void CmPoke(void);
    void CmAdvise(void);

    ////////////////////////////
    // Variables
    ////////////////////////////

    // Font variables
		LOGFONT   cursfont;         // font structure
		HFONT     holdsfont;        // handle of original font
		HFONT     hnewsfont;        // handle of new fixed font
 		HFONT hfont;
		int TextHeight;

    // Global window variables
  	bool CanIClose;


    // Instance variables
		DWORD   InstId() {
			return ((PM_APPL*)GetApplication())->InstId;
			}

    // DDE variables
		HCONV   hConv[2];
		HSZ     Service;
		HSZ     Topic;
		HSZ     Item;
		char    Data[128];

    void ReceivedData(HDDEDATA);
    void ReceivedData(char *s);
    LRESULT  Ev_Msg_Char(WPARAM wp,LPARAM);

    bool IsMultiCall(char *call);

	DECLARE_RESPONSE_TABLE(PACKMONWnd);
};


