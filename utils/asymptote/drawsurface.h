/*****
 * drawsurface.h
 *
 * Stores a surface that has been added to a picture.
 *****/

#ifndef DRAWSURFACE_H
#define DRAWSURFACE_H

#include "drawelement.h"
#include "arrayop.h"
#include "path3.h"
#include "beziercurve.h"
#include "bezierpatch.h"

namespace camp {

#ifdef HAVE_GL
void storecolor(GLfloat *colors, int i, const vm::array &pens, int j);
#endif  

class drawSurface : public drawElement {
protected:
  triple *controls;
  size_t ncontrols;
  triple center;
  bool straight; // True iff Bezier patch is planar and has straight edges.
  prc::RGBAColour diffuse;
  prc::RGBAColour emissive;
  prc::RGBAColour specular;
  prc::RGBAColour *colors;
  double opacity;
  double shininess;
  double metallic;
  double fresnel0;
  double PRCshininess;
  bool invisible;
  Interaction interaction;
  
  triple Min,Max;
  bool prc;
  
public:
#ifdef HAVE_GL
  static BezierCurve C;
#endif  
  
  string wrongsize() {
    return (ncontrols == 16 ? "4x4" : "triangular")+
      string(" array of triples and array of 4 pens required");
  }
  
  drawSurface(const vm::array& g, size_t ncontrols, triple center,
              bool straight, const vm::array&p, double opacity,
              double shininess, double metallic, double fresnel0, double PRCshininess, const vm::array &pens,
              Interaction interaction, bool prc, const string& key="") :
    drawElement(key), ncontrols(ncontrols), center(center), straight(straight),
    opacity(opacity), shininess(shininess), metallic(metallic), fresnel0(fresnel0), PRCshininess(PRCshininess),
    interaction(interaction), prc(prc) {
    if(checkArray(&g) != 4 || checkArray(&p) != 3)
      reportError(wrongsize());
    
    size_t k=0;
    controls=new(UseGC) triple[ncontrols];
    for(unsigned int i=0; i < 4; ++i) {
      vm::array *gi=vm::read<vm::array*>(g,i);
      size_t n=(ncontrols == 16 ? 4 : i+1);
      if(checkArray(gi) != n)
        reportError(wrongsize());
      for(unsigned int j=0; j < n; ++j)
        controls[k++]=vm::read<triple>(gi,j);
    }
    
    pen surfacepen=vm::read<camp::pen>(p,0);
    invisible=surfacepen.invisible();
    
    diffuse=rgba(surfacepen);
    emissive=rgba(vm::read<camp::pen>(p,1));
    specular=rgba(vm::read<camp::pen>(p,2));
    
    size_t nodes=(ncontrols == 16 ? 4 : 3);
    size_t size=checkArray(&pens);
    if(size > 0) {
      if(size != nodes) reportError("4 vertex pens required");
      colors=new(UseGC) prc::RGBAColour[nodes];
      for(size_t i=0; i < nodes; ++i)
      colors[i]=rgba(vm::read<camp::pen>(pens,i));
    } else colors=NULL;
  }
  
  drawSurface(const double* t, const drawSurface *s) :
    drawElement(s->KEY), ncontrols(s->ncontrols), straight(s->straight),
    diffuse(s->diffuse), emissive(s->emissive),
    specular(s->specular), colors(s->colors), opacity(s->opacity),
    shininess(s->shininess),  metallic(s->metallic), fresnel0(s->fresnel0),
    PRCshininess(s->PRCshininess), invisible(s->invisible),
    interaction(s->interaction), prc(s->prc) { 
    
    if(s->controls) {
      controls=new(UseGC) triple[ncontrols];
      for(unsigned int i=0; i < ncontrols; ++i)
        controls[i]=t*s->controls[i];
    } else controls=NULL;
  
#ifdef HAVE_GL
    center=t*s->center;
#endif    
  }
  
  virtual ~drawSurface() {}

  bool is3D() {return true;}
};
  
class drawBezierPatch : public drawSurface {
public:  
#ifdef HAVE_GL
  static BezierPatch S;
#endif  
  
  drawBezierPatch(const vm::array& g, triple center, bool straight,
              const vm::array&p, double opacity, double shininess,
              double metallic, double fresnel0,
              double PRCshininess, const vm::array &pens,
              Interaction interaction, bool prc) : 
    drawSurface(g,16,center,straight,p,opacity,
                shininess,metallic,fresnel0,PRCshininess,pens,interaction,prc) {}

