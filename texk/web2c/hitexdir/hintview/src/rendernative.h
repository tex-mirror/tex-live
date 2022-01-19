/*376:*/
#line 7624 "hint.w"

#ifndef _RENDERNATIVE_H
#define _RENDERNATIVE_H

/*328:*/
#line 6556 "hint.w"

extern void nativeInit(void);
extern void nativeClear(void);
/*:328*//*329:*/
#line 6563 "hint.w"

extern void nativeSetSize(int px_h,int px_v,double xdpi,double ydpi);
/*:329*//*330:*/
#line 6568 "hint.w"

extern void nativeSetDark(int dark);
/*:330*//*331:*/
#line 6573 "hint.w"

extern void nativeBlank(void);
/*:331*//*332:*/
#line 6583 "hint.w"

typedef struct gcache_s*gcache_s_ptr;

extern void nativeGlyph(double x,double dx,double y,double dy,double w,double h,struct gcache_s*g,uint8_t s);
/*:332*//*333:*/
#line 6594 "hint.w"

void nativeRule(double x,double y,double w,double h);
/*:333*//*334:*/
#line 6600 "hint.w"

void nativeImage(double x,double y,double w,double h,unsigned char*istart,unsigned char*iend);
/*:334*//*335:*/
#line 6606 "hint.w"

extern void nativeSetPK(struct gcache_s*g);
extern void nativeSetFreeType(struct gcache_s*g);
/*:335*//*336:*/
#line 6613 "hint.w"

void nativeFreeGlyph(struct gcache_s*g);
/*:336*/
#line 7628 "hint.w"


#endif
/*:376*/
