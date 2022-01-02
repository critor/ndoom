#include <os.h>
#include "dirlist.h"


int dirlist(char* path, char* pattern, char** result)
{ unsigned char StructDstat[512];
  chdir(path);
  if (NU_Get_First((struct dstat*)StructDstat, pattern)) {
    printf("Error: dirlist() failed to find file\n");
    NU_Done((struct dstat*)StructDstat);
    return -1;
  }
  int i = 0;
  do {
    char* dirname = (char*) malloc(128);
    strcpy(dirname, (char*) &StructDstat[13]);
    result[i] = dirname;
    i++;
  } while (NU_Get_Next((struct dstat*)StructDstat) == 0); 
  NU_Done((struct dstat*)StructDstat);
  return i;
}