  drawBezierPatch(const double* t, const drawBezierPatch *s) :
    drawSurface(t,s) {
  }
  
  void bounds(const double* t, bbox3& b);
  
  void ratio(const double* t, pair &b, double (*m)(double, double),
             double fuzz, bool &first);
  
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  
  void render(double, const triple& Min, const triple& Max,
              double perspective, bool transparent);
  drawElement *transformed(const double* t);
};
  
class drawBezierTriangle : public drawSurface {
public:
#ifdef HAVE_GL
  static BezierTriangle S;
#endif  
  
  drawBezierTriangle(const vm::array& g, triple center, bool straight,
              const vm::array&p, double opacity, double shininess,
              double metallic, double fresnel0,
              double PRCshininess, const vm::array &pens,
              Interaction interaction, bool prc) : 
    drawSurface(g,10,center,straight,p,opacity,shininess,metallic,fresnel0,
                PRCshininess,pens,interaction,prc) {}
  
  drawBezierTriangle(const double* t, const drawBezierTriangle *s) :
    drawSurface(t,s) {
  }
  
  void bounds(const double* t, bbox3& b);
  
  void ratio(const double* t, pair &b, double (*m)(double, double),
             double fuzz, bool &first);
  
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  
  void render(double, const triple& Min, const triple& Max,
              double perspective, bool transparent);
  drawElement *transformed(const double* t);
};
  
class drawNurbs : public drawElement {
protected:
  size_t udegree,vdegree;
  size_t nu,nv;
  triple *controls;
  double *weights;
  double *uknots, *vknots;
  prc::RGBAColour diffuse;
  prc::RGBAColour emissive;
  prc::RGBAColour specular;
  double opacity;
  double shininess;
  double metallic;
  double fresnel0;
  double PRCshininess;
  triple normal;
  bool invisible;
  
  triple Min,Max;
  
#ifdef HAVE_GL
  GLfloat *colors;
  GLfloat *Controls;
  GLfloat *uKnots;
  GLfloat *vKnots;
#endif  
  
public:
  drawNurbs(const vm::array& g, const vm::array* uknot, const vm::array* vknot,
            const vm::array* weight, const vm::array&p, double opacity,
            double shininess, double metallic, double fresnel0, double PRCshininess, const vm::array &pens,
            const string& key="") 
    : drawElement(key), opacity(opacity), shininess(shininess),
    metallic(metallic), fresnel0(fresnel0), PRCshininess(PRCshininess) {
    size_t weightsize=checkArray(weight);
    
    const string wrongsize="Inconsistent NURBS data";
    nu=checkArray(&g);
    
    if(nu == 0 || (weightsize != 0 && weightsize != nu) || checkArray(&p) != 3)
      reportError(wrongsize);
    
    vm::array *g0=vm::read<vm::array*>(g,0);
    nv=checkArray(g0);
    
    size_t n=nu*nv;
    controls=new(UseGC) triple[n];
    
    size_t k=0;
    for(size_t i=0; i < nu; ++i) {
      vm::array *gi=vm::read<vm::array*>(g,i);
      if(checkArray(gi) != nv)  
        reportError(wrongsize);
      for(size_t j=0; j < nv; ++j)
        controls[k++]=vm::read<triple>(gi,j);
    }
      
    if(weightsize > 0) {
      size_t k=0;
      weights=new(UseGC) double[n];
      for(size_t i=0; i < nu; ++i) {
        vm::array *weighti=vm::read<vm::array*>(weight,i);
        if(checkArray(weighti) != nv)  
          reportError(wrongsize);
        for(size_t j=0; j < nv; ++j)
          weights[k++]=vm::read<double>(weighti,j);
      }
    } else weights=NULL;
      
    size_t nuknots=checkArray(uknot);
    size_t nvknots=checkArray(vknot);
    
    if(nuknots <= nu+1 || nuknots > 2*nu || nvknots <= nv+1 || nvknots > 2*nv)
      reportError(wrongsize);

    udegree=nuknots-nu-1;
    vdegree=nvknots-nv-1;
    
    run::copyArrayC(uknots,uknot,0,UseGC);
    run::copyArrayC(vknots,vknot,0,UseGC);
    
    pen surfacepen=vm::read<camp::pen>(p,0);
    invisible=surfacepen.invisible();
    
    diffuse=rgba(surfacepen);
    emissive=rgba(vm::read<camp::pen>(p,1));
    specular=rgba(vm::read<camp::pen>(p,2));
    
#ifdef HAVE_GL
    Controls=NULL;
    int size=checkArray(&pens);
    if(size > 0) {
      colors=new(UseGC) GLfloat[16];
      if(size != 4) reportError(wrongsize);
      storecolor(colors,0,pens,0);
      storecolor(colors,8,pens,1);
      storecolor(colors,12,pens,2);
      storecolor(colors,4,pens,3);
    } else colors=NULL;
#endif  
  }
  
