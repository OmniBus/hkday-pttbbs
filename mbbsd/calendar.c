/* $Id: calendar.c 5776 2013-01-23 10:38:22Z piaip $ */
#include "bbs.h"

// TODO deprecate this file.

typedef struct event_t {
    int             year, month, day, days;
    int             color;
    char           *content;
    struct event_t *next;
}               event_t;

static int
MonthDay(int m, int leap)
{
    int      day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    assert(1<=m && m<=12);
    return leap && m == 2 ? 29 : day[m - 1];
}

static int
Days(int y, int m, int d)
{
    int             i, w;

    w = 1 + 365 * (y - 1)
	+ ((y - 1) / 4) - ((y - 1) / 100) + ((y - 1) / 400)
	+ d - 1;
    for (i = 1; i < m; i++)
	w += MonthDay(i, is_leap_year(y));
    return w;
}

/**
 * return 1 if date and time is invalid
 */
int ParseDateTime(const char *date, int *year, int *month, int *day,
		  int *hour, int *min, int *sec)
{
    char           *y, *m, *d, *hh, *mm, *ss;
    char           buf[128];
    char *strtok_pos;

    strlcpy(buf, date, sizeof(buf));
    y = strtok_r(buf, "/", &strtok_pos); if (!y) return 1;
    m = strtok_r(NULL, "/", &strtok_pos);if (!m) return 1;
    d = strtok_r(NULL, " ", &strtok_pos); if (!d) return 1;

    if (hour) { 
	hh = strtok_r(NULL, ":", &strtok_pos); 
	if (!hh) return 1; 
	*hour = atoi(hh);
    }
    if (min ) { 
	mm = strtok_r(NULL, ":", &strtok_pos); 
	if (!mm) return 1; 
	*min  = atoi(mm);
    }
    if (sec ) { 
	ss = strtok_r(NULL, "",  &strtok_pos); 
	if (!ss) return 1; 
	*sec  = atoi(ss);
    }

    *year = atoi(y);
    *month = atoi(m);
    *day = atoi(d);

    if (hour && (*hour < 0 || *hour > 23)) return 1;
    if (min  && (*min  < 0 || *min  > 59)) return 1;
    if (sec  && (*sec  < 0 || *sec  > 59)) return 1;

    if (*year < 1 || *month < 1 || *month > 12 ||
	*day < 1 || *day > MonthDay(*month, is_leap_year(*year)))
	return 1;
    return 0;
}

/**
 * return 1 if date is invalid
 */
int ParseDate(const char *date, int *year, int *month, int *day)
{
    return ParseDateTime(date, year, month, day, NULL, NULL, NULL);
}

/**
 * return 1 if date is invalid
 */
static int
ParseEventDate(const char *date, event_t * t)
{
    int retval = ParseDate(date, &t->year, &t->month, &t->day);
    if (retval)
	return retval;
    t->days = Days(t->year, t->month, t->day);
    return retval;
}

static int
ParseColor(const char *color)
{
    struct {
	char           *str;
	int             val;
    }               c[] = {
	{ "black", 0 },
	{ "red", 1 },
	{ "green", 2 },
	{ "yellow", 3 },
	{ "blue", 4 },
	{ "magenta", 5 },
	{ "cyan", 6 },
	{ "white", 7 }
    };
    int             i;

    for (i = 0; (unsigned)i < sizeof(c) / sizeof(c[0]); i++)
	if (strcasecmp(color, c[i].str) == 0)
	    return c[i].val;
    return 7;
}

static void
InsertEvent(event_t * head, event_t * t)
{
    event_t        *p;

    for (p = head; p->next && p->next->days < t->days; p = p->next);
    t->next = p->next;
    p->next = t;
}

static void
FreeEvent(event_t * e)
{
    event_t        *n;

    while (e) {
	n = e->next;
	free(e->content); /* from strdup() */
	free(e);
	e = n;
    }
}

static event_t *
ReadEvent(int today)
{
    FILE           *fp;
    char            buf[PATHLEN];
    event_t  head;

    head.next = NULL;
    sethomefile(buf, cuser.userid, "calendar");
    fp = fopen(buf, "r");
    if (fp) {
	while (fgets(buf, sizeof(buf), fp)) {
	    char           *date, *color, *content;
	    event_t        *t;
	    char *strtok_pos;

	    if (buf[0] == '#')
		continue;

	    date = strtok_r(buf, " \t\n", &strtok_pos);
	    color = strtok_r(NULL, " \t\n", &strtok_pos);
	    content = strtok_r(NULL, "\n", &strtok_pos);
	    if (!date || !color || !content)
		continue;

	    t = malloc(sizeof(event_t));
	    if (ParseEventDate(date, t) || t->days < today) {
		free(t);
		continue;
	    }
	    t->color = ParseColor(color) + 30;
	    for (; *content == ' ' || *content == '\t'; content++);
	    t->content = strdup(content);
	    InsertEvent(&head, t);
	}
	fclose(fp);
    }
    return head.next;
}

static char   **
AllocCalBuffer(int line, int len)
{
    int             i;
    char          **p;

    p = malloc(sizeof(char *) * line);
    p[0] = malloc(sizeof(char) * line * len);
    for (i = 1; i < line; i++)
	p[i] = p[i - 1] + len;
    return p;
}

static void
FreeCalBuffer(char **buf)
{
    free(buf[0]);
    free(buf);
}

