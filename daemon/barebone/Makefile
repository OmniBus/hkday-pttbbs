# $Id: Makefile 5362 2011-06-25 15:10:09Z wens $

SRCROOT=	../..
.include "$(SRCROOT)/pttbbs.mk"

PROG=	server
SRCS=	server.c
MAN=

CFLAGS+=	$(LIBEVENT_CFLAGS)
LDFLAGS+=	$(LIBEVENT_LIBS_L)

LDADD+=	$(LIBEVENT_LIBS_l)

.include <bsd.prog.mk>