  drawNurbs(const double* t, const drawNurbs *s) :
    drawElement(s->KEY), udegree(s->udegree), vdegree(s->vdegree), nu(s->nu),
    nv(s->nv), weights(s->weights), uknots(s->uknots), vknots(s->vknots),
    diffuse(s->diffuse),
    emissive(s->emissive), specular(s->specular), opacity(s->opacity),
    shininess(s->shininess), PRCshininess(s->PRCshininess), 
    invisible(s->invisible) {
    
    const size_t n=nu*nv;
    controls=new(UseGC) triple[n];
    for(unsigned int i=0; i < n; ++i)
      controls[i]=t*s->controls[i];
    
#ifdef HAVE_GL
    Controls=NULL;
    colors=s->colors;
#endif    
  }
  
  bool is3D() {return true;}
  
  void bounds(const double* t, bbox3& b);
  
  virtual ~drawNurbs() {}

  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  
  void displacement();
  void ratio(const double* t, pair &b, double (*m)(double, double), double,
             bool &first);

  void render(double size2, const triple& Min, const triple& Max,
              double perspective, bool transparent);
    
  drawElement *transformed(const double* t);
};
  
// Draw a transformed PRC object.
class drawPRC : public drawElementLC {
protected:
  prc::RGBAColour diffuse;
  prc::RGBAColour emissive;
  prc::RGBAColour specular;
  double opacity;
  double shininess;
  bool invisible;
public:
  drawPRC(const vm::array& t, const vm::array&p, double opacity,
          double shininess) : 
    drawElementLC(t), opacity(opacity), shininess(shininess) {

    string needthreepens="array of 3 pens required";
    if(checkArray(&p) != 3)
      reportError(needthreepens);
    
    pen surfacepen=vm::read<camp::pen>(p,0);
    invisible=surfacepen.invisible();
    
    diffuse=rgba(surfacepen);
    emissive=rgba(vm::read<camp::pen>(p,1));
    specular=rgba(vm::read<camp::pen>(p,2));
  }
  
  drawPRC(const double* t, const drawPRC *s) :
    drawElementLC(t,s), diffuse(s->diffuse),
    emissive(s->emissive), specular(s->specular), opacity(s->opacity),
    shininess(s->shininess), invisible(s->invisible) {
  }
  
  bool write(prcfile *out, unsigned int *, double, groupsmap&) {
    return true;
  }
  virtual void transformedbounds(const double*, bbox3&) {}
  virtual void transformedratio(const double*, pair&,
                                double (*)(double, double), double, bool&) {}

};
  
// Draw a PRC unit sphere.
class drawSphere : public drawPRC {
  bool half;
  int type;
public:
  drawSphere(const vm::array& t, bool half, const vm::array&p, double opacity,
             double shininess, int type) :
    drawPRC(t,p,opacity,shininess), half(half), type(type) {}

  drawSphere(const double* t, const drawSphere *s) :
    drawElement(s->KEY), drawPRC(t,s), half(s->half), type(s->type) {}
    
  void P(triple& t, double x, double y, double z);
  
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  
  drawElement *transformed(const double* t) {
    return new drawSphere(t,this);
  }
};
  
// Draw a PRC unit cylinder.
class drawCylinder : public drawPRC {
public:
  drawCylinder(const vm::array& t, const vm::array&p, double opacity,
               double shininess) :
    drawPRC(t,p,opacity,shininess) {}

