/*****
 * bezierpatch.h
 * Authors: John C. Bowman and Jesse Frohlich
 *
 * Render Bezier patches and triangles.
 *****/

#ifndef BEZIERPATCH_H
#define BEZIERPATCH_H

#include "drawelement.h"

namespace camp {

#ifdef HAVE_GL

extern const double Fuzz4;

class vertexData {
public:
  GLfloat position[3];
  GLfloat normal[3];
  GLint  material;
  vertexData() {};
  vertexData(const triple& v, const triple& n) {
    position[0]=v.getx();
    position[1]=v.gety();
    position[2]=v.getz();
    normal[0]=n.getx();
    normal[1]=n.gety();
    normal[2]=n.getz();
    material=drawElement::materialIndex;
  }
};

class VertexData {
public:
  GLfloat position[3];
  GLfloat normal[3];
  GLuint color;
  GLint  material;
  VertexData() {};
  VertexData(const triple& v, const triple& n) {
    position[0]=v.getx();
    position[1]=v.gety();
    position[2]=v.getz();
    normal[0]=n.getx();
    normal[1]=n.gety();
    normal[2]=n.getz();
    color=0;
    material=drawElement::materialIndex;
  }
  VertexData(const triple& v, const triple& n, GLfloat *c) {
    position[0]=v.getx();
    position[1]=v.gety();
    position[2]=v.getz();
    normal[0]=n.getx();
    normal[1]=n.gety();
    normal[2]=n.getz();
    color=glm::packUnorm4x8(glm::vec4(c[0],c[1],c[2],c[3]));
    material=-(int) drawElement::materialIndex-1; // request explicit color
  }

};

struct BezierPatch
{
  static std::vector<vertexData> vertexbuffer;
  static std::vector<VertexData> Vertexbuffer;
  static std::vector<VertexData> tVertexbuffer; // for transparent surfaces
  static std::vector<GLuint> indices;
  static std::vector<GLuint> Indices;
  static std::vector<GLuint> tIndices;
  static GLuint nvertices;
  static GLuint Nvertices;
  static GLuint Ntvertices;
  
  static GLuint vertsBufferIndex; 
  static GLuint VertsBufferIndex; 
  static GLuint tVertsBufferIndex; 
  
  static GLuint elemBufferIndex; 
  static GLuint ElemBufferIndex; 
  static GLuint tElemBufferIndex; 
  
  std::vector<GLuint> *pindices;
  triple u,v,w;
  double epsilon;
  double Epsilon;
  double res2;
  double Res2; // Reduced resolution for Bezier triangles flatness test.
  triple Min,Max;
  typedef GLuint vertexFunction(const triple &v, const triple& n);
  typedef GLuint VertexFunction(const triple &v, const triple& n, GLfloat *c);
  vertexFunction *pvertex;
  VertexFunction *pVertex;
  
  BezierPatch() {}
  
  void init(double res, const triple& Min, const triple& Max,
            bool transparent, GLfloat *colors=NULL);
    
// Store the vertex v and its normal vector n in the buffer.
  static GLuint vertex(const triple &v, const triple& n) {
    vertexbuffer.push_back(vertexData(v,n));
    return nvertices++;
  }
  
// Store the vertex v and its normal vector n and colour c in the buffer.
  static GLuint Vertex(const triple& v, const triple& n, GLfloat *c) {
    Vertexbuffer.push_back(VertexData(v,n,c));
    return Nvertices++;
  }
  
  static GLuint tvertex(const triple &v, const triple& n) {
    tVertexbuffer.push_back(VertexData(v,n));
    return Ntvertices++;
  }
  
  static GLuint tVertex(const triple& v, const triple& n, GLfloat *c) {
    tVertexbuffer.push_back(VertexData(v,n,c));
    return Ntvertices++;
  }
  
  triple normal(triple left3, triple left2, triple left1, triple middle,
                triple right1, triple right2, triple right3) {
    triple rp=right1-middle;
    triple lp=left1-middle;
    triple n=triple(rp.gety()*lp.getz()-rp.getz()*lp.gety(),
                    rp.getz()*lp.getx()-rp.getx()*lp.getz(),
                    rp.getx()*lp.gety()-rp.gety()*lp.getx());
    if(abs2(n) > epsilon)
      return unit(n);
    
    triple lpp=bezierPP(middle,left1,left2);
    triple rpp=bezierPP(middle,right1,right2);
    n=cross(rpp,lp)+cross(rp,lpp);
    if(abs2(n) > epsilon)
      return unit(n);
    
    triple lppp=bezierPPP(middle,left1,left2,left3);
    triple rppp=bezierPPP(middle,right1,right2,right3);
    
    return unit(9.0*cross(rpp,lpp)+
                3.0*(cross(rp,lppp)+cross(rppp,lp)+
                     cross(rppp,lpp)+cross(rpp,lppp))+
                cross(rppp,lppp));
  }

  triple derivative(triple p0, triple p1, triple p2, triple p3) {
    triple lp=p1-p0;
    if(abs2(lp) > epsilon)
      return lp;
    
    triple lpp=bezierPP(p0,p1,p2);
    if(abs2(lpp) > epsilon)
      return lpp;
    
    return bezierPPP(p0,p1,p2,p3);
  }

