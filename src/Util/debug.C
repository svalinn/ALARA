/* $Id: debug.C,v 1.4 2000-02-19 05:44:48 wilson Exp $ */
#include "alara.h"

int verb_level = 0;
int debug_level = -1;

void verbose(int msg_level, char *msgFmt, ...)
{

  static char msg[1024];
  
  if (msg_level < verb_level)
    {
      /* make message from variable argument list */
      va_list args;
      va_start(args, msgFmt);
      vsprintf(msg,msgFmt,args);
      va_end(args);

      while (msg_level-- >0)
	cout << "\t";
      cout << msg << endl;
    }

}

void debug(int msg_level, char *msgFmt, ...)
{

  static char msg[1024];

  if (msg_level < debug_level)
    {
      /* make message from variable argument list */
      va_list args;
      va_start(args, msgFmt);
      vsprintf(msg,msgFmt,args);
      va_end(args);

      while (msg_level--)
	cout << "\t";
      cout << "**debug: " << msg << endl;
    }

}

void error(int error_num, char *msgFmt, ...)
{
  static char msg[1024];

  /* make message from variable argument list */
  va_list args;
  va_start(args, msgFmt);
  vsprintf(msg,msgFmt,args);
  va_end(args);

  cerr << "Error #" << error_num << ": " << msg << endl;
  exit(error_num);
}

void memCheck(void* ptr, const char *msg)
{
  if (ptr == NULL)
    error(-1,"Memory allocation error: %s",msg);
}

void warning(int error_num, char *msgFmt, ...)
{
  static char msg[1024];

  /* make message from variable argument list */
  va_list args;
  va_start(args, msgFmt);
  vsprintf(msg,msgFmt,args);
  va_end(args);

  cerr << "Warning #" << error_num << ": " << msg << endl;
}