  drawCylinder(const double* t, const drawCylinder *s) :
    drawPRC(t,s) {}
    
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  
  drawElement *transformed(const double* t) {
    return new drawCylinder(t,this);
  }
};
  
// Draw a PRC unit disk.
class drawDisk : public drawPRC {
public:
  drawDisk(const vm::array& t, const vm::array&p, double opacity,
           double shininess) :
    drawPRC(t,p,opacity,shininess) {}

  drawDisk(const double* t, const drawDisk *s) :
    drawPRC(t,s) {}
    
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  
  drawElement *transformed(const double* t) {
    return new drawDisk(t,this);
  }
};
  
// Draw a PRC tube.
class drawTube : public drawElement {
protected:
  path3 center;
  path3 g;
  prc::RGBAColour diffuse;
  prc::RGBAColour emissive;
  prc::RGBAColour specular;
  double opacity;
  double shininess;
  bool invisible;
public:
  drawTube(path3 center, path3 g, const vm::array&p, double opacity,
           double shininess) : 
    center(center), g(g), opacity(opacity), shininess(shininess) {
    string needthreepens="array of 3 pens required";
    if(checkArray(&p) != 3)
      reportError(needthreepens);
    
    pen surfacepen=vm::read<camp::pen>(p,0);
    invisible=surfacepen.invisible();
    
    diffuse=rgba(surfacepen);
    emissive=rgba(vm::read<camp::pen>(p,1));
    specular=rgba(vm::read<camp::pen>(p,2));
  }
  
  drawTube(const double* t, const drawTube *s) :
    drawElement(s->KEY), center(camp::transformed(t,s->center)),
    g(camp::transformed(t,s->g)), 
    diffuse(s->diffuse), emissive(s->emissive),
    specular(s->specular), opacity(s->opacity),
    shininess(s->shininess), invisible(s->invisible) {
  }
  
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
                        
  drawElement *transformed(const double* t) {
    return new drawTube(t,this);
  }
};

  
class drawBaseTriangles : public drawElement {
protected:
#ifdef HAVE_GL
  Triangles R;
#endif  
  
  size_t nP;
  triple* P;
  size_t nN;
  triple* N;
  size_t nI;
  uint32_t (*PI)[3];
  uint32_t (*NI)[3];
  
  triple Min,Max;

  static const string wrongsize;
  static const string outofrange;
    
public:
  drawBaseTriangles(const vm::array& v, const vm::array& vi,
                    const vm::array& n, const vm::array& ni) {
    nP=checkArray(&v);
    P=new(UseGC) triple[nP];
    for(size_t i=0; i < nP; ++i)
      P[i]=vm::read<triple>(v,i);
  
    nI=checkArray(&vi);
    PI=new(UseGC) uint32_t[nI][3];
    for(size_t i=0; i < nI; ++i) {
      vm::array *vii=vm::read<vm::array*>(vi,i);
      if(checkArray(vii) != 3) reportError(wrongsize);
      uint32_t *PIi=PI[i];
      for(size_t j=0; j < 3; ++j) {
        size_t index=unsignedcast(vm::read<Int>(vii,j));
        if(index >= nP) reportError(outofrange);
        PIi[j]=index;
      }
    }
    
    nN=checkArray(&n);
    if(nN) {
      N=new(UseGC) triple[nN];
      for(size_t i=0; i < nN; ++i)
        N[i]=vm::read<triple>(n,i);
    
      if(checkArray(&ni) != nI)
        reportError("Index arrays have different lengths");
      NI=new(UseGC) uint32_t[nI][3];
      for(size_t i=0; i < nI; ++i) {
        vm::array *nii=vm::read<vm::array*>(ni,i);
        if(checkArray(nii) != 3) reportError(wrongsize);
        uint32_t *NIi=NI[i];
        for(size_t j=0; j < 3; ++j) {
          size_t index=unsignedcast(vm::read<Int>(nii,j));
          if(index >= nN) reportError(outofrange);
          NIi[j]=index;
        }
      }
    }
  }

