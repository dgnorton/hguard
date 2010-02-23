#pragma warning(disable : 4996)

#include <stdio.h>
#include <string>
#include <algorithm>
#include <windows.h>
#include "boost/uuid/uuid.hpp"

using namespace std;
using namespace boost::uuids;

// copy text to clipboard
bool strtocb(const char *s, size_t size) {
   HGLOBAL glob = GlobalAlloc(GMEM_MOVEABLE, size + 1);
   if (glob) {
      char* dest = (char*)GlobalLock(glob);
      if (dest) {
         OpenClipboard(NULL);
         EmptyClipboard();
         strcpy(dest, s);
         GlobalUnlock(glob);
         SetClipboardData(CF_TEXT, glob);
         CloseClipboard();
         return true;
      }
   }
   return false;
}

// convert any char in "from" to "to"
struct char2char {
   char2char(const char* from, char to) : m_from(from), m_to(to) {}
   int operator()(int c) { return (::strchr(m_from, c) ? m_to : c); }
   const char* m_from;
   int m_to;
};

// takes header file name on command line and
// writes an include guard to the clipboard
int main(int argc, char* argv[]) {
   if (argc < 2) {
      printf("usage: hguard <header-name>\r\n");
      return 1;
   }
   // build ID string (ex, "MyHeader.h_a2779bb4-5db4-4518-b0db-d40527864f35")
   string id(argv[1]);
   uuid_generator gen;
   id.append("_").append(gen().to_string());
   // convert to upper case
   transform(id.begin(), id.end(), id.begin(), ::toupper);
   // replace '-' and '.' with '_'
   char2char replacer("-.", '_');
   transform(id.begin(), id.end(), id.begin(), replacer);
   // build #ifdef / #define string
   string hguard("#ifndef ");
   hguard.append(id).append("\r\n");
   hguard.append("#define ").append(id).append("\r\n\r\n\r\n\r\n");
   hguard.append("#endif // ").append(id);
   // send it to the clipboard
   strtocb(hguard.c_str(), hguard.size());

   return 1;
}
