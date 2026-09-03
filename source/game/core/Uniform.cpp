#include "game/core/Uniform.hpp"

Uniform::Uniform(const std::wstring& n, int t, int c, Effect* e)
    : mLocation(-1), mCount(c), mType(t), mIntBuffer(NULL), mFloatBuffer(NULL), mName(n), mEffect(e), mDirty(false)
{
    if (t < 4) mIntBuffer = new IntBuffer(c);
    else mFloatBuffer = new FloatBuffer(c);
    markDirty();
}
Uniform::~Uniform(){delete mIntBuffer;delete mFloatBuffer;}
void Uniform::markDirty(){mDirty=true;if(mEffect)mEffect->markDirty();}
void Uniform::set(float a){mFloatBuffer->position(0);mFloatBuffer->put(0,a);markDirty();}
void Uniform::set(float a,float b){mFloatBuffer->position(0);mFloatBuffer->put(0,a);mFloatBuffer->put(1,b);markDirty();}
void Uniform::set(float a,float b,float c){mFloatBuffer->position(0);mFloatBuffer->put(0,a);mFloatBuffer->put(1,b);mFloatBuffer->put(2,c);markDirty();}
void Uniform::set(const Vector3f& v){set(v.x,v.y,v.z);}
void Uniform::set(float a,float b,float c,float d){mFloatBuffer->position(0);mFloatBuffer->put(a);mFloatBuffer->put(b);mFloatBuffer->put(c);mFloatBuffer->put(d);mFloatBuffer->flip();markDirty();}
void Uniform::setSafe(float a,float b,float c,float d){
    mFloatBuffer->position(0);
    if(mCount>3)mFloatBuffer->put(0,a); if(mCount>4)mFloatBuffer->put(1,b);
    if(mCount>5)mFloatBuffer->put(2,c); if(mCount>6)mFloatBuffer->put(3,d); markDirty();
}
void Uniform::setSafe(int a,int b,int c,int d){mIntBuffer->position(0);if(mCount>0)mIntBuffer->put(0,a);if(mCount>1)mIntBuffer->put(1,b);if(mCount>2)mIntBuffer->put(2,c);if(mCount>3)mIntBuffer->put(3,d);markDirty();}
void Uniform::set(int a){mIntBuffer->position(0);mIntBuffer->put(0,a);markDirty();}
void Uniform::set(int a,int b){mIntBuffer->position(0);mIntBuffer->put(0,a);mIntBuffer->put(1,b);markDirty();}
void Uniform::set(int a,int b,int c){mIntBuffer->position(0);mIntBuffer->put(0,a);mIntBuffer->put(1,b);mIntBuffer->put(2,c);markDirty();}
void Uniform::set(int a,int b,int c,int d){
    mIntBuffer->position(0); mIntBuffer->put(0,a); mIntBuffer->put(1,b);
    mIntBuffer->put(2,c); mIntBuffer->put(3,d); markDirty();
}
void Uniform::set(arrayWithLength<float> v){if(mCount<=(int)v.length){mFloatBuffer->position(0);mFloatBuffer->put(v);mFloatBuffer->position(0);markDirty();}}
void Uniform::setMat2x2(float a,float b,float c,float d){set(a,b,c,d);}
void Uniform::setMat2x3(float a,float b,float c,float d,float e,float f){
    mFloatBuffer->position(0); mFloatBuffer->put(0,a); mFloatBuffer->put(1,b);
    mFloatBuffer->put(2,c); mFloatBuffer->put(3,d); mFloatBuffer->put(4,e);
    mFloatBuffer->put(5,f); markDirty();
}
void Uniform::setMat2x4(float a,float b,float c,float d,float e,float f,float g,float h){
    mFloatBuffer->position(0); float v[8]={a,b,c,d,e,f,g,h};
    for(int i=0;i<8;i++) mFloatBuffer->put(i,v[i]); markDirty();
}
void Uniform::setMat3x2(float a,float b,float c,float d,float e,float f){setMat2x3(a,b,c,d,e,f);}
void Uniform::setMat3x3(float a,float b,float c,float d,float e,float f,float g,float h,float i){mFloatBuffer->position(0);float v[9]={a,b,c,d,e,f,g,h,i};for(int j=0;j<9;j++)mFloatBuffer->put(j,v[j]);markDirty();}
void Uniform::setMat3x4(float a,float b,float c,float d,float e,float f,float g,float h,float i,float j,float k,float l){
    mFloatBuffer->position(0); float v[12]={a,b,c,d,e,f,g,h,i,j,k,l};
    for(int n=0;n<12;n++) mFloatBuffer->put(n,v[n]); markDirty();
}
void Uniform::setMat4x2(float a,float b,float c,float d,float e,float f,float g,float h){setMat2x4(a,b,c,d,e,f,g,h);}
void Uniform::setMat4x3(float a,float b,float c,float d,float e,float f,float g,float h,float i,float j,float k,float l){setMat3x4(a,b,c,d,e,f,g,h,i,j,k,l);}
void Uniform::setMat4x4(float a,float b,float c,float d,float e,float f,float g,float h,float i,float j,float k,float l,float m,float n,float o,float p){
    mFloatBuffer->position(0); float v[16]={a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p};
    for(int q=0;q<16;q++) mFloatBuffer->put(q,v[q]); markDirty();
}
void Uniform::set(Matrix4f* m){setMat4x4(m->m[0],m->m[1],m->m[2],m->m[3],m->m[4],m->m[5],m->m[6],m->m[7],m->m[8],m->m[9],m->m[10],m->m[11],m->m[12],m->m[13],m->m[14],m->m[15]);}
int Uniform::getTypeFromString(const std::wstring& n){if(n==L"int")return 0;if(n==L"float")return 4;return -1;}
void Uniform::setLocation(int l){mLocation=l;} std::wstring Uniform::getName(){return mName;} void Uniform::set(int i,float v){mFloatBuffer->put(i,v);markDirty();}
void Uniform::uploadAsInteger(){if(mEffect!=NULL)mEffect->apply();}
void Uniform::uploadAsFloat(){if(mEffect!=NULL)mEffect->apply();}
void Uniform::uploadAsMatrix(){if(mEffect!=NULL)mEffect->apply();}
void Uniform::upload(){if(mType<4)uploadAsInteger();else if(mType<8)uploadAsFloat();else uploadAsMatrix();}
int Uniform::getLocation(){return mLocation;}
int Uniform::getCount(){return mCount;}
int Uniform::getType(){return mType;}
IntBuffer* Uniform::getIntBuffer(){return mIntBuffer;}
FloatBuffer* Uniform::getFloatBuffer(){return mFloatBuffer;}