  drawBaseTriangles(const double* t, const drawBaseTriangles *s) :
    drawElement(s->KEY), nP(s->nP), nN(s->nN), nI(s->nI) {
    P=new(UseGC) triple[nP];
    for(size_t i=0; i < nP; i++)
      P[i]=t*s->P[i];
    
    PI=new(UseGC) uint32_t[nI][3];
    for(size_t i=0; i < nI; ++i) {
      uint32_t *PIi=PI[i];
      uint32_t *sPIi=s->PI[i];
      for(size_t j=0; j < 3; ++j)
        PIi[j]=sPIi[j];
    }

    if(nN) {
      N=new(UseGC) triple[nN];
      for(size_t i=0; i < nN; i++)
        N[i]=transformNormal(t,s->N[i]);
    
      NI=new(UseGC) uint32_t[nI][3];
      for(size_t i=0; i < nI; ++i) {
        uint32_t *NIi=NI[i];
        uint32_t *sNIi=s->NI[i];
        for(size_t j=0; j < 3; ++j)
          NIi[j]=sNIi[j];
      }
    }
  }
    
  bool is3D() {return true;}
  
  void bounds(const double* t, bbox3& b);
  
  void ratio(const double* t, pair &b, double (*m)(double, double),
             double fuzz, bool &first);
  
  virtual ~drawBaseTriangles() {}
  
  drawElement *transformed(const double* t) {
    return new drawBaseTriangles(t,this);
  }
};
  
class drawTriangles : public drawBaseTriangles {
  size_t nC;
  prc::RGBAColour*C;
  uint32_t (*CI)[3];
   
  // Asymptote material data
  prc::RGBAColour diffuse;
  prc::RGBAColour emissive;
  prc::RGBAColour specular;
  double opacity;
  double shininess;
  double metallic;
  double fresnel0;
  double PRCshininess;
  bool invisible;
   
public:
  drawTriangles(const vm::array& v, const vm::array& vi,
                const vm::array& n, const vm::array& ni,
                const vm::array&p, double opacity, double shininess,
                double metallic, double fresnel0,
                double PRCshininess, const vm::array& c, const vm::array& ci) :
    drawBaseTriangles(v,vi,n,ni),
    opacity(opacity),shininess(shininess),metallic(metallic),
    fresnel0(fresnel0),PRCshininess(PRCshininess) {

    const string needthreepens="array of 3 pens required";
    if(checkArray(&p) != 3)
      reportError(needthreepens);
      
    const pen surfacepen=vm::read<camp::pen>(p,0);
    invisible=surfacepen.invisible();
    diffuse=rgba(surfacepen);
    
    nC=checkArray(&c);
    if(nC) {
      C=new(UseGC) prc::RGBAColour[nC];
      for(size_t i=0; i < nC; ++i)
        C[i]=rgba(vm::read<camp::pen>(c,i));
    
      size_t nI=checkArray(&vi);
    
      if(checkArray(&ci) != nI)
        reportError("Index arrays have different lengths");
      CI=new(UseGC) uint32_t[nI][3];
      for(size_t i=0; i < nI; ++i) {
        vm::array *cii=vm::read<vm::array*>(ci,i);
        if(checkArray(cii) != 3) reportError(wrongsize);
        uint32_t *CIi=CI[i];
        for(size_t j=0; j < 3; ++j) {
          size_t index=unsignedcast(vm::read<Int>(cii,j));
          if(index >= nC) reportError(outofrange);
          CIi[j]=index;
        }
      }
    } else {
      emissive=rgba(vm::read<camp::pen>(p,1));
    }
    specular=rgba(vm::read<camp::pen>(p,2));
  }
  
  drawTriangles(const double* t, const drawTriangles *s) :
    drawBaseTriangles(t,s), nC(s->nC),
    diffuse(s->diffuse), emissive(s->emissive),
    specular(s->specular), opacity(s->opacity), shininess(s->shininess), 
    metallic(s->metallic), fresnel0(s->fresnel0), PRCshininess(s->PRCshininess),
    invisible(s->invisible) {
    
    if(nC) {
      C=new(UseGC) prc::RGBAColour[nC];
      for(size_t i=0; i < nC; ++i)
        C[i]=s->C[i];
    
      CI=new(UseGC) uint32_t[nI][3];
      for(size_t i=0; i < nI; ++i) {
        uint32_t *CIi=CI[i];
        uint32_t *sCIi=s->CI[i];
        for(size_t j=0; j < 3; ++j)
          CIi[j]=sCIi[j];
      }
    }
  }
 
  virtual ~drawTriangles() {}
 
  void render(double size2, const triple& Min, const triple& Max,
              double perspective, bool transparent);
 
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
 
  drawElement *transformed(const double* t) {
    return new drawTriangles(t,this);
  }
};

}

#endif
