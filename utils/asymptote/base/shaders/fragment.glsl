struct Material
{
  vec4 diffuse,emissive,specular;
  vec4 parameters;
};

struct Light
{
  vec3 direction;
  vec3 color;
};

uniform uint nlights;
uniform Light lights[max(Nlights,1)];

uniform MaterialBuffer {
  Material Materials[Nmaterials];
};

flat in int materialIndex;
out vec4 outColor;

// PBR material parameters
vec3 Diffuse; // Diffuse for nonmetals, reflectance for metals.
vec3 Specular; // Specular tint for nonmetals
float Metallic; // Metallic/Nonmetals parameter
float Fresnel0; // Fresnel at zero for nonmetals
float Roughness2; // roughness squared, for smoothing
float Roughness;

#ifdef HAVE_SSBO
struct Fragment
{
  vec4 color;
  float depth;
};

layout(binding=1, std430) buffer offsetBuffer {
  uint offset[];
};

layout(binding=2, std430) buffer countBuffer {
  uint count[];
};

layout(binding=3, std430) buffer fragmentBuffer {
  Fragment fragment[];
};

uniform uint width;
#endif

#ifdef NORMAL

#ifndef ORTHOGRAPHIC
in vec3 ViewPosition;
#endif
in vec3 Normal;
vec3 normal;

#ifdef USE_IBL
uniform sampler2D reflBRDFSampler;
uniform sampler2D diffuseSampler;
uniform sampler3D reflImgSampler;

const float pi=acos(-1.0);
const float piInv=1.0/pi;
const float twopi=2.0*pi;
const float twopiInv=1.0/twopi;

// (x,y,z) -> (r,theta,phi);
// theta -> [0,pi]: colatitude
// phi -> [-pi,pi]: longitude
vec3 cart2sphere(vec3 cart)
{
  float x=cart.x;
  float y=cart.z;
  float z=cart.y;

  float r=length(cart);
  float theta=r > 0.0 ? acos(z/r) : 0.0;
  float phi=atan(y,x);

  return vec3(r,theta,phi);
}

vec2 normalizedAngle(vec3 cartVec)
{
  vec3 sphericalVec=cart2sphere(cartVec);
  sphericalVec.y=sphericalVec.y*piInv;
  sphericalVec.z=0.75-sphericalVec.z*twopiInv;

  return sphericalVec.zy;
}

vec3 IBLColor(vec3 viewDir)
{
  //
  // based on the split sum formula approximation
  // L(v)=\int_\Omega L(l)f(l,v) \cos \theta_l
  // which, by the split sum approiximation (assuming independence+GGX distrubition),
  // roughly equals (within a margin of error)
  // [\int_\Omega L(l)] * [\int_\Omega f(l,v) \cos \theta_l].
  // the first term is the reflectance irradiance integral

  vec3 IBLDiffuse=Diffuse*texture(diffuseSampler,normalizedAngle(normal)).rgb;
  vec3 reflectVec=normalize(reflect(-viewDir,normal));
  vec2 reflCoord=normalizedAngle(reflectVec);
  vec3 IBLRefl=texture(reflImgSampler,vec3(reflCoord,Roughness)).rgb;
  vec2 IBLbrdf=texture(reflBRDFSampler,vec2(dot(normal,viewDir),Roughness)).rg;
  float specularMultiplier=Fresnel0*IBLbrdf.x+IBLbrdf.y;
  vec3 dielectric=IBLDiffuse+specularMultiplier*IBLRefl;
  vec3 metal=Diffuse*IBLRefl;
  return mix(dielectric,metal,Metallic);
}
#else
// h is the halfway vector between normal and light direction
// GGX Trowbridge-Reitz Approximation
float NDF_TRG(vec3 h)
{
  float ndoth=max(dot(normal,h),0.0);
  float alpha2=Roughness2*Roughness2;
  float denom=ndoth*ndoth*(alpha2-1.0)+1.0;
  return denom != 0.0 ? alpha2/(denom*denom) : 0.0;
}