  virtual double Distance(const triple *p) {
    triple p0=p[0];
    triple p3=p[3];
    triple p12=p[12];
    triple p15=p[15];
    
    // Check the flatness of the quad.
    double d=Distance2(p15,p0,normal(p3,p[2],p[1],p0,p[4],p[8],p12));
    
    // Determine how straight the edges are.
    d=max(d,Straightness(p0,p[1],p[2],p3));
    d=max(d,Straightness(p0,p[4],p[8],p12));
    d=max(d,Straightness(p3,p[7],p[11],p15));
    d=max(d,Straightness(p12,p[13],p[14],p15));
    
    // Determine how straight the interior control curves are.
    d=max(d,Straightness(p[4],p[5],p[6],p[7]));
    d=max(d,Straightness(p[8],p[9],p[10],p[11]));
    d=max(d,Straightness(p[1],p[5],p[9],p[13]));
    return max(d,Straightness(p[2],p[6],p[10],p[14]));
  }
  
  struct Split3 {
    triple m0,m2,m3,m4,m5;
    Split3() {}
    Split3(triple z0, triple c0, triple c1, triple z1) {
      m0=0.5*(z0+c0);
      triple m1=0.5*(c0+c1);
      m2=0.5*(c1+z1);
      m3=0.5*(m0+m1);
      m4=0.5*(m1+m2);
      m5=0.5*(m3+m4);
    }
  };
  
// Approximate bounds by bounding box of control polyhedron.
  bool offscreen(size_t n, const triple *v) {
    double x,y,z;
    double X,Y,Z;
    
    boundstriples(x,y,z,X,Y,Z,n,v);
    return
      X < Min.getx() || x > Max.getx() ||
      Y < Min.gety() || y > Max.gety() ||
      Z < Min.getz() || z > Max.getz();
  }

  static void clear() {
    nvertices=0;
    vertexbuffer.clear();
    indices.clear();
  }
  
  static void Clear() {
    Nvertices=0;
    Vertexbuffer.clear();
    Indices.clear();
  }
  
  static void tClear() {
    Ntvertices=0;
    tVertexbuffer.clear();
    tIndices.clear();
  }
  
  ~BezierPatch() {}
  
  void render(const triple *p,
              GLuint I0, GLuint I1, GLuint I2, GLuint I3,
              triple P0, triple P1, triple P2, triple P3,
              bool flat0, bool flat1, bool flat2, bool flat3,
              GLfloat *C0=NULL, GLfloat *C1=NULL, GLfloat *C2=NULL,
              GLfloat *C3=NULL);
  virtual void render(const triple *p, bool straight, GLfloat *c0=NULL);
  
  void queue(const triple *g, bool straight, double ratio,
             const triple& Min, const triple& Max, bool transparent,
             GLfloat *colors=NULL) {
    init(pixel*ratio,Min,Max,transparent,colors);
    render(g,straight,colors);
  }
  
  void drawMaterials();
  void drawColors(GLuint& Nvertices,
                  std::vector<VertexData>& Vertexbuffer,
                  std::vector<GLuint>& Indices);
  void sortTriangles();
  
  void drawColors() {
    drawColors(Nvertices,Vertexbuffer,Indices);
  }
    
  void drawOpaque() {
    drawMaterials();
    drawColors();
  }
    
  void drawTransparent() {
    glDepthMask(GL_FALSE);
    sortTriangles();
    drawColors(Ntvertices,tVertexbuffer,tIndices);
    glDepthMask(GL_TRUE);
  }
  
  void draw() {
    drawOpaque();
    drawTransparent();
  }
  
};

struct BezierTriangle : public BezierPatch {
public:
  BezierTriangle() : BezierPatch() {}
  
  double Distance(const triple *p) {
    triple p0=p[0];
    triple p6=p[6];
    triple p9=p[9];
    
    // Check how far the internal point is from the centroid of the vertices.
    double d=abs2((p0+p6+p9)*third-p[4]);

    // Determine how straight the edges are.
    d=max(d,Straightness(p0,p[1],p[3],p6));
    d=max(d,Straightness(p0,p[2],p[5],p9));
    return max(d,Straightness(p6,p[7],p[8],p9));
  }
  
  void render(const triple *p,
              GLuint I0, GLuint I1, GLuint I2,
              triple P0, triple P1, triple P2,
              bool flat0, bool flat1, bool flat2,
              GLfloat *C0=NULL, GLfloat *C1=NULL, GLfloat *C2=NULL);
  void render(const triple *p, bool straight, GLfloat *c0=NULL);
};


struct Triangles : public BezierPatch {
public:
  Triangles() : BezierPatch() {}

  void queue(size_t nP, triple* P, size_t nN, triple* N,
             size_t nC, prc::RGBAColour* C, size_t nI,
             uint32_t (*PI)[3], uint32_t (*NI)[3], uint32_t (*CI)[3],
             bool transparent);
};


#endif

} //namespace camp

#endif
