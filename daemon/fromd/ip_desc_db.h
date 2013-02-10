// $Id: ip_desc_db.h 4092 2008-04-07 16:52:37Z wens $
#ifndef _IP_DESC_DB_H
#define _IP_DESC_DB_H
int ip_desc_db_reload(const char * cfgfile);
const char * ip_desc_db_lookup(const char * ip);
#endif