float GGX_Geom(vec3 v)
{
  float ndotv=max(dot(v,normal),0.0);
  float ap=1.0+Roughness2;
  float k=0.125*ap*ap;
  return ndotv/((ndotv*(1.0-k))+k);
}

float Geom(vec3 v, vec3 l)
{
  return GGX_Geom(v)*GGX_Geom(l);
}

// Schlick's approximation
float Fresnel(vec3 h, vec3 v, float fresnel0)
{
  float a=1.0-max(dot(h,v),0.0);
  float b=a*a;
  return fresnel0+(1.0-fresnel0)*b*b*a;
}

vec3 BRDF(vec3 viewDirection, vec3 lightDirection)
{
  vec3 lambertian=Diffuse;
  // Cook-Torrance model
  vec3 h=normalize(lightDirection+viewDirection);

  float omegain=max(dot(viewDirection,normal),0.0);
  float omegaln=max(dot(lightDirection,normal),0.0);

  float D=NDF_TRG(h);
  float G=Geom(viewDirection,lightDirection);
  float F=Fresnel(h,viewDirection,Fresnel0);

  float denom=4.0*omegain*omegaln;
  float rawReflectance=denom > 0.0 ? (D*G)/denom : 0.0;

  vec3 dielectric=mix(lambertian,rawReflectance*Specular,F);
  vec3 metal=rawReflectance*Diffuse;

  return mix(dielectric,metal,Metallic);
}
#endif

#endif

#ifdef COLOR
in vec4 Color;
#endif

void main()
{
  vec4 diffuse;
  vec4 emissive;

  Material m;
#ifdef GENERAL
  m=Materials[abs(materialIndex)-1];
  emissive=m.emissive;
  if(materialIndex >= 0)
    diffuse=m.diffuse;
  else {
    diffuse=Color;
#if Nlights == 0
    emissive += Color;
#endif
  }
#else
  m=Materials[int(materialIndex)];
  emissive=m.emissive;
#ifdef COLOR
  diffuse=Color;
#if Nlights == 0
  emissive += Color;
#endif
#else
  diffuse=m.diffuse;
#endif
#endif

#if defined(NORMAL) && Nlights > 0
  Specular=m.specular.rgb;
  vec4 parameters=m.parameters;
  Roughness=1.0-parameters[0];
  Roughness2=Roughness*Roughness;
  Metallic=parameters[1];
  Fresnel0=parameters[2];
  Diffuse=diffuse.rgb;

  // Given a point x and direction \omega,
  // L_i=\int_{\Omega}f(x,\omega_i,\omega) L(x,\omega_i)(\hat{n}\cdot \omega_i)
  // d\omega_i, where \Omega is the hemisphere covering a point,
  // f is the BRDF function, L is the radiance from a given angle and position.

  normal=normalize(Normal);
  normal=gl_FrontFacing ? normal : -normal;
#ifdef ORTHOGRAPHIC
  vec3 viewDir=vec3(0.0,0.0,1.0);
#else
  vec3 viewDir=-normalize(ViewPosition);
#endif
  vec3 color;
#ifdef USE_IBL
  color=IBLColor(viewDir);
#else
  // For a finite point light, the rendering equation simplifies.
  color=emissive.rgb;
  for(uint i=0u; i < nlights; ++i) {
    Light Li=lights[i];
    vec3 L=Li.direction;
    float cosTheta=max(dot(normal,L),0.0); // $\omega_i \cdot n$ term
    vec3 radiance=cosTheta*Li.color;
    color += BRDF(viewDir,L)*radiance;
  }
#endif
  outColor=vec4(color,diffuse.a);
#else
  outColor=emissive;
#endif

#ifdef HAVE_SSBO
  uint headIndex=uint(gl_FragCoord.y)*width+uint(gl_FragCoord.x);
  uint listIndex=offset[headIndex]+atomicAdd(count[headIndex],1u);
  fragment[listIndex].color=outColor;
  fragment[listIndex].depth=gl_FragCoord.z;
#ifdef TRANSPARENT
#ifndef WIREFRAME
  discard;
#endif
#endif
#endif
}
