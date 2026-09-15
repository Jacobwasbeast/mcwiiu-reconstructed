#include "game/W/WitherSkull.hpp"
#include "game/B/BlockPos.hpp"
#include "game/E/Entity.hpp"
#include "game/E/Explosion.hpp"
#include "game/F/Fireball.hpp"
#include "game/H/HitResult.hpp"
#include "game/L/Level.hpp"
#include "game/S/SynchedEntityData.hpp"
#include "game/W/WitherBoss.hpp"
#include "_thirdparty/boost__unordered__unordered_map_unsigned_long_long__int__boost__hash_unsigned_long_long___std__equal_to_unsigned_long_lo.hpp"
#include "_thirdparty/std__vector_unsigned_long_long__std__allocator_unsigned_long_long__.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(Level*) [0x02AA2730] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WitherSkull::WitherSkull(gptr<Level> a1)
{
    guest::stub(0x02AA2730u, gaddr(this), (uint32_t)a1);
}
#else
WitherSkull::WitherSkull(gptr<Level> a1)
{
    guest::Fireball_ct((uint32_t)(gaddr(this)), (uint32_t)(a1));
    ST32((uint32_t)((gaddr(this) + 0x348)), GSTR(L"", 0x102D234C));
    ST32((uint32_t)((gaddr(this) + 8)), 0x102D2B64u);
    this->Entity::fjDerivedCtorCalls();
    guest::Entity_setSize((double)((double)(global_0x102C0080)), (uint32_t)(gaddr(this)));
}
#endif

/* {ctor}(Level*, boost::shared_ptr<LivingEntity>, double, double, double) [0x02AA27EC] - boundary stub */
WitherSkull::WitherSkull(uint64_t a1, uint64_t a2, double a3, double a4, double a5)
{
    guest::stub(0x02AA27ECu, gaddr(this), 0u, (uint32_t)((uint64_t)a1 >> 32), (uint32_t)a1, (uint32_t)((uint64_t)a2 >> 32), (uint32_t)a2, (uint32_t)a4, (uint32_t)a5, (double)a3);
}

/* {ctor}(Level*, double, double, double, double, double, double) [0x02AA29C0] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WitherSkull::WitherSkull(uint64_t a1, double a2, double a3, double a4, double a5, double a6, double a7)
{
    guest::stub(0x02AA29C0u, gaddr(this), 0u, (uint32_t)((uint64_t)a1 >> 32), (uint32_t)a1, (uint32_t)a7, (double)a2, (double)a3, (double)a4, (double)a5, (double)a6);
}
#else
WitherSkull::WitherSkull(uint64_t a1, double a2, double a3, double a4, double a5, double a6, double a7)
{
    uint32_t _frame = native_frame_open(112);
    guest::Fireball_ct_2((uint32_t)((a1) >> 32), (uint32_t)(a1), (double)(a2), (double)(a3), (double)(a4), (double)(a5), (double)(a6), (uint32_t)(gaddr(this)), (double)(a7), (uint32_t)a1);
    ST32((uint32_t)((gaddr(this) + 0x348)), GSTR(L"", 0x102D234C));
    ST32((uint32_t)((gaddr(this) + 8)), 0x102D2B64u);
    this->Entity::fjDerivedCtorCalls();
    guest::Entity_setSize((double)((double)(global_0x102C0080)), (uint32_t)(gaddr(this)));
    native_frame_close(_frame, 112);
}
#endif

/* {dtor}() [0x02ACDA44] - LOWERED-DRAFT; not reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WitherSkull::~WitherSkull()
{
    guest::stub(0x02ACDA44u, gaddr(this), 0u);
}
#else
WitherSkull::~WitherSkull()
{
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t piVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t piVar5 = 0;
    uint8_t bVar6 = 0;
    if ((gaddr(this) != 0))
    {
        ST32((uint32_t)((gaddr(this) + 8)), 0x100FBDC4u);
        if (((uint32_t)((gaddr(this) + 0x354)) != (uint32_t)(0x0)))
        {
            piVar5 = LD32((uint32_t)((gaddr(this) + 0x354)));
            bVar1 = (piVar5 == (uint32_t)(0x0));
            if (!(bVar1))
            {
                ((void)0 /* sync dropped */);
                do
                {
                    iVar4 = (gelem<int32_t>((uint32_t)(piVar5), 0) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), iVar4), 1);
                        gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                ((void)0 /* instructionSynchronize dropped */);
                bVar6 = ((iVar4 == 0) << 1);
                if ((iVar4 == 0))
                {
                    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) + 0x14)), (uint32_t)(piVar5)) >> 32);
                    piVar3 = (piVar5 + (1) * 4);
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar4 = gelem<int32_t>((uint32_t)(piVar3), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar2 = (ST32((uint32_t)(piVar3) + (uint32_t)(0), (iVar4 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar3), 0) = iVar2;
                            bVar6 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar6 >> 1) & 1))));
                    ((void)0 /* instructionSynchronize dropped */);
                    if (((iVar4 + -(1)) == 0))
                    {
                        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) + 0x1c)), (uint32_t)(piVar5)) >> 32);
                    }
                }
            }
        }
        guest::Entity_dt((uint32_t)(gaddr(this)), (uint32_t)(0));
    }
}
#endif

