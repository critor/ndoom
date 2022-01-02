#include <os.h>
#include "screen.h"
#include "console.h"
#include "tools.h"
#include "charmap.h"

int col=0;
int line=0;

int prx=-1;
int pry=-1;

void dispBuf( unsigned char* buf, char* message, int ret )
{ int l = strlen(message);
  int i, stop=0;
  for (i = 0; i < l && !stop; i++) {
    if (message[i] == 0x0A) {
      if ( ret )
      { col = 0;
        line ++;
      }
      else
      { putBufChar(buf, col*CHAR_WIDTH, line*CHAR_HEIGHT, ' ', 0);
        col++;
      }
    } else {
      putBufChar(buf, col*CHAR_WIDTH, line*CHAR_HEIGHT, message[i], 0);
      col ++;
    }
    if (col >= MAX_COL)
    { if ( !ret ) stop=1;
      else
      { col = 0;
        line ++;
      }
    }
    if(line>=MAX_LGN) { line=0; }
  }
}

void displnBuf( unsigned char* buf, char* message, int ret )
{	dispBuf(buf, message, ret);
	col=0;
	line++;
	if(line>=MAX_LGN) { line=0; }
}

/*
void pause( char* message, int ret, int invite )
{	if(message)	displn(message, ret);
	if(invite)	displn("Press Esc to continue...", 0);
	while(!isKeyPressed(KEY_NSPIRE_ESC)) {}
}
*/

void resetConsole()
{	col=0;
	line=0;
}