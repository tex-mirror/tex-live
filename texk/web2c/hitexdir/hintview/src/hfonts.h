/*375:*/
#line 7603 "hint.w"

#ifndef _HFONTS_H
#define _HFONTS_H

/*306:*/
#line 5692 "hint.w"

typedef enum{no_format,pk_format,ft_format}FontFormat;
/*:306*//*307:*/
#line 5704 "hint.w"

/*339:*/
#line 6684 "hint.w"


typedef struct
{unsigned char*pk_comment;
unsigned int cs;
double ds;
unsigned char id;
}PKfont;
/*:339*//*351:*/
#line 7015 "hint.w"

typedef struct
{FT_Face face;
}FTfont;
/*:351*//*352:*/
#line 7022 "hint.w"

typedef struct
{int dummy;
}FTglyph;
/*:352*/
#line 5705 "hint.w"


typedef struct font_s{
unsigned char n;
unsigned char*font_data;
int data_size;
double s;
double hpxs,vpxs;
/*313:*/
#line 5799 "hint.w"

struct gcache_s**g0;
struct gcache_s***g1;
struct gcache_s****g2;
struct gcache_s*****g3;

/*:313*/
#line 5713 "hint.w"

FontFormat ff;
union{PKfont pk;FTfont tt;};
}Font;
/*:307*//*318:*/
#line 6011 "hint.w"

/*340:*/
#line 6697 "hint.w"

typedef struct
{unsigned char flag;
unsigned char*encoding;
}PKglyph;
/*:340*//*342:*/
#line 6724 "hint.w"

typedef struct{
int j;
int r;
int f;
unsigned char*data;
}PKparse;
/*:342*/
#line 6012 "hint.w"


struct gcache_s{
int w,h;
int hoff,voff;
unsigned char*bits;
unsigned int GLtexture;
FontFormat ff;
union{
PKglyph pk;
FTglyph tt;
};
};
typedef struct gcache_s Gcache;
/*:318*/
#line 7607 "hint.w"



#endif
/*:375*/