/* create(Level*) [static] [0x023E6220] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void WitherSkull::create(gptr<Level> a1)
{
    guest::stub(0x023E6220u, (uint32_t)a1);
}
#else
void WitherSkull::create(gptr<Level> a1)
{
    guest::WitherSkull_ct((uint32_t)(0), (uint32_t)(a1));
    return;
}
#endif

/* staticCtor() [static] [0x02AA266C] - LOWERED-DRAFT; not reconstructed (1 open edges, 5 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherSkull::staticCtor()
{
    guest::stub(0x02AA266Cu);
}
#else
void WitherSkull::staticCtor()
{
    uint32_t _frame = native_frame_open(8);
    int32_t iVar1 = 0;
    uint32_t uVar2 = 0;
    uint32_t puVar3 = 0;
    iVar1 = global_0x104DD264;
    ST32((_frame + 4), 0x8a400002);
    ST32(_frame, 0);
    uVar2 = SynchedEntityData::defineIdInternal((uint32_t)(0), (uint32_t)(0x8a400002), (uint32_t)(0), (uint32_t)(0x8a400000));
    puVar3 = (uint32_t)(((map_unsigned_long_long_int_ *)gmem(0x104F0A34u))->operator[](_frame));
    gelem<uint32_t>((uint32_t)(puVar3), 0) = uVar2;
    if ((iVar1 == 0))
    {
        global_0x104C5224 = 0;
    }
    else
    {
        global_0x104C5224 = (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar1 + 0x1c))) + 0x1c)), (uint32_t)(iVar1), (uint32_t)(uVar2)) >> 32);
    }
    native_frame_close(_frame, 8);
    return;
    native_frame_close(_frame, 8);
}
#endif

/* isOnFire() [0x02AA2B28] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AA2B28
uint32_t WitherSkull::isOnFire()
{
    return 0;
}
#else
uint32_t WitherSkull::isOnFire()
{
    return (uint32_t)(guest::stub(0x02AA2B28u, gaddr(this)) >> 32);
}
#endif

/* getBlockExplosionResistance(Explosion*, Level*, const BlockPos&) [0x02AA2BD0] - LOWERED-DRAFT; not reconstructed (2 open edges, 2 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
double WitherSkull::getBlockExplosionResistance(gptr<Explosion> a1, gptr<Level> a2, gptr<BlockPos> a3)
{
    guest::stub(0x02AA2BD0u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
    return native_fret();
}
#else
double WitherSkull::getBlockExplosionResistance(gptr<Explosion> a1, gptr<Level> a2, gptr<BlockPos> a3)
{
    uint32_t _frame = native_frame_open(32);
    uint32_t uVar1 = 0;
    int32_t iVar2 = 0;
    double dVar3 = 0;
    dVar3 = (double)(guest::Entity_getBlockExplosionResistance(gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3));
    uVar1 = (uint32_t)(guest::indirect(LD32((a2->field_0x0 + 0x94)), (uint32_t)(a2), (uint32_t)(a3)) >> 32);
    iVar2 = this->virt_isDangerous();
    if ((((iVar2 != 0) && (iVar2 = WitherBoss::canDestroy((gptr<Block>)(uVar1)), (iVar2 != 0))) && ((float)(((double)(global_0x102B1E20) - dVar3)) < 0.0)))
    {
        dVar3 = (double)(global_0x102B1E20);
    }
    { double t1 = dVar3;
    native_frame_close(_frame, 32);
    return t1; }
    native_frame_close(_frame, 32);
}
#endif

/* addDataWalkers(DataFixerUpper*) [static] [0x02AA2CA4] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void WitherSkull::addDataWalkers(uint32_t a1, uint32_t a2)
{
    guest::stub(0x02AA2CA4u, (uint32_t)a1, (uint32_t)a2);
}
#else
void WitherSkull::addDataWalkers(uint32_t a1, uint32_t a2)
{
    guest::Fireball_addDataWalkers((uint32_t)(a1), (uint32_t)(a2), (uint32_t)(0), (uint32_t)(0x8a400002));
    return;
}
#endif

/* isPickable() [0x02AA2CC0] - LOWERED-DRAFT; not reconstructed */
#if defined(RECON_RUNTIME_BUILD)
uint32_t WitherSkull::isPickable()
{
    return (uint32_t)(guest::stub(0x02AA2CC0u, gaddr(this)) >> 32);
}
#else
uint32_t WitherSkull::isPickable()
{
    this->virt_isDangerous();
    return;
}
#endif