#define CALENDAR_COLOR  ANSI_COLOR(0;30;47)
#define HEADER_COLOR    ANSI_COLOR(1;44)
#define HEADER_SUNDAY_COLOR    ANSI_COLOR(31)
#define HEADER_DAY_COLOR       ANSI_COLOR(33)

static int
GenerateCalendar(char **buf, int y, int m, int today, event_t * e)
{
    char    *week_str[7] = {"日", "一", "二", "三", "四", "五", "六"};
    char    *month_color[12] = {
	ANSI_COLOR(1;32), ANSI_COLOR(1;33), ANSI_COLOR(1;35), ANSI_COLOR(1;36),
	ANSI_COLOR(1;32), ANSI_COLOR(1;33), ANSI_COLOR(1;35), ANSI_COLOR(1;36),
	ANSI_COLOR(1;32), ANSI_COLOR(1;33), ANSI_COLOR(1;35), ANSI_COLOR(1;36)
    };
    char    *month_str[12] = {
	"一月  ", "二月  ", "三月  ", "四月  ", "五月  ", "六月  ",
	"七月  ", "八月  ", "九月  ", "十月  ", "十一月", "十二月"
    };

    char           *p, attr1[16], *attr2;
    int             i, d, w, line = 0, first_day = Days(y, m, 1);


    /* week day banner */
    p = buf[line];
    p += sprintf(p, "    %s%s%s%s", HEADER_COLOR, HEADER_SUNDAY_COLOR,
		 week_str[0], HEADER_DAY_COLOR);
    for (i = 1; i < 7; i++)
	p += sprintf(p, " %s", week_str[i]);
    p += sprintf(p, ANSI_RESET);

    /* indent for first line */
    p = buf[++line];
    p += sprintf(p, "    %s", CALENDAR_COLOR);
    for (i = 0, w = first_day % 7; i < w; i++)
	p += sprintf(p, "   ");

    /* initial event */
    for (; e && e->days < first_day; e = e->next);

    d = MonthDay(m, is_leap_year(y));
    for (i = 1; i <= d; i++, w = (w + 1) % 7) {
	attr1[0] = 0;
	attr2 = "";
	while (e && e->days == first_day + i - 1) {
	    sprintf(attr1, ANSI_COLOR(1;%d), e->color);
	    attr2 = CALENDAR_COLOR;
	    e = e->next;
	}
	if (today == first_day + i - 1) {
	    strlcpy(attr1, ANSI_COLOR(1;37;42), sizeof(attr1));
	    attr2 = CALENDAR_COLOR;
	}
	p += sprintf(p, "%s%2d%s", attr1, i, attr2);

	if (w == 6) {
	    p += sprintf(p, ANSI_RESET);
	    p = buf[++line];
	    /* show month */
	    if (line >= 2 && line <= 4)
		p += sprintf(p, "%s%2.2s\33[m  %s", month_color[m - 1],
			     month_str[m - 1] + (line - 2) * 2,
			     CALENDAR_COLOR);
	    else if (i < d)
		p += sprintf(p, "    %s", CALENDAR_COLOR);
	} else
	    *p++ = ' ';
    }

    /* fill up the last line */
    if (w) {
	for (w = 7 - w; w; w--)
	    p += sprintf(p, w == 1 ? "  " : "   ");
	p += sprintf(p, ANSI_RESET);
    }
    return line + 1;
}

int
calendar(void)
{
    char          **buf;
    struct tm       snow;
    int             i, y, m, today, lines = 0;
    event_t        *head = NULL, *e = NULL;

    /* initialize date */
    localtime4_r(&now, &snow);
    today = Days(snow.tm_year + 1900, snow.tm_mon + 1, snow.tm_mday);
    y = snow.tm_year + 1900, m = snow.tm_mon + 1;

    /* read event */
    head = e = ReadEvent(today);

    /* generate calendar */
    buf = AllocCalBuffer(22, 256);
    for (i = 0; i < 22; i++)
	sprintf(buf[i], "%24s", "");
    for (i = 0; i < 3; i++) {
	lines += GenerateCalendar(buf + lines, y, m, today, e) + 1;
	if (m == 12)
	    y++, m = 1;
	else
	    m++;
    }

    /* output */
    clear();
    outc('\n');
    for (i = 0; i < 22; i++) {
	outs(buf[i]);
	if (i == 0) {
	    prints("\t" ANSI_COLOR(1;37)
		    "現在是 %d.%02d.%02d %2d:%02d:%02d%cm" ANSI_RESET, 
		   snow.tm_year + 1900, snow.tm_mon + 1, snow.tm_mday,
		   (snow.tm_hour == 0 || snow.tm_hour == 12) ?
		   12 : snow.tm_hour % 12, snow.tm_min, snow.tm_sec,
		   snow.tm_hour >= 12 ? 'p' : 'a');
	} else if (i >= 2 && e) {
	    prints("\t" ANSI_COLOR(1;37) 
		    "(尚有 " ANSI_COLOR(%d) "%3d"
		    ANSI_COLOR(37) " 天)"
		    ANSI_RESET " %02d/%02d %s",
		   e->color, e->days - today,
		   e->month, e->day, e->content);
	    e = e->next;
	}
	outc('\n');
    }
    FreeEvent(head);
    FreeCalBuffer(buf);
    i = vmsg("行事曆將於2013/06後擇期取消。要檢視現有行事曆請按 e。");
    i = tolower(((unsigned char)i) & 0xFF);
    if (i == 'e')
    {
        char fpath[PATHLEN];
        setuserfile(fpath, "calendar");
        more(fpath, YEA);
    }
    return 0;
}

