/* Imported from the dvbstream project
 *
 * Modified for use with MPlayer, for details see the CVS changelog at
 * http://www.mplayerhq.hu/cgi-bin/cvsweb.cgi/main/
 * $Id: rtp.h,v 1.4 2005/04/16 12:51:09 diego Exp $
 */

#ifndef _RTP_H
#define _RTP_H

#include "config.h"
#ifndef HAVE_WINSOCK2
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif

struct rtpbits {
  unsigned int v:2;           /* version: 2 */
  unsigned int p:1;           /* is there padding appended: 0 */
  unsigned int x:1;           /* number of extension headers: 0 */
  unsigned int cc:4;          /* number of CSRC identifiers: 0 */
  unsigned int m:1;           /* marker: 0 */
  unsigned int pt:7;          /* payload type: 33 for MPEG2 TS - RFC 1890 */
  unsigned int sequence:16;   /* sequence number: random */
};

struct rtpheader {	/* in network byte order */
  struct rtpbits b;
  int timestamp;	/* start: random */
  int ssrc;		/* random */
};


int getrtp2(int fd, struct rtpheader *rh, char** data, int* lengthData);

#endif