/* isDangerous() [0x02AA2CD0] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint8_t WitherSkull::isDangerous()
{
    return (uint8_t)(guest::stub(0x02AA2CD0u, gaddr(this)) >> 32);
}
#else
uint8_t WitherSkull::isDangerous()
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return LD8((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C5224u)) * 4)) + 4))) + 0xc)));
}
#endif

/* setDangerous(bool) [0x02AA2D50] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t WitherSkull::setDangerous(bool a1)
{
    return (uint64_t)guest::stub(0x02AA2D50u, gaddr(this), (uint32_t)a1);
}
#else
uint64_t WitherSkull::setDangerous(bool a1)
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return ((uint64_t)(uint32_t)(guest::SynchedEntityData_set__tm__2_b((uint32_t)(iVar1), (uint32_t)(global_0x104C5224), (uint32_t)(a1), (uint32_t)(1))) << 32);
}
#endif

/* getInertia() [0x02AA2DE0] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AA2DE0
double WitherSkull::getInertia()
{
    double dVar2 = 0;
    if (((this->virt_isDangerous()) != 0))
    {
        return gref<double>(0x102C0204u);
    }
    dVar2 = (double)(this->Fireball::getInertia());
    return dVar2;
}
#else
double WitherSkull::getInertia()
{
    guest::stub(0x02AA2DE0u, gaddr(this));
    return native_fret();
}
#endif

/* onHit(const HitResult*) [0x02AA2E50] - boundary stub; body not reconstructed */
void WitherSkull::onHit(gptr<HitResult> a1)
{
    guest::stub(0x02AA2E50u, gaddr(this), (uint32_t)a1);
}

/* defineSynchedData() [0x02AA36F0] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AA36F0
void WitherSkull::defineSynchedData()
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar1))->createDateItem__tm__2_b(global_0x104C5224, 0);
    return;
}
#else
void WitherSkull::defineSynchedData()
{
    guest::stub(0x02AA36F0u, gaddr(this));
}
#endif

/* shouldBurn() [0x02AA3770] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AA3770
uint32_t WitherSkull::shouldBurn()
{
    return 0;
}
#else
uint32_t WitherSkull::shouldBurn()
{
    return (uint32_t)(guest::stub(0x02AA3770u, gaddr(this)) >> 32);
}
#endif

/* getEntityTypesToIgnoreCollisionsWith() [0x02AA3778] - LOWERED-DRAFT; not reconstructed (2 open edges, 2 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gptr<vector_unsigned_long_long> WitherSkull::getEntityTypesToIgnoreCollisionsWith()
{
    return (gptr<vector_unsigned_long_long>)(guest::stub(0x02AA3778u, gaddr(this)) >> 32);
}
#else
gptr<vector_unsigned_long_long> WitherSkull::getEntityTypesToIgnoreCollisionsWith()
{
    uint32_t _frame = native_frame_open(8);
    gptr<vector_unsigned_long_long> obj = 0u;
    obj = guest::alloc<vector_unsigned_long_long>((uint32_t)(0x10));
    if ((obj != 0))
    {
        obj->_Buy((uint32_t)(0));
    }
    ST64(_frame, this->virt_GetType());
    obj->push_back(_frame);
    { int32_t t1 = obj;
    native_frame_close(_frame, 8);
    return t1; }
    native_frame_close(_frame, 8);
}
#endif

/* GetType() [0x02AD0E50] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AD0E50
uint64_t WitherSkull::GetType()
{
    return 0x8a40000200000000;
}
#else
uint64_t WitherSkull::GetType()
{
    return (uint64_t)guest::stub(0x02AD0E50u, gaddr(this));
}
#endif

}  /* namespace recon */
