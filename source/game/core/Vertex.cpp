#include "game/V/Vertex.hpp"
#include "game/V/Vec3.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(float, float, float, float, float) [0x033B8F34] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Vertex::Vertex(float a1, float a2, float a3, float a4, float a5)
{
    guest::stub(0x033B8F34u, gaddr(this), (double)a1, (double)a2, (double)a3, (double)a4, (double)a5);
}
#else
Vertex::Vertex(float a1, float a2, float a3, float a4, float a5)
{
    uint32_t _frame = native_frame_open(96);
    uint32_t uVar1 = 0;
    uVar1 = Vec3::newPermanent((double)(a1), (double)(a2), (double)(a3));
    ST32(gaddr(this) + (1) * 4, (float)(a4));
    ST32(gaddr(this) + (2) * 4, (float)(a5));
    this->field_0x0 = uVar1;
    native_frame_close(_frame, 96);
}
#endif

/* {ctor}(Vertex*, float, float) [0x033B9024] - boundary stub */
Vertex::Vertex(double a1, float a2, float a3)
{
    guest::stub(0x033B9024u, gaddr(this), (uint32_t)a3, (double)a1, (double)a2);
}

/* remap(float, float) [0x033B90B4] - LOWERED-DRAFT; not reconstructed (1 open edges, 1 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Vertex::remap(float a1, float a2)
{
    guest::stub(0x033B90B4u, gaddr(this), (double)a1, (double)a2);
}
#else
void Vertex::remap(float a1, float a2)
{
    uint32_t _frame = native_frame_open(48);
    uint32_t uVar1 = 0;
    uint32_t puVar2 = 0;
    puVar2 = (uint32_t)(guest::guest_new((uint32_t)(0xc)));
    if ((puVar2 != (uint32_t)(0x0)))
    {
        uVar1 = this->field_0x0;
        gelem<uint32_t>((uint32_t)(puVar2) + 1u, 0) = (float)(a1);
        gelem<uint32_t>((uint32_t)(puVar2), 0) = uVar1;
        gelem<uint32_t>((uint32_t)(puVar2) + 2u, 0) = (float)(a2);
    }
    native_frame_close(_frame, 48);
    return;
    native_frame_close(_frame, 48);
}
#endif

}  /* namespace recon */
