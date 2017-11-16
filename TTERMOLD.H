class TTerm : public PACKMONWnd{
	public:
		TTerm(TWindow*, unsigned int, char*, int);
		virtual void SetupWindow(void);

		void CloseWindow(int retval);
		void SetScrollDim(UINT TotalHeight);

		LRESULT WMFONTS(unsigned int, long);
    void EvPaint();
		void Paint(TDC &, bool, TRect & );
		void InsertTerm(unsigned char *, int);
		long CountLines(void);

		HFONT hfont;
		int TextHeight;

		char *Buffer;
		int QueFront;
		int QueBack;
		int BufferSize;
		bool FlagFullBuffer;
		bool CanIClose;

		DECLARE_RESPONSE_TABLE(TTerm);
};



