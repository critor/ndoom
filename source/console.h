#define MAX_COL SCREEN_WIDTH/CHAR_WIDTH
#define MAX_LGN SCREEN_HEIGHT/CHAR_HEIGHT
#define LSEPARATOR "----------------------------------------"

void dispBuf( unsigned char* buf, char* message, int ret );
void displnBuf( unsigned char* buf, char* message, int ret );
void resetConsole();
//void pause( char* message, int ret, int invite);
