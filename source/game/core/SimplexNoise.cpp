#include "game/core/SimplexNoise.hpp"
#include "game/util/Random.hpp"
#include <cmath>

static int grad3[12][3]={{1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},{1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},{0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}};
static const double F2=0.36602540378443864676,G2=0.21132486540518711775,F3=0.33333333333333333333,G3=0.16666666666666666667;

SimplexNoise::SimplexNoise():p(NULL),pLength(0),pMask(0),xo(0.0),yo(0.0),zo(0.0){}
SimplexNoise::SimplexNoise(Random* random){init(random);}
SimplexNoise::~SimplexNoise(){delete[] p;}
void SimplexNoise::init(Random* random)
{
 p=new int[512]; pLength=512; pMask=255; xo=random->nextDouble()*256.0; yo=random->nextDouble()*256.0; zo=random->nextDouble()*256.0;
 int i; for(i=0;i<256;++i)p[i]=i;
 for(i=0;i<256;++i){int j=random->nextInt(256-i)+i;int v=p[i];p[i]=p[j];p[j]=v;p[i+256]=p[i];}
}
int SimplexNoise::fastfloor(double v){return v>0.0?(int)v:(int)v-1;}
double SimplexNoise::dot(int* g,double x,double y){return g[0]*x+g[1]*y;}
double SimplexNoise::dot(int* g,double x,double y,double z){return g[0]*x+g[1]*y+g[2]*z;}
double SimplexNoise::getValue(double x,double y)
{
 double s=(x+y)*F2;int i=fastfloor(x+s),j=fastfloor(y+s);double t=(i+j)*G2,x0=x-(i-t),y0=y-(j-t);int i1=x0>y0,j1=x0>y0?0:1;
 double x1=x0-i1+G2,y1=y0-j1+G2,x2=x0-1+2*G2,y2=y0-1+2*G2;int ii=i&255,jj=j&255;
 int g0=p[ii+p[jj]]%12,g1=p[ii+i1+p[jj+j1]]%12,g2=p[ii+1+p[jj+1]]%12;double n0=0,n1=0,n2=0,q;
 q=.5-x0*x0-y0*y0;if(q>0){q*=q;n0=q*q*dot(grad3[g0],x0,y0);}q=.5-x1*x1-y1*y1;if(q>0){q*=q;n1=q*q*dot(grad3[g1],x1,y1);}q=.5-x2*x2-y2*y2;if(q>0){q*=q;n2=q*q*dot(grad3[g2],x2,y2);}return 70*(n0+n1+n2);
}
double SimplexNoise::getValue(double x,double y,double z)
{
 double s=(x+y+z)*F3;int i=fastfloor(x+s),j=fastfloor(y+s),k=fastfloor(z+s);double t=(i+j+k)*G3,x0=x-i+t,y0=y-j+t,z0=z-k+t;int i1,j1,k1,i2,j2,k2;
 if(x0>=y0){if(y0>=z0){i1=1;j1=0;k1=0;i2=1;j2=1;k2=0;}else if(x0>=z0){i1=1;j1=0;k1=0;i2=1;j2=0;k2=1;}else{i1=0;j1=0;k1=1;i2=1;j2=0;k2=1;}}else if(x0<z0){i1=0;j1=0;k1=1;i2=0;j2=1;k2=1;}else if(y0<z0){i1=0;j1=1;k1=0;i2=0;j2=1;k2=1;}else{i1=0;j1=1;k1=0;i2=1;j2=1;k2=0;}
 double x1=x0-i1+G3,y1=y0-j1+G3,z1=z0-k1+G3,x2=x0-i2+2*G3,y2=y0-j2+2*G3,z2=z0-k2+2*G3,x3=x0-1+3*G3,y3=y0-1+3*G3,z3=z0-1+3*G3;int ii=i&255,jj=j&255,kk=k&255;
 int g0=p[ii+p[jj+p[kk]]]%12,g1=p[ii+i1+p[jj+j1+p[kk+k1]]]%12,g2=p[ii+i2+p[jj+j2+p[kk+k2]]]%12,g3=p[ii+1+p[jj+1+p[kk+1]]]%12;double n0=0,n1=0,n2=0,n3=0,q;
 q=.6-x0*x0-y0*y0-z0*z0;if(q>0){q*=q;n0=q*q*dot(grad3[g0],x0,y0,z0);}q=.6-x1*x1-y1*y1-z1*z1;if(q>0){q*=q;n1=q*q*dot(grad3[g1],x1,y1,z1);}q=.6-x2*x2-y2*y2-z2*z2;if(q>0){q*=q;n2=q*q*dot(grad3[g2],x2,y2,z2);}q=.6-x3*x3-y3*y3-z3*z3;if(q>0){q*=q;n3=q*q*dot(grad3[g3],x3,y3,z3);}return 32*(n0+n1+n2+n3);
}
void SimplexNoise::add(arrayWithLength<double> v,double x,double y,int xs,int ys,double sx,double sy,double scale){int n=0;for(int j=0;j<ys;++j)for(int i=0;i<xs;++i)v.data[n++]+=getValue((x+i)*sx+xo,(y+j)*sy+yo)*scale;}
void SimplexNoise::add(arrayWithLength<double> v,double x,double y,double z,int xs,int ys,int zs,double sx,double sy,double sz,double scale){int n=0;for(int k=0;k<zs;++k)for(int j=0;j<ys;++j)for(int i=0;i<xs;++i)v.data[n++]+=getValue((x+i)*sx+xo,(y+j)*sy+yo,(z+k)*sz+zo)*scale;}
