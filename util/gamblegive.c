/* $Id: gamblegive.c 2840 2005-06-14 13:15:00Z in2 $ */
/* 爭議賭盤 產生紅包機格式 */
#define _UTIL_C_
#include "bbs.h"

int main(int argc, char **argv)
{
    int money = 12, num;
    char buf[512], *userid,*p;
    FILE *fp;

    if(argc<3 ||
    !(num=atoi(argv[1])) ||
    !(fp = fopen (argv[2], "r")) )
     {
       printf("%s <tickey-price> <result-file>", argv[0]);
       return 0;
     }

    while (fgets(buf,512,fp) )
	{
            if(strncmp(buf, "恭喜", 4)) continue; 
            userid = strtok(buf+5," ");
            p = strtok(NULL, " ");
            num = atoi(p+4);
           printf("%s:%d\n", userid, num*money); 
	}
    return 0;
}
