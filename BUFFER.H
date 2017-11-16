
class pmBuff {
  public:
      // Constructor
    pmBuff(int size);

    //Functions
    void insert(char *s, int count);
    char *GetLine(int start);
    long CountLines(void);
    void strsetsz(char *s, int m);
    void reset(void);

    // Variables
		char *Buffer;
		int QueFront;
		int QueBack;
		int BufferSize;
		int FlagFullBuffer;
};

