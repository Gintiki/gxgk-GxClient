#define _Stringizing(v)    #v
#define _VerJoin(a, b, c, d)  _Stringizing(a.b.c.d)

#define VER_Major    1
#define VER_Minor    1
#define VER_Release  3323       
#define VER_Build    3323       
#define VER_Version    MAKELONG(MAKEWORD(VER_Major, VER_Minor), VER_Release)

#define STR_BuildDate    TEXT(__DATE__)
#define STR_BuildTime    TEXT(__TIME__)
#define STR_BuilDateTime  TEXT(__DATE__) TEXT(" ") TEXT(__TIME__)
#define STR_Version    TEXT(_VerJoin(VER_Major, VER_Minor, VER_Release, VER_Build))

#define MAX_STRING 2048
#define CONFIGNAME TEXT("gxgk.conf")

#ifdef _Debug
#define STR_AppName    TEXT("ݸ��ͻ���(�ڲ��)")
#else
#define STR_AppName    TEXT("ݸ��ͻ���")
#endif

#define STR_Author    TEXT("YZX")
#define STR_Corporation    TEXT("ݸ����")
#define STR_Modify    TEXT("���������")

#define STR_WEB      TEXT("www.gxgk.cc")
#define STR_WEB_URL    TEXT("http://") STR_WEB
#define STR_AUTHOR_URL TEXT("http://bbs.gxgk.cc/space-uid-207.html")
#define STR_Modify_URL	TEXT("http://www.lastfighting.com")


#define STR_Description    TEXT("ݸ����")
#define STR_Copyright    TEXT("��Ȩû�� (C) 2009-2015 ")
#define Copyright     TEXT(STR_Copyright STR_Description)