#include "game/W/WitherBoss.hpp"
#include "game/A/AABB.hpp"
#include "game/B/Block.hpp"
#include "game/C/CompoundTag.hpp"
#include "game/D/DamageSource.hpp"
#include "game/D/DifficultyInstance.hpp"
#include "game/E/Entity.hpp"
#include "game/F/FlyingPathNavigation.hpp"
#include "game/G/GroundPathNavigation.hpp"
#include "game/L/L10N.hpp"
#include "game/L/Level.hpp"
#include "game/L/LivingEntity.hpp"
#include "game/M/Mob.hpp"
#include "game/M/MobEffectInstance.hpp"
#include "game/M/MobGroupData.hpp"
#include "game/M/Monster.hpp"
#include "game/M/Mth.hpp"
#include "game/S/ServerBossEvent.hpp"
#include "game/S/SynchedEntityData.hpp"
#include "_thirdparty/boost.hpp"
#include "_thirdparty/boost__shared_ptr_Entity_.hpp"
#include "_thirdparty/boost__shared_ptr_LivingEntity_.hpp"
#include "_thirdparty/boost__shared_ptr_ServerPlayer_.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(Level*) [0x02AACBD0] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WitherBoss::WitherBoss(gptr<Level> a1)
{
    guest::stub(0x02AACBD0u, gaddr(this), (uint32_t)a1);
}
#else
WitherBoss::WitherBoss(gptr<Level> a1)
{
    uint32_t _frame = native_frame_open(80);
    uint32_t uVar1 = 0;
    float fVar2 = 0;
    uint32_t pcVar3 = 0;
    uint32_t sVar4 = 0;
    uint32_t uVar5 = 0;
    uint32_t uVar6 = 0;
    double dVar7 = 0;
    double dVar8 = 0;
    double dVar9 = 0;
    guest::Monster_ct((uint32_t)(gaddr(this)), (uint32_t)(a1));
    fVar2 = global_0x102B5164;
    dVar9 = (double)(global_0x102B1E18);
    dVar8 = (double)(global_0x102B5164);
    this->field_0x774 = global_0x102B1E18;
    ST32((uint32_t)((gaddr(this) + 0x730)), 0x102C422Cu);
    ST32((uint32_t)((gaddr(this) + 0x734)), 0x102C4254u);
    this->field_0x738 = 0x28;
    this->field_0x760 = 1;
    this->field_0x764 = 0;
    this->field_0x768 = 0;
    this->field_0x76c = 0;
    this->field_0x770 = 0xf;
    this->field_0x784 = 0;
    this->field_0x780 = 0;
    this->field_0x79c = 0;
    this->field_0x788 = 3;
    this->field_0x78c = 0x14;
    this->field_0x778 = fVar2;
    this->field_0x790 = 0x14;
    this->field_0x798 = 200;
    uVar5 = global_0x102B628C;
    this->field_0x77c = fVar2;
    ST32((uint32_t)((gaddr(this) + 0x794)), uVar5);
    uVar5 = global_0x102B728C;
    this->field_0x7a8 = 0;
    this->field_0x7c0 = 0;
    this->field_0x7ec = 0;
    this->field_0x7a0 = 0;
    this->field_0x7a4 = 0;
    this->field_0x7ac = 0;
    this->field_0x7b0 = 0;
    this->field_0x7b4 = 0x28;
    this->field_0x7b8 = 0;
    this->field_0x7bc = 0x28;
    this->field_0x7c8 = 0xffffffff;
    this->field_0x7e0 = 5;
    ST32((uint32_t)((gaddr(this) + 0x7cc)), uVar5);
    ST32((uint32_t)((gaddr(this) + 8)), 0x102D5DFCu);
    ST32((uint32_t)((gaddr(this) + 0x348)), GSTR(L"", 0x102D4FA4));
    this->defineSynchedData();
    this->registerAttributes();
    this->Mob::fjDerivedCtorCalls();
    ST32((_frame + 28), 7);
    ST32((_frame + 24), 0);
    ST16((_frame + 4), 0);
    sVar4 = ::recon::wcslen((uint32_t)(GSTR(L"", 0x102C0D64)));
    ((wstring *)gmem(_frame))->assign(GSTR(L"", 0x102C0D64), (uint32_t)(sVar4));
    uVar5 = guest::ServerBossEvent_ct((uint32_t)(0), (uint32_t)(0xb6a67881), (uint32_t)(_frame), (uint32_t)(5), (uint32_t)(0));
    uVar5 = ((ServerBossEvent *)gmem(uVar5))->setDarkenScreen((bool)(1));
    this->field_0x7f0 = uVar5;
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    this->field_0x73c = fVar2;
    ST32((_frame + 24), 0);
    ST32((_frame + 28), 7);
    this->field_0x7d0 = 0;
    this->field_0x7d8 = 0;
    this->field_0x7d4 = 0;
    this->field_0x7dc = 0;
    this->field_0x744 = fVar2;
    this->field_0x74c = fVar2;
    this->field_0x754 = fVar2;
    this->field_0x740 = fVar2;
    this->field_0x748 = fVar2;
    this->field_0x750 = fVar2;
    this->field_0x758 = fVar2;
    this->field_0x1f5 = 1;
    ST16((_frame + 4), 0);
    uVar5 = this->Mob::getNavigation();
    ((GroundPathNavigation *)gmem(uVar5))->setCanFloat((bool)(1));
    dVar7 = (double)(global_0x102C0D60);
    this->field_0x5f4 = 0x32;
    this->field_0x190 = (this->field_0x190 | 0x4004);
    this->Entity::setSize((float)(dVar9), (float)(dVar7));
    this->setAerialAttack((bool)(1));
    dVar7 = (double)(this->LivingEntity::getMaxHealth());
    this->field_0x7c4 = (int32_t)(dVar7);
    this->LivingEntity::setHealth((float)(dVar9));
    (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((this->field_0x7f0 + 0x54))) + 0x24)), (double)(dVar8)) >> 32);
    dVar8 = (double)(this->LivingEntity::getMaxHealth());
    this->field_0x7c8 = (int32_t)(dVar8);
    dVar8 = (double)(this->LivingEntity::getMaxHealth());
    uVar1 = (uint32_t)(dVar8);
    this->field_0x7e4 = (((int32_t)(uVar1) >> 1) + (uint32_t)((((int32_t)(uVar1) < 0) && ((uVar1 & 1) != 0))));
    dVar8 = (double)(this->LivingEntity::getMaxHealth());
    pcVar3 = LD32((uint32_t)((LD32((uint32_t)((global_0x104DAC48 + 0x18))) + 0x44)));
    this->field_0x7e8 = ((int32_t)(dVar8) / 6);
    uVar6 = (uint32_t)(guest::indirect((uint32_t)(pcVar3)) >> 32);
    uVar5 = global_0x102B6BB0;
    this->field_0x75c = uVar6;
    this->field_0x3f0 = uVar5;
    native_frame_close(_frame, 80);
}
#endif

/* {dtor}() [0x02A1D840] - LOWERED-DRAFT; not reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WitherBoss::~WitherBoss()
{
    guest::stub(0x02A1D840u, gaddr(this), 0u);
}
#else
WitherBoss::~WitherBoss()
{
    gptr<DamageSource> source = 0u;
    source = this->field_0x75c;
    if ((source != 0))
    {
        ST32((uint32_t)((gaddr(this) + 8)), 0x102D5DFCu);
        source->virt_0xc((uint32_t)(3));
    }
}
#endif

/* create(Level*) [static] [0x023E65F4] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void WitherBoss::create(gptr<Level> a1)
{
    guest::stub(0x023E65F4u, (uint32_t)a1);
}
#else
void WitherBoss::create(gptr<Level> a1)
{
    guest::WitherBoss_ct((uint32_t)(0), (uint32_t)(a1));
    return;
}
#endif

/* getDisplayName() [0x02A1D91C] - LOWERED-DRAFT; not reconstructed (3 open edges, 3 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gresult<wstring> WitherBoss::getDisplayName()
{
    gresult<wstring> result;
    guest::stub(0x02A1D91Cu, gaddr(this), (uint32_t)result);
    return result;
}
#else
gresult<wstring> WitherBoss::getDisplayName(int32_t a1)
{
    int32_t iVar1 = 0;
    uint32_t __s = 0;
    uint32_t sVar2 = 0;
    iVar1 = this->virt_hasCustomName();
    if ((iVar1 != 0))
    {
        this->virt_getCustomName((uint32_t)(a1));
        return;
    }
    __s = (uint32_t)(L10N::GetString((int32_t)(0xb6a67881)));
    if (((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0x20)), (a1 != 0))))
    {
        ST32((uint32_t)((a1 + 0x18)), 0);
        ST16((uint32_t)((a1 + 4)), 0);
        ST32((uint32_t)((a1 + 0x1c)), 7);
        sVar2 = ::recon::wcslen(__s);
        ((wstring *)gmem(a1))->assign(__s, (uint32_t)(sVar2));
    }
    return;
}
#endif

/* performRangedAttack(boost::shared_ptr<LivingEntity>, float) [0x02A1DA44] - LOWERED-DRAFT; not reconstructed (3 open edges, 25 raw IR ops, 7 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t WitherBoss::performRangedAttack(uint32_t a1)
{
    return (uint64_t)guest::stub(0x02A1DA44u, gaddr(this), (uint32_t)a1);
}
#else
uint64_t WitherBoss::performRangedAttack(uint32_t a1)
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t piVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t piVar5 = 0;
    uint8_t bVar6 = 0;
    iVar4 = this->field_0x348;
    ST32(_frame, gelem<uint32_t>((uint32_t)(a1), 0));
    ST32((_frame + 4), (uint32_t)(gelem<uint32_t>((uint32_t)(a1) + 1u, 0)));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    while (!(bVar1))
    {
        if (('\x01' != '\0'))
        {
            iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
            ST32(LD32((_frame + 4)), iVar2);
            bVar1 = 1;
        }
    }
    (uint32_t)(guest::indirect(LD32((iVar4 + 0xe1c)), (uint32_t)(gaddr(this)), (uint32_t)(0), (uint32_t)(_frame)) >> 32);
    piVar3 = LD32((_frame + 4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar4 = (LD32(LD32((_frame + 4))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar4), 1);
                ST32(LD32((_frame + 4)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar6 = ((iVar4 == 0) << 1);
        if ((iVar4 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            piVar5 = (piVar3 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar5), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar6 = 2;
                }
            }
            while (!((uint8_t)(((bVar6 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) + 0x1c)), (uint32_t)(piVar3)) >> 32);
            }
        }
    }
    { uint64_t t1 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
    native_frame_close(_frame, 8);
    return t1; }
    native_frame_close(_frame, 8);
}
#endif

/* isAggressive() [0x02A1DB48] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A1DB48
uint32_t WitherBoss::isAggressive()
{
    return 0;
}
#else
uint32_t WitherBoss::isAggressive()
{
    return (uint32_t)(guest::stub(0x02A1DB48u, gaddr(this)) >> 32);
}
#endif

/* setAggressive(bool) [0x02A1DB64] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A1DB64
void WitherBoss::setAggressive(bool a1)
{
    return;
}
#else
void WitherBoss::setAggressive(bool a1)
{
    guest::stub(0x02A1DB64u, gaddr(this), (uint32_t)a1);
}
#endif

/* canDestroy(Block*) [static] [0x02AA2B30] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AA2B30
uint32_t WitherBoss::canDestroy(gptr<Block> a1)
{
    uint32_t uVar1 = 0;
    uVar1 = 0;
    if (((((((a1 != global_0x104C190C) && (a1 != global_0x104C1B10)) && (a1 != global_0x104C1B14)) && ((a1 != global_0x104C1B58) && (a1 != global_0x104C1BB8))) && ((a1 != global_0x104C1CE8) && ((a1 != global_0x104C1C58) && (a1 != global_0x104C1988)))) && (a1 != global_0x104C1C38)))
    {
        uVar1 = 1;
    }
    return uVar1;
}
#else
uint32_t WitherBoss::canDestroy(gptr<Block> a1)
{
    return (uint32_t)(guest::stub(0x02AA2B30u, (uint32_t)a1) >> 32);
}
#endif

/* addDataWalkers(DataFixerUpper*) [static] [0x02AABFF0] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AABFF0
void WitherBoss::addDataWalkers(uint32_t a1, uint32_t a2)
{
    Mob::addDataWalkers((a1), (a2), 0, 0x5d540047);
    return;
}
#else
void WitherBoss::addDataWalkers(uint32_t a1, uint32_t a2)
{
    guest::stub(0x02AABFF0u, (uint32_t)a1, (uint32_t)a2);
}
#endif

/* staticCtor() [static] [0x02AAC00C] - boundary stub; body not reconstructed */
uint64_t WitherBoss::staticCtor()
{
    return (uint64_t)guest::stub(0x02AAC00Cu);
}

/* defineSynchedData() [0x02AAC5CC] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AAC5CC
void WitherBoss::defineSynchedData()
{
    uint32_t uVar1 = 0;
    uint32_t uVar2 = 0;
    int32_t iVar3 = 0;
    this->Mob::defineSynchedData();
    iVar3 = this->field_0x1f8;
    if ((iVar3 == 0))
    {
        iVar3 = this->field_0x1f8;
    }
    uVar1 = global_0x104C522C;
    uVar2 = this->Entity::getId();
    ((SynchedEntityData *)gmem(iVar3))->createDateItem__tm__2_i(uVar1, (uVar2));
    iVar3 = this->field_0x1f8;
    if ((iVar3 == 0))
    {
        iVar3 = this->field_0x1f8;
    }
    uVar1 = global_0x104C5230;
    uVar2 = this->Entity::getId();
    ((SynchedEntityData *)gmem(iVar3))->createDateItem__tm__2_i(uVar1, (uVar2));
    iVar3 = this->field_0x1f8;
    if ((iVar3 == 0))
    {
        iVar3 = this->field_0x1f8;
    }
    uVar1 = global_0x104C5234;
    uVar2 = this->Entity::getId();
    ((SynchedEntityData *)gmem(iVar3))->createDateItem__tm__2_i(uVar1, (uVar2));
    iVar3 = this->field_0x1f8;
    if ((iVar3 == 0))
    {
        iVar3 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar3))->createDateItem__tm__2_i(global_0x104C5238, 0);
    iVar3 = this->field_0x1f8;
    if ((iVar3 == 0))
    {
        iVar3 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar3))->createDateItem__tm__2_b(global_0x104C523C, 0);
    iVar3 = this->field_0x1f8;
    if ((iVar3 == 0))
    {
        iVar3 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar3))->createDateItem__tm__2_b(global_0x104C5240, 0);
    return;
}
#else
void WitherBoss::defineSynchedData()
{
    guest::stub(0x02AAC5CCu, gaddr(this));
}
#endif

/* registerAttributes() [0x02AAC838] - LOWERED-DRAFT; not reconstructed (11 open edges, 12 raw IR ops, 6 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherBoss::registerAttributes()
{
    guest::stub(0x02AAC838u, gaddr(this));
}
#else
void WitherBoss::registerAttributes()
{
    uint32_t _frame = native_frame_open(32);
    int32_t iVar1 = 0;
    uint32_t piVar2 = 0;
    uint32_t uVar3 = 0;
    uint32_t pcVar4 = 0;
    double dVar5 = 0;
    double dVar6 = 0;
    guest::Monster_registerAttributes(gaddr(this));
    iVar1 = this->virt_getAttributes();
    (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar1 + 0x24))) + 0x24)), (uint32_t)(iVar1), (uint32_t)(global_0x104EFE20)) >> 32);
    piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE10)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102C0C88)) >> 32);
    piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE1C)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102C0C90)) >> 32);
    iVar1 = (uint32_t)((this->field_0xf8->getDifficulty()) >> 32);
    if ((iVar1 == global_0x104DCB24))
    {
        piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE10)));
        pcVar4 = LD32((uint32_t)((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)));
        dVar5 = (double)(this->virt_getMaxHealth());
        (uint32_t)(guest::indirect((uint32_t)(pcVar4), (double)((double)((float)((dVar5 * (double)(global_0x102B435C))))), (uint32_t)(piVar2)) >> 32);
        uVar3 = this->field_0x738;
        pcVar4 = LD32((uint32_t)((this->field_0x348 + 0xa24)));
        this->field_0x738 = (((int32_t)(uVar3) >> 1) + (uint32_t)((((int32_t)(uVar3) < 0) && ((uVar3 & 1) != 0))));
        piVar2 = (uint32_t)((uint32_t)(guest::indirect((uint32_t)(pcVar4), (uint32_t)(gaddr(this)), (uint32_t)(global_0x104EFE14)) >> 32));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102C0C98)) >> 32);
        piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE2C)));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102B77A8)) >> 32);
    }
    else
    {
        iVar1 = (uint32_t)((this->field_0xf8->getDifficulty()) >> 32);
        if ((iVar1 == global_0x104DCB28))
        {
            piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE10)));
            dVar6 = (double)(global_0x102B1E14);
            pcVar4 = LD32((uint32_t)((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)));
            dVar5 = (double)(this->virt_getMaxHealth());
            (uint32_t)(guest::indirect((uint32_t)(pcVar4), (double)((double)((float)((dVar5 * dVar6)))), (uint32_t)(piVar2)) >> 32);
            this->field_0x738 = (int32_t)(((double)((float)(((double)(int32_t)(this->field_0x738)))) * dVar6));
        }
        piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE14)));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102C0C98)) >> 32);
        piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE2C)));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102B77A8)) >> 32);
    }
    native_frame_close(_frame, 32);
    return;
    native_frame_close(_frame, 32);
}
#endif

/* setAerialAttack(bool) [0x02AACB40] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AACB40
void WitherBoss::setAerialAttack(bool a1)
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar1))->set__tm__2_b(global_0x104C523C, (a1), 1);
    return;
}
#else
void WitherBoss::setAerialAttack(bool a1)
{
    guest::stub(0x02AACB40u, gaddr(this), (uint32_t)a1);
}
#endif

/* setCustomName(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&, const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&, bool) [0x02AAD018] - LOWERED-DRAFT; not reconstructed (2 open edges, 5 raw IR ops, 2 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherBoss::setCustomName(gptr<wstring> a1, gptr<wstring> a2, bool a3)
{
    guest::stub(0x02AAD018u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
}
#else
void WitherBoss::setCustomName(gptr<wstring> a1, gptr<wstring> a2, bool a3)
{
    uint32_t _frame = native_frame_open(32);
    int32_t iVar1 = 0;
    guest::Entity_setCustomName(gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
    iVar1 = LD32((uint32_t)((this->field_0x7f0 + 0x54)));
    this->virt_getDisplayName((int32_t)(_frame));
    (uint32_t)(guest::indirect(LD32((iVar1 + 0x14)), (uint32_t)(this->field_0x7f0), (uint32_t)(_frame)) >> 32);
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    native_frame_close(_frame, 32);
    return;
    native_frame_close(_frame, 32);
}
#endif

/* getAerialAttack() const [0x02AAD0AC] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint8_t WitherBoss::getAerialAttack()
{
    return (uint8_t)(guest::stub(0x02AAD0ACu, gaddr(this)) >> 32);
}
#else
uint8_t WitherBoss::getAerialAttack()
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return LD8((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C523Cu)) * 4)) + 4))) + 0xc)));
}
#endif

/* addAdditonalSaveData(CompoundTag*) [0x02AAD12C] - boundary stub; body not reconstructed */
void WitherBoss::addAdditonalSaveData(gptr<CompoundTag> a1)
{
    guest::stub(0x02AAD12Cu, gaddr(this), (uint32_t)a1);
}

/* getInvulnerableTicks() [0x02AAD740] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t WitherBoss::getInvulnerableTicks()
{
    return (uint32_t)(guest::stub(0x02AAD740u, gaddr(this)) >> 32);
}
#else
uint32_t WitherBoss::getInvulnerableTicks()
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return LD32((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C5238u)) * 4)) + 4))) + 0xc)));
}
#endif

/* setInvulnerableTicks(int) [0x02AAD7C0] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AAD7C0
void WitherBoss::setInvulnerableTicks(int32_t a1)
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar1))->set__tm__2_i(global_0x104C5238, (a1), 1);
    return;
}
#else
void WitherBoss::setInvulnerableTicks(int32_t a1)
{
    guest::stub(0x02AAD7C0u, gaddr(this), (uint32_t)a1);
}
#endif

/* readAdditionalSaveData(CompoundTag*) [0x02AAD850] - LOWERED-DRAFT; not reconstructed (48 open edges, 190 raw IR ops, 4 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherBoss::readAdditionalSaveData(gptr<CompoundTag> a1)
{
    guest::stub(0x02AAD850u, gaddr(this), (uint32_t)a1);
}
#else
void WitherBoss::readAdditionalSaveData(gptr<CompoundTag> a1)
{
    uint32_t _frame = native_frame_open(424);
    uint8_t bVar1 = 0;
    uint32_t sVar2 = 0;
    uint32_t uVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t uVar5 = 0;
    double dVar6 = 0;
    guest::Mob_readAdditionalSaveData(gaddr(this), (uint32_t)a1);
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar2 = ::recon::wcslen(GSTR("Invul", 0x102C10F0));
    ((wstring *)gmem(_frame))->assign(GSTR("Invul", 0x102C10F0), (uint32_t)(sVar2));
    uVar3 = a1->getInt((gptr<wstring>)(_frame));
    this->setInvulnerableTicks((int32_t)(uVar3));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    iVar4 = this->virt_hasCustomName();
    if ((iVar4 != 0))
    {
        iVar4 = LD32((uint32_t)((this->field_0x7f0 + 0x54)));
        this->virt_getDisplayName((int32_t)((_frame + 360)));
        (uint32_t)(guest::indirect(LD32((iVar4 + 0x14)), (uint32_t)(this->field_0x7f0), (uint32_t)((_frame + 360))) >> 32);
        if ((7 < LD32((_frame + 388))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 380))));
        }
        ST32((_frame + 384), 0);
        ST16((_frame + 364), 0);
        ST32((_frame + 388), 7);
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    bVar1 = 1;
    sVar2 = ::recon::wcslen(GSTR("Phase", 0x102C10FC));
    ((wstring *)gmem(_frame))->assign(GSTR("Phase", 0x102C10FC), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    if ((iVar4 != 0))
    {
        ST32((_frame + 28), 7);
        ST16((_frame + 4), 0);
        ST32((_frame + 24), 0);
        ST32((_frame + 64), 0);
        ST16((_frame + 44), 0);
        ST32((_frame + 68), 7);
        sVar2 = ::recon::wcslen(GSTR("Phase", 0x102C10FC));
        ((wstring *)gmem((_frame + 40)))->assign(GSTR("Phase", 0x102C10FC), (uint32_t)(sVar2));
        uVar3 = a1->getInt((gptr<wstring>)((_frame + 40)));
        this->field_0x760 = uVar3;
        if ((7 < LD32((_frame + 68))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 60))));
        }
        ST32((_frame + 64), 0);
        ST32((_frame + 68), 7);
        bVar1 = 0;
        ST16((_frame + 44), 0);
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar2 = ::recon::wcslen(GSTR("AirAttack", 0x102C1108));
    ((wstring *)gmem(_frame))->assign(GSTR("AirAttack", 0x102C1108), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    if ((iVar4 != 0))
    {
        ST32((_frame + 28), 7);
        ST32((_frame + 24), 0);
        ST16((_frame + 4), 0);
        ST32((_frame + 96), 0);
        ST16((_frame + 76), 0);
        ST32((_frame + 100), 7);
        sVar2 = ::recon::wcslen(GSTR("AirAttack", 0x102C1108));
        ((wstring *)gmem((_frame + 72)))->assign(GSTR("AirAttack", 0x102C1108), (uint32_t)(sVar2));
        uVar3 = a1->getBoolean((gptr<wstring>)((_frame + 72)));
        this->setAerialAttack((bool)(uVar3));
        if ((7 < LD32((_frame + 100))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 92))));
        }
        ST32((_frame + 96), 0);
        ST16((_frame + 76), 0);
        ST32((_frame + 100), 7);
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar2 = ::recon::wcslen(GSTR("ShieldHealth", 0x102C1130));
    ((wstring *)gmem(_frame))->assign(GSTR("ShieldHealth", 0x102C1130), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    if ((iVar4 != 0))
    {
        ST32((_frame + 28), 7);
        ST16((_frame + 4), 0);
        ST32((_frame + 24), 0);
        ST32((_frame + 132), 7);
        ST32((_frame + 128), 0);
        ST16((_frame + 108), 0);
        sVar2 = ::recon::wcslen(GSTR("ShieldHealth", 0x102C1130));
        ((wstring *)gmem((_frame + 104)))->assign(GSTR("ShieldHealth", 0x102C1130), (uint32_t)(sVar2));
        uVar3 = a1->getInt((gptr<wstring>)((_frame + 104)));
        this->field_0x7b4 = uVar3;
        if ((7 < LD32((_frame + 132))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 124))));
        }
        ST32((_frame + 128), 0);
        ST16((_frame + 108), 0);
        ST32((_frame + 132), 7);
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar2 = ::recon::wcslen(GSTR("maxHealth", 0x102C111C));
    ((wstring *)gmem(_frame))->assign(GSTR("maxHealth", 0x102C111C), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    if ((iVar4 != 0))
    {
        ST32((_frame + 28), 7);
        ST32((_frame + 24), 0);
        ST16((_frame + 4), 0);
        ST32((_frame + 160), 0);
        ST16((_frame + 140), 0);
        ST32((_frame + 164), 7);
        sVar2 = ::recon::wcslen(GSTR("maxHealth", 0x102C111C));
        ((wstring *)gmem((_frame + 136)))->assign(GSTR("maxHealth", 0x102C111C), (uint32_t)(sVar2));
        uVar3 = a1->getInt((gptr<wstring>)((_frame + 136)));
        this->field_0x7c4 = uVar3;
        if ((7 < LD32((_frame + 164))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 156))));
        }
        ST32((_frame + 160), 0);
        ST16((_frame + 140), 0);
        ST32((_frame + 164), 7);
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar2 = ::recon::wcslen(GSTR("lastHealthInterval", 0x102C114C));
    ((wstring *)gmem(_frame))->assign(GSTR("lastHealthInterval", 0x102C114C), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    if ((iVar4 != 0))
    {
        ST32((_frame + 28), 7);
        ST32((_frame + 192), 0);
        ST16((_frame + 4), 0);
        ST32((_frame + 196), 7);
        ST32((_frame + 24), 0);
        ST16((_frame + 172), 0);
        sVar2 = ::recon::wcslen(GSTR("lastHealthInterval", 0x102C114C));
        ((wstring *)gmem((_frame + 168)))->assign(GSTR("lastHealthInterval", 0x102C114C), (uint32_t)(sVar2));
        uVar3 = a1->getInt((gptr<wstring>)((_frame + 168)));
        this->field_0x7c8 = uVar3;
        if ((7 < LD32((_frame + 196))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 188))));
        }
        ST32((_frame + 192), 0);
        ST16((_frame + 172), 0);
        ST32((_frame + 196), 7);
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar2 = ::recon::wcslen(GSTR("dyingFrames", 0x102C10C0));
    ((wstring *)gmem(_frame))->assign(GSTR("dyingFrames", 0x102C10C0), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    if ((iVar4 != 0))
    {
        ST32((_frame + 28), 7);
        ST32((_frame + 24), 0);
        ST16((_frame + 4), 0);
        ST32((_frame + 224), 0);
        ST16((_frame + 204), 0);
        ST32((_frame + 228), 7);
        sVar2 = ::recon::wcslen(GSTR("dyingFrames", 0x102C10C0));
        ((wstring *)gmem((_frame + 200)))->assign(GSTR("dyingFrames", 0x102C10C0), (uint32_t)(sVar2));
        uVar3 = a1->getInt((gptr<wstring>)((_frame + 200)));
        this->field_0x76c = uVar3;
        if ((7 < LD32((_frame + 228))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 220))));
        }
        ST32((_frame + 224), 0);
        ST16((_frame + 204), 0);
        ST32((_frame + 228), 7);
    }
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    ST32((_frame + 28), 7);
    sVar2 = ::recon::wcslen(GSTR("swellAmount", 0x102C10D8));
    ((wstring *)gmem(_frame))->assign(GSTR("swellAmount", 0x102C10D8), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    if ((iVar4 != 0))
    {
        ST32((_frame + 28), 7);
        ST32((_frame + 256), 0);
        ST16((_frame + 4), 0);
        ST32((_frame + 260), 7);
        ST32((_frame + 24), 0);
        ST16((_frame + 236), 0);
        sVar2 = ::recon::wcslen(GSTR("swellAmount", 0x102C10D8));
        ((wstring *)gmem((_frame + 232)))->assign(GSTR("swellAmount", 0x102C10D8), (uint32_t)(sVar2));
        dVar6 = (double)(a1->getFloat((gptr<wstring>)((_frame + 232))));
        this->field_0x778 = (float)(dVar6);
        if ((7 < LD32((_frame + 260))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 252))));
        }
        ST32((_frame + 256), 0);
        ST16((_frame + 236), 0);
        ST32((_frame + 260), 7);
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar2 = ::recon::wcslen(GSTR("oldSwellAmount", 0x102C1174));
    ((wstring *)gmem(_frame))->assign(GSTR("oldSwellAmount", 0x102C1174), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    if ((iVar4 != 0))
    {
        ST32((_frame + 28), 7);
        ST32((_frame + 24), 0);
        ST16((_frame + 4), 0);
        ST32((_frame + 292), 7);
        ST32((_frame + 288), 0);
        ST16((_frame + 268), 0);
        sVar2 = ::recon::wcslen(GSTR("oldSwellAmount", 0x102C1174));
        ((wstring *)gmem((_frame + 264)))->assign(GSTR("oldSwellAmount", 0x102C1174), (uint32_t)(sVar2));
        dVar6 = (double)(a1->getFloat((gptr<wstring>)((_frame + 264))));
        this->field_0x77c = (float)(dVar6);
        if ((7 < LD32((_frame + 292))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 284))));
        }
        ST32((_frame + 288), 0);
        ST16((_frame + 268), 0);
        ST32((_frame + 292), 7);
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar2 = ::recon::wcslen(GSTR("firerate", 0x102C1194));
    ((wstring *)gmem(_frame))->assign(GSTR("firerate", 0x102C1194), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 24), 0);
    ST16((_frame + 4), 0);
    ST32((_frame + 28), 7);
    if ((iVar4 != 0))
    {
        ST32((_frame + 324), 7);
        ST16((_frame + 300), 0);
        ST32((_frame + 320), 0);
        sVar2 = ::recon::wcslen(GSTR("firerate", 0x102C1194));
        ((wstring *)gmem((_frame + 296)))->assign(GSTR("firerate", 0x102C1194), (uint32_t)(sVar2));
        uVar3 = a1->getInt((gptr<wstring>)((_frame + 296)));
        this->field_0x78c = uVar3;
        if ((7 < LD32((_frame + 324))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 316))));
        }
        ST32((_frame + 320), 0);
        ST16((_frame + 300), 0);
        ST32((_frame + 324), 7);
    }
    uVar5 = this->getInvulnerableTicks();
    ST16((_frame + 4), 0);
    ST32((_frame + 28), 7);
    ST32((_frame + 24), 0);
    sVar2 = ::recon::wcslen(GSTR("SpawningFrames", 0x102C11A8));
    ((wstring *)gmem(_frame))->assign(GSTR("SpawningFrames", 0x102C11A8), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    if ((iVar4 == 0))
    {
        if (((int32_t)((uVar5 | (uVar5 - 1))) < 0))
        {
            this->field_0x798 = 0;
        }
    }
    else
    {
        ST32((_frame + 28), 7);
        ST32((_frame + 352), 0);
        ST16((_frame + 4), 0);
        ST32((_frame + 356), 7);
        ST32((_frame + 24), 0);
        ST16((_frame + 332), 0);
        sVar2 = ::recon::wcslen(GSTR("SpawningFrames", 0x102C11A8));
        ((wstring *)gmem((_frame + 328)))->assign(GSTR("SpawningFrames", 0x102C11A8), (uint32_t)(sVar2));
        uVar3 = a1->getInt((gptr<wstring>)((_frame + 328)));
        this->field_0x798 = uVar3;
        if ((7 < LD32((_frame + 356))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 348))));
        }
        ST32((_frame + 352), 0);
        ST16((_frame + 332), 0);
        ST32((_frame + 356), 7);
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar2 = ::recon::wcslen(GSTR("overlayAlpha", 0x102C11C8));
    ((wstring *)gmem(_frame))->assign(GSTR("overlayAlpha", 0x102C11C8), (uint32_t)(sVar2));
    iVar4 = a1->contains((gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 24), 0);
    ST16((_frame + 4), 0);
    ST32((_frame + 28), 7);
    if ((iVar4 == 0))
    {
        if ((this->field_0x798 < 1))
        {
            ST32((uint32_t)((gaddr(this) + 0x774)), global_0x102B5164);
        }
        else
        {
            ST32((uint32_t)((gaddr(this) + 0x774)), global_0x102B1E18);
        }
    }
    else
    {
        ST32((_frame + 416), 0);
        ST16((_frame + 396), 0);
        ST32((_frame + 420), 7);
        sVar2 = ::recon::wcslen(GSTR("overlayAlpha", 0x102C11C8));
        ((wstring *)gmem((_frame + 392)))->assign(GSTR("overlayAlpha", 0x102C11C8), (uint32_t)(sVar2));
        dVar6 = (double)(a1->getFloat((gptr<wstring>)((_frame + 392))));
        this->field_0x774 = (float)(dVar6);
        ((wstring *)gmem((_frame + 392)))->_Tidy((bool)(1), (uint32_t)(0));
    }
    if (bVar1)
    {
        dVar6 = (double)(this->virt_getMaxHealth());
        uVar5 = (uint32_t)(dVar6);
        ST32((_frame + 32), uVar5);
        dVar6 = (double)(this->virt_getHealth());
        this->field_0x7c8 = (int32_t)(dVar6);
        iVar4 = (((int32_t)(uVar5) >> 1) + (uint32_t)((((int32_t)(uVar5) < 0) && ((uVar5 & 1) != 0))));
        this->field_0x7e4 = iVar4;
        this->field_0x7e8 = ((int32_t)(uVar5) / 6);
        if (((int32_t)(dVar6) <= iVar4))
        {
            this->field_0x760 = 0;
        }
    }
    native_frame_close(_frame, 424);
    return;
    native_frame_close(_frame, 424);
}
#endif

/* readAdditionContructionTag(CompoundTag*) [0x02AAE814] - LOWERED-DRAFT; not reconstructed (22 open edges, 56 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherBoss::readAdditionContructionTag(gptr<CompoundTag> a1)
{
    guest::stub(0x02AAE814u, gaddr(this), (uint32_t)a1);
}
#else
void WitherBoss::readAdditionContructionTag(gptr<CompoundTag> a1)
{
    uint32_t _frame = native_frame_open(32);
    uint32_t sVar1 = 0;
    uint32_t uVar2 = 0;
    double dVar3 = 0;
    ST32((_frame + 24), 0);
    ST16((_frame + 4), 0);
    ST32((_frame + 28), 7);
    sVar1 = ::recon::wcslen(GSTR("AirAttack", 0x102C1214));
    ((wstring *)gmem(_frame))->assign(GSTR("AirAttack", 0x102C1214), (uint32_t)(sVar1));
    uVar2 = a1->getBoolean((gptr<wstring>)(_frame));
    this->setAerialAttack((bool)(uVar2));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar1 = ::recon::wcslen(GSTR("ShieldHealth", 0x102C1248));
    ((wstring *)gmem(_frame))->assign(GSTR("ShieldHealth", 0x102C1248), (uint32_t)(sVar1));
    uVar2 = a1->getInt((gptr<wstring>)(_frame));
    this->field_0x7b4 = uVar2;
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar1 = ::recon::wcslen(GSTR("Phase", 0x102C1228));
    ((wstring *)gmem(_frame))->assign(GSTR("Phase", 0x102C1228), (uint32_t)(sVar1));
    uVar2 = a1->getInt((gptr<wstring>)(_frame));
    this->field_0x760 = uVar2;
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar1 = ::recon::wcslen(GSTR("SpawningFrames", 0x102C1264));
    ((wstring *)gmem(_frame))->assign(GSTR("SpawningFrames", 0x102C1264), (uint32_t)(sVar1));
    uVar2 = a1->getInt((gptr<wstring>)(_frame));
    this->field_0x798 = uVar2;
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar1 = ::recon::wcslen(GSTR("maxHealth", 0x102C1234));
    ((wstring *)gmem(_frame))->assign(GSTR("maxHealth", 0x102C1234), (uint32_t)(sVar1));
    uVar2 = a1->getInt((gptr<wstring>)(_frame));
    this->field_0x7c4 = uVar2;
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar1 = ::recon::wcslen(GSTR("lastHealthInterval", 0x102C1284));
    ((wstring *)gmem(_frame))->assign(GSTR("lastHealthInterval", 0x102C1284), (uint32_t)(sVar1));
    uVar2 = a1->getInt((gptr<wstring>)(_frame));
    this->field_0x7c8 = uVar2;
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    ST32((_frame + 28), 7);
    sVar1 = ::recon::wcslen(GSTR("dyingFrames", 0x102C11E4));
    ((wstring *)gmem(_frame))->assign(GSTR("dyingFrames", 0x102C11E4), (uint32_t)(sVar1));
    uVar2 = a1->getInt((gptr<wstring>)(_frame));
    this->field_0x76c = uVar2;
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar1 = ::recon::wcslen(GSTR("overlayAlpha", 0x102C12AC));
    ((wstring *)gmem(_frame))->assign(GSTR("overlayAlpha", 0x102C12AC), (uint32_t)(sVar1));
    dVar3 = (double)(a1->getFloat((gptr<wstring>)(_frame)));
    this->field_0x774 = (float)(dVar3);
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    sVar1 = ::recon::wcslen(GSTR("swellAmount", 0x102C11FC));
    ((wstring *)gmem(_frame))->assign(GSTR("swellAmount", 0x102C11FC), (uint32_t)(sVar1));
    dVar3 = (double)(a1->getFloat((gptr<wstring>)(_frame)));
    this->field_0x778 = (float)(dVar3);
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    ST32((_frame + 28), 7);
    sVar1 = ::recon::wcslen(GSTR("oldSwellAmount", 0x102C12C8));
    ((wstring *)gmem(_frame))->assign(GSTR("oldSwellAmount", 0x102C12C8), (uint32_t)(sVar1));
    dVar3 = (double)(a1->getFloat((gptr<wstring>)(_frame)));
    this->field_0x77c = (float)(dVar3);
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 24), 0);
    ST16((_frame + 4), 0);
    ST32((_frame + 28), 7);
    sVar1 = ::recon::wcslen(GSTR("firerate", 0x102C12E8));
    ((wstring *)gmem(_frame))->assign(GSTR("firerate", 0x102C12E8), (uint32_t)(sVar1));
    uVar2 = a1->getInt((gptr<wstring>)(_frame));
    this->field_0x78c = uVar2;
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    native_frame_close(_frame, 32);
    return;
    native_frame_close(_frame, 32);
}
#endif

/* getAdditionContructionTag() [0x02AAEE18] - boundary stub; body not reconstructed */
uint32_t WitherBoss::getAdditionContructionTag()
{
    return (uint32_t)(guest::stub(0x02AAEE18u, gaddr(this)) >> 32);
}

/* getAlternativeTarget(int) [0x02AAF468] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 4 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t WitherBoss::getAlternativeTarget(int32_t a1)
{
    return (uint32_t)(guest::stub(0x02AAF468u, gaddr(this), (uint32_t)a1) >> 32);
}
#else
uint32_t WitherBoss::getAlternativeTarget(int32_t a1)
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return LD32((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32((uint32_t)((0x104C527Cu + (a1 * 4))))) * 4)) + 4))) + 0xc)));
}
#endif

/* isPowered() [0x02AAF4F8] - LOWERED-DRAFT; not reconstructed (2 open edges, 3 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
bool WitherBoss::isPowered()
{
    return (bool)(guest::stub(0x02AAF4F8u, gaddr(this)) >> 32);
}
#else
bool WitherBoss::isPowered()
{
    uint32_t _frame = native_frame_open(32);
    int32_t iVar1 = 0;
    double dVar2 = 0;
    double dVar3 = 0;
    iVar1 = this->field_0x348;
    dVar2 = (double)((uint32_t)(guest::indirect(LD32((iVar1 + 0x92c))) >> 32));
    dVar3 = (double)((uint32_t)(guest::indirect(LD32((iVar1 + 0x9fc)), (uint32_t)(gaddr(this))) >> 32));
    { int32_t t1 = (dVar2 <= (double)((float)((dVar3 * (double)(global_0x102B435C)))));
    native_frame_close(_frame, 32);
    return t1; }
    native_frame_close(_frame, 32);
}
#endif

/* generateParticles() [0x02AAF580] - boundary stub; body not reconstructed */
uint64_t WitherBoss::generateParticles()
{
    return (uint64_t)guest::stub(0x02AAF580u, gaddr(this));
}

/* IsChargeAttacking() [0x02AAFDC8] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint8_t WitherBoss::IsChargeAttacking()
{
    return (uint8_t)(guest::stub(0x02AAFDC8u, gaddr(this)) >> 32);
}
#else
uint8_t WitherBoss::IsChargeAttacking()
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return LD8((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C5240u)) * 4)) + 4))) + 0xc)));
}
#endif

/* canShoot() [0x02AAFE48] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AAFE48
uint32_t WitherBoss::canShoot()
{
    uint32_t uVar1 = 0;
    uVar1 = 0;
    if (((((this->field_0x7a0 == 0) && !(this->field_0x7a8)) && ((uint8_t)this->field_0x784 == '\0')) && ((this->field_0x780 == 0) && (this->field_0x764 < 1))))
    {
        uVar1 = 1;
    }
    return uVar1;
}
#else
uint32_t WitherBoss::canShoot()
{
    return (uint32_t)(guest::stub(0x02AAFE48u, gaddr(this)) >> 32);
}
#endif

/* aiStep() [0x02AAFE94] - boundary stub; body not reconstructed */
uint64_t WitherBoss::aiStep()
{
    return (uint64_t)guest::stub(0x02AAFE94u, gaddr(this));
}

/* makeInvulnerable() [0x02AB1960] - LOWERED-DRAFT; not reconstructed */
#if defined(RECON_RUNTIME_BUILD)
uint32_t WitherBoss::makeInvulnerable()
{
    return (uint32_t)(guest::stub(0x02AB1960u, gaddr(this)) >> 32);
}
#else
uint32_t WitherBoss::makeInvulnerable()
{
    int32_t iVar1 = 0;
    this->field_0x1f0 = 200;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar1))->set__tm__2_i(global_0x104C5238, (int32_t)(200), (bool)(1));
    return;
}
#endif

/* makeStuckInWeb() [0x02AB1978] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB1978
void WitherBoss::makeStuckInWeb()
{
    return;
}
#else
void WitherBoss::makeStuckInWeb()
{
    guest::stub(0x02AB1978u, gaddr(this));
}
#endif

/* startSeenByPlayer(boost::shared_ptr<ServerPlayer>) [0x02AB197C] - LOWERED-DRAFT; not reconstructed (4 open edges, 47 raw IR ops, 13 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherBoss::startSeenByPlayer(gptr<sp_ServerPlayer> a1)
{
    guest::stub(0x02AB197Cu, gaddr(this), (uint32_t)a1);
}
#else
void WitherBoss::startSeenByPlayer(gptr<sp_ServerPlayer> a1)
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t piVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t piVar5 = 0;
    uint8_t bVar6 = 0;
    ST32(_frame, a1->field_0x0);
    ST32((_frame + 4), (uint32_t)(a1->field_0x4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    while (!(bVar1))
    {
        if (('\x01' != '\0'))
        {
            iVar4 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
            ST32(LD32((_frame + 4)), iVar4);
            bVar1 = 1;
        }
    }
    this->Entity::startSeenByPlayer((gptr<sp_ServerPlayer>)(_frame));
    piVar3 = LD32((_frame + 4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar4 = (LD32(LD32((_frame + 4))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar4), 1);
                ST32(LD32((_frame + 4)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar6 = ((iVar4 == 0) << 1);
        if ((iVar4 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            piVar5 = (piVar3 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar5), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar6 = 2;
                }
            }
            while (!((uint8_t)(((bVar6 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) + 0x1c)), (uint32_t)(piVar3)) >> 32);
            }
        }
    }
    ST32(_frame, a1->field_0x0);
    ST32((_frame + 4), (uint32_t)(a1->field_0x4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    while (!(bVar1))
    {
        if (('\x01' != '\0'))
        {
            iVar4 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
            ST32(LD32((_frame + 4)), iVar4);
            bVar1 = 1;
        }
    }
    this->field_0x7f0->addPlayer((gptr<sp_ServerPlayer>)(_frame), (int32_t)(0));
    piVar3 = LD32((_frame + 4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar4 = (LD32(LD32((_frame + 4))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar4), 1);
                ST32(LD32((_frame + 4)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar6 = ((iVar4 == 0) << 1);
        if ((iVar4 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            piVar5 = (piVar3 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar5), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar6 = 2;
                }
            }
            while (!((uint8_t)(((bVar6 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) + 0x1c)), (uint32_t)(piVar3)) >> 32);
            }
        }
    }
    native_frame_close(_frame, 8);
    return;
    native_frame_close(_frame, 8);
}
#endif

/* stopSeenByPlayer(boost::shared_ptr<ServerPlayer>) [0x02AB1B4C] - LOWERED-DRAFT; not reconstructed (4 open edges, 47 raw IR ops, 13 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherBoss::stopSeenByPlayer(gptr<sp_ServerPlayer> a1)
{
    guest::stub(0x02AB1B4Cu, gaddr(this), (uint32_t)a1);
}
#else
void WitherBoss::stopSeenByPlayer(gptr<sp_ServerPlayer> a1)
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t piVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t piVar5 = 0;
    uint8_t bVar6 = 0;
    ST32(_frame, a1->field_0x0);
    ST32((_frame + 4), (uint32_t)(a1->field_0x4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    while (!(bVar1))
    {
        if (('\x01' != '\0'))
        {
            iVar4 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
            ST32(LD32((_frame + 4)), iVar4);
            bVar1 = 1;
        }
    }
    this->Entity::stopSeenByPlayer((gptr<sp_ServerPlayer>)(_frame));
    piVar3 = LD32((_frame + 4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar4 = (LD32(LD32((_frame + 4))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar4), 1);
                ST32(LD32((_frame + 4)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar6 = ((iVar4 == 0) << 1);
        if ((iVar4 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            piVar5 = (piVar3 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar5), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar6 = 2;
                }
            }
            while (!((uint8_t)(((bVar6 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) + 0x1c)), (uint32_t)(piVar3)) >> 32);
            }
        }
    }
    ST32(_frame, a1->field_0x0);
    ST32((_frame + 4), (uint32_t)(a1->field_0x4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    while (!(bVar1))
    {
        if (('\x01' != '\0'))
        {
            iVar4 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
            ST32(LD32((_frame + 4)), iVar4);
            bVar1 = 1;
        }
    }
    this->field_0x7f0->removePlayer((gptr<sp_ServerPlayer>)(_frame));
    piVar3 = LD32((_frame + 4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar4 = (LD32(LD32((_frame + 4))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar4), 1);
                ST32(LD32((_frame + 4)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar6 = ((iVar4 == 0) << 1);
        if ((iVar4 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            piVar5 = (piVar3 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar5), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar6 = 2;
                }
            }
            while (!((uint8_t)(((bVar6 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) + 0x1c)), (uint32_t)(piVar3)) >> 32);
            }
        }
    }
    native_frame_close(_frame, 8);
    return;
    native_frame_close(_frame, 8);
}
#endif

/* getPlayerParty() [0x02AB1D18] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB1D18
void WitherBoss::getPlayerParty()
{
    this->field_0x7f0->getPartyPlayers();
    return;
}
#else
void WitherBoss::getPlayerParty()
{
    guest::stub(0x02AB1D18u, gaddr(this));
}
#endif

/* getArmorValue() [0x02AB1D2C] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB1D2C
uint32_t WitherBoss::getArmorValue()
{
    return 4;
}
#else
uint32_t WitherBoss::getArmorValue()
{
    return (uint32_t)(guest::stub(0x02AB1D2Cu, gaddr(this)) >> 32);
}
#endif

/* changePhase() [0x02AB1D34] - LOWERED-DRAFT; not reconstructed (1 open edges, 1 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherBoss::changePhase()
{
    guest::stub(0x02AB1D34u, gaddr(this));
}
#else
void WitherBoss::changePhase()
{
    uint32_t pcVar1 = 0;
    double dVar2 = 0;
    pcVar1 = LD32((uint32_t)((this->field_0x348 + 0x9fc)));
    this->field_0x760 = (this->field_0x760 + -(1));
    dVar2 = (double)((uint32_t)(guest::indirect((uint32_t)(pcVar1)) >> 32));
    this->field_0x7c4 = (this->field_0x7c4 - ((int32_t)(dVar2) - this->field_0x7e4));
    this->setAerialAttack((bool)(0));
    this->field_0x79c = 1;
    this->field_0x78c = 0x14;
    this->field_0x7a8 = 0;
    this->field_0x7bc = (this->field_0x7bc << 1);
    return;
}
#endif

/* hurt(const DamageSource*, float) [0x02AB1DD8] - boundary stub; body not reconstructed */
uint32_t WitherBoss::hurt(double a1, float a2)
{
    return (uint32_t)(guest::stub(0x02AB1DD8u, gaddr(this), (uint32_t)a2, (double)a1) >> 32);
}

/* getLightColor() [0x02AB3274] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB3274
uint32_t WitherBoss::getLightColor()
{
    return 0xf000f0;
}
#else
uint32_t WitherBoss::getLightColor()
{
    return (uint32_t)(guest::stub(0x02AB3274u, gaddr(this)) >> 32);
}
#endif

/* isPickable() [0x02AB3280] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB3280
uint8_t WitherBoss::isPickable()
{
    return ((uint8_t)this->field_0x197 ^ 1);
}
#else
uint8_t WitherBoss::isPickable()
{
    return (uint8_t)(guest::stub(0x02AB3280u, gaddr(this)) >> 32);
}
#endif

/* causeFallDamage(float, float) [0x02AB3290] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB3290
void WitherBoss::causeFallDamage(float a1, float a2)
{
    return;
}
#else
void WitherBoss::causeFallDamage(float a1, float a2)
{
    guest::stub(0x02AB3290u, gaddr(this), (double)a1, (double)a2);
}
#endif

/* addEffect(MobEffectInstance*, const boost::shared_ptr<Entity>&) [0x02AB3294] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB3294
void WitherBoss::addEffect(gptr<MobEffectInstance> a1, gptr<sp_Entity> a2)
{
    return;
}
#else
void WitherBoss::addEffect(gptr<MobEffectInstance> a1, gptr<sp_Entity> a2)
{
    guest::stub(0x02AB3294u, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}
#endif

/* setAlternativeTarget(int, int) [0x02AB3298] - boundary stub; body not reconstructed */
uint64_t WitherBoss::setAlternativeTarget(int32_t a1, int32_t a2)
{
    return (uint64_t)guest::stub(0x02AB3298u, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}

/* getHeadYRot(int) [0x02AB3490] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB3490
double WitherBoss::getHeadYRot(int32_t a1)
{
    return (double)(gelem<float>(gaddr(&this->field_0x744), a1));
}
#else
double WitherBoss::getHeadYRot(int32_t a1)
{
    guest::stub(0x02AB3490u, gaddr(this), (uint32_t)a1);
    return native_fret();
}
#endif

/* getHeadXRot(int) [0x02AB34A0] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB34A0
double WitherBoss::getHeadXRot(int32_t a1)
{
    return (double)(gelem<float>(gaddr(&this->field_0x73c), a1));
}
#else
double WitherBoss::getHeadXRot(int32_t a1)
{
    guest::stub(0x02AB34A0u, gaddr(this), (uint32_t)a1);
    return native_fret();
}
#endif

/* getMobType() [0x02AB34B0] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB34B0
uint32_t WitherBoss::getMobType()
{
    return 1;
}
#else
uint32_t WitherBoss::getMobType()
{
    return (uint32_t)(guest::stub(0x02AB34B0u, gaddr(this)) >> 32);
}
#endif

/* die(const DamageSource*) [0x02AB34B8] - LOWERED-DRAFT; not reconstructed (4 open edges, 8 raw IR ops, 4 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherBoss::die(gptr<DamageSource> a1)
{
    guest::stub(0x02AB34B8u, gaddr(this), (uint32_t)a1);
}
#else
void WitherBoss::die(gptr<DamageSource> a1)
{
    uint32_t _frame = native_frame_open(16);
    uint8_t bVar1 = 0;
    int32_t iVar3 = 0;
    uint32_t uVar4 = 0;
    uint32_t pcVar5 = 0;
    int32_t iVar8 = 0;
    pcVar5 = LD32((uint32_t)((a1->field_0x18 + 0x44)));
    this->field_0x76c = 200;
    iVar3 = (uint32_t)(guest::indirect((uint32_t)(pcVar5), (uint32_t)(a1)) >> 32);
    iVar8 = this->field_0x348;
    pcVar5 = LD32((uint32_t)((iVar8 + 0x98c)));
    this->field_0x780 = 0;
    this->field_0x75c = iVar3;
    this->field_0x784 = 0;
    this->field_0x768 = 0;
    uVar4 = (uint32_t)(guest::indirect((uint32_t)(pcVar5), (uint32_t)(gaddr(this))) >> 32);
    (uint32_t)(guest::indirect(LD32((iVar8 + 0xf4)), (double)((double)(global_0x102B435C)), (double)((double)(global_0x102B1E18)), (uint32_t)(gaddr(this)), (uint32_t)(uVar4)) >> 32);
    iVar3 = LD32((uint32_t)(this->field_0xf8));
    ST32((_frame + 4), this->field_0x0);
    if (((uint32_t)(LD32((_frame + 4))) != gaddr(this)))
    {
    }
    if (!(bVar1))
    {
        do
        {
            if (('\x01' != '\0'))
            {
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        if (!(bVar1))
        {
            do
            {
                if (('\x01' != '\0'))
                {
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
        }
    }
    (uint32_t)(guest::indirect(LD32((iVar3 + 0x2c4)), (uint32_t)(this->field_0xf8), (uint32_t)((_frame + 4)), (uint32_t)(3), (uint32_t)(0)) >> 32);
    if (!(bVar1))
    {
        do
        {
            if (('\x01' != '\0'))
            {
                bVar1 = 1;
            }
        }
        while (!(bVar1));
    }
    native_frame_close(_frame, 16);
    return;
    native_frame_close(_frame, 16);
}
#endif

/* handleEntityEvent(unsigned char, int) [0x02AB3708] - boundary stub; body not reconstructed */
void WitherBoss::handleEntityEvent(uint8_t a1, int32_t a2)
{
    guest::stub(0x02AB3708u, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}

/* getOverlayAlpha() [0x02AB372C] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB372C
double WitherBoss::getOverlayAlpha()
{
    return (double)(this->field_0x774);
}
#else
double WitherBoss::getOverlayAlpha()
{
    guest::stub(0x02AB372Cu, gaddr(this));
    return native_fret();
}
#endif

/* getSwellAmount(float) const [0x02AB3734] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB3734
double WitherBoss::getSwellAmount(float a1)
{
    return (double)(((float)((((double)((float)(((double)(this->field_0x778) - (double)(this->field_0x77c)))) * a1) + (double)(this->field_0x77c))) / 28.0f));
}
#else
double WitherBoss::getSwellAmount(float a1)
{
    guest::stub(0x02AB3734u, gaddr(this), (double)a1);
    return native_fret();
}
#endif

/* wantsToMove() [0x02AB3754] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB3754
uint8_t WitherBoss::wantsToMove()
{
    return (uint8_t)this->field_0x7a8;
}
#else
uint8_t WitherBoss::wantsToMove()
{
    return (uint8_t)(guest::stub(0x02AB3754u, gaddr(this)) >> 32);
}
#endif

/* setWantsToMove(bool) [0x02AB375C] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB375C
void WitherBoss::setWantsToMove(bool a1)
{
    this->field_0x7a8 = a1;
    return;
}
#else
void WitherBoss::setWantsToMove(bool a1)
{
    guest::stub(0x02AB375Cu, gaddr(this), (uint32_t)a1);
}
#endif

/* setIsPathing(bool) [0x02AB3764] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB3764
void WitherBoss::setIsPathing(bool a1)
{
    this->field_0x7ec = a1;
    return;
}
#else
void WitherBoss::setIsPathing(bool a1)
{
    guest::stub(0x02AB3764u, gaddr(this), (uint32_t)a1);
}
#endif

/* setShotDelay(int) [0x02AB376C] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB376C
void WitherBoss::setShotDelay(int32_t a1)
{
    this->field_0x764 = a1;
    return;
}
#else
void WitherBoss::setShotDelay(int32_t a1)
{
    guest::stub(0x02AB376Cu, gaddr(this), (uint32_t)a1);
}
#endif

/* heal(float) [0x02AB3774] - LOWERED-DRAFT; not reconstructed (2 raw IR ops, 2 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherBoss::heal(float a1)
{
    guest::stub(0x02AB3774u, gaddr(this), (double)a1);
}
#else
void WitherBoss::heal(float a1)
{
    uint32_t _frame = native_frame_open(32);
    double dVar1 = 0;
    dVar1 = (double)(this->virt_getHealth());
    if ((((double)(global_0x102B5164) < dVar1) && (LD8((uint32_t)((this->field_0xf8 + 0x168))) == '\0')))
    {
        this->LivingEntity::heal((float)(a1));
    }
    native_frame_close(_frame, 32);
    return;
    native_frame_close(_frame, 32);
}
#endif

/* canFly() [0x02AB3800] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB3800
uint32_t WitherBoss::canFly()
{
    return 1;
}
#else
uint32_t WitherBoss::canFly()
{
    return (uint32_t)(guest::stub(0x02AB3800u, gaddr(this)) >> 32);
}
#endif

/* getTarget() [0x02AB3808] - LOWERED-DRAFT; not reconstructed (7 open edges, 43 raw IR ops, 14 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t WitherBoss::getTarget()
{
    return (uint32_t)(guest::stub(0x02AB3808u, gaddr(this)) >> 32);
}
#else
uint32_t WitherBoss::getTarget(uint32_t a1)
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    uint32_t uVar4 = 0;
    uint32_t piVar5 = 0;
    uint32_t pcVar6 = 0;
    uint8_t bVar7 = 0;
    uint64_t __pair = 0;
    if ((LD8((uint32_t)((this->field_0xf8 + 0x168))) != '\0'))
    {
        iVar2 = this->getAlternativeTarget((int32_t)(0));
        iVar3 = this->Entity::getId();
        if ((iVar2 != iVar3))
        {
            piVar5 = LD32((uint32_t)((gaddr(this) + 0xf8)));
            pcVar6 = LD32((uint32_t)((gelem<int32_t>((uint32_t)(piVar5), 0) + 0x29c)));
            uVar4 = this->getAlternativeTarget((int32_t)(0));
            (uint32_t)((__pair = (uint64_t)(guest::indirect((uint32_t)(pcVar6), (uint32_t)(piVar5), (uint32_t)(_frame), (uint32_t)(uVar4)))) >> 32);
            if (((LD32(_frame) != 0) && ((uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((LD32(_frame) + 0x348))) + 0xc)), (uint32_t)(LD32(_frame))))) >> 32), ((((uint32_t)__pair) & 0xc000000) == 0xc000000))))
            {
                ((boost *)gmem(_frame))->static_pointer_cast__tm__22_12LivingEntity6Entity(a1);
                if ((LD32((_frame + 4)) == (uint32_t)(0x0)))
                {
                    { uint64_t t1 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
                    native_frame_close(_frame, 8);
                    return t1; }
                }
                ((void)0 /* sync dropped */);
                bVar1 = 0;
                do
                {
                    iVar2 = (LD32(LD32((_frame + 4))) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar3 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar2), 1);
                        ST32(LD32((_frame + 4)), iVar3);
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                ((void)0 /* instructionSynchronize dropped */);
                bVar7 = ((iVar2 == 0) << 1);
                if ((iVar2 != 0))
                {
                    { uint64_t t2 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
                    native_frame_close(_frame, 8);
                    return t2; }
                }
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                piVar5 = (LD32((_frame + 4)) + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar2 = gelem<int32_t>((uint32_t)(piVar5), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar3 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar2 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar5), 0) = iVar3;
                        bVar7 = 2;
                    }
                }
                while (!((uint8_t)(((bVar7 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar2 + -(1)) != 0))
                {
                    { uint64_t t3 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
                    native_frame_close(_frame, 8);
                    return t3; }
                }
                { uint64_t t4 = ((uint64_t)(uint32_t)((uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 4)))))) >> 32)) << 32);
                native_frame_close(_frame, 8);
                return t4; }
            }
            bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
            if (!(bVar1))
            {
                ((void)0 /* sync dropped */);
                do
                {
                    iVar2 = (LD32(LD32((_frame + 4))) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar3 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar2), 1);
                        ST32(LD32((_frame + 4)), iVar3);
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                ((void)0 /* instructionSynchronize dropped */);
                bVar7 = ((iVar2 == 0) << 1);
                if ((iVar2 == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                    piVar5 = (LD32((_frame + 4)) + (1) * 4);
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar2 = gelem<int32_t>((uint32_t)(piVar5), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar3 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar2 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar5), 0) = iVar3;
                            bVar7 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar7 >> 1) & 1))));
                    ((void)0 /* instructionSynchronize dropped */);
                    if (((iVar2 + -(1)) == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                    }
                }
            }
        }
    }
    { uint64_t t5 = (__pair = (uint64_t)(this->Mob::getTarget(a1)));
    native_frame_close(_frame, 8);
    return t5; }
    native_frame_close(_frame, 8);
}
#endif

/* IsSpawning() const [0x02AB3A54] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB3A54
uint32_t WitherBoss::IsSpawning()
{
    return ((-(this->field_0x798) & ~(this->field_0x798)) >> 0x1f);
}
#else
uint32_t WitherBoss::IsSpawning()
{
    return (uint32_t)(guest::stub(0x02AB3A54u, gaddr(this)) >> 32);
}
#endif

/* finalizeMobSpawn(const DifficultyInstance&, MobGroupData*, const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) [0x02AB3A68] - LOWERED-DRAFT; not reconstructed (15 open edges, 15 raw IR ops, 12 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gptr<MobGroupData> WitherBoss::finalizeMobSpawn(gptr<DifficultyInstance> a1, gptr<MobGroupData> a2, gptr<wstring> a3)
{
    return (gptr<MobGroupData>)(guest::stub(0x02AB3A68u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3) >> 32);
}
#else
gptr<MobGroupData> WitherBoss::finalizeMobSpawn(gptr<DifficultyInstance> a1, gptr<MobGroupData> a2, gptr<wstring> a3)
{
    uint32_t _frame = native_frame_open(32);
    gptr<MobGroupData> data = 0u;
    uint32_t piVar2 = 0;
    int32_t iVar3 = 0;
    uint32_t uVar4 = 0;
    uint32_t pcVar5 = 0;
    double dVar6 = 0;
    double dVar7 = 0;
    data = guest::Mob_finalizeMobSpawn(gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
    dVar7 = (double)(global_0x102B1E18);
    (uint32_t)(guest::indirect(LD32((this->field_0x348 + 0x4c)), (double)(dVar7), (double)((double)(global_0x102C0D60)), (uint32_t)(gaddr(this))) >> 32);
    this->setAerialAttack((bool)(1));
    pcVar5 = LD32((uint32_t)((this->field_0x348 + 0xa24)));
    this->field_0x798 = 200;
    piVar2 = (uint32_t)((uint32_t)(guest::indirect((uint32_t)(pcVar5), (uint32_t)(gaddr(this)), (uint32_t)(global_0x104EFE1C)) >> 32));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102C0C90)) >> 32);
    piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE14)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102C0C98)) >> 32);
    (uint32_t)(guest::indirect(LD32((this->field_0x348 + 0xe0c)), (uint32_t)(gaddr(this))) >> 32);
    iVar3 = (uint32_t)((this->field_0xf8->getDifficulty()) >> 32);
    if ((iVar3 == global_0x104DCB24))
    {
        piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE10)));
        pcVar5 = LD32((uint32_t)((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)));
        dVar6 = (double)(this->virt_getMaxHealth());
        (uint32_t)(guest::indirect((uint32_t)(pcVar5), (double)((dVar6 * 0.5)), (uint32_t)(piVar2)) >> 32);
        pcVar5 = LD32((uint32_t)((this->field_0x348 + 0x9fc)));
        uVar4 = this->field_0x738;
        this->field_0x738 = (((int32_t)(uVar4) >> 1) + (uint32_t)((((int32_t)(uVar4) < 0) && ((uVar4 & 1) != 0))));
        dVar6 = (double)((uint32_t)(guest::indirect((uint32_t)(pcVar5), (uint32_t)(gaddr(this))) >> 32));
        pcVar5 = LD32((uint32_t)((this->field_0x348 + 0x934)));
        this->field_0x7c4 = (int32_t)(dVar6);
        (uint32_t)(guest::indirect((uint32_t)(pcVar5), (double)(dVar7), (uint32_t)(gaddr(this))) >> 32);
        dVar7 = (double)(this->virt_getMaxHealth());
        pcVar5 = LD32((uint32_t)((this->field_0x348 + 0x9fc)));
        this->field_0x7c8 = (int32_t)(dVar7);
        dVar7 = (double)((uint32_t)(guest::indirect((uint32_t)(pcVar5), (uint32_t)(gaddr(this))) >> 32));
        uVar4 = (uint32_t)(dVar7);
        pcVar5 = LD32((uint32_t)((this->field_0x348 + 0x9fc)));
        this->field_0x7e4 = (((int32_t)(uVar4) >> 1) + (uint32_t)((((int32_t)(uVar4) < 0) && ((uVar4 & 1) != 0))));
        dVar7 = (double)((uint32_t)(guest::indirect((uint32_t)(pcVar5), (uint32_t)(gaddr(this))) >> 32));
        this->field_0x7e8 = ((int32_t)(dVar7) / 6);
    }
    else
    {
        iVar3 = (uint32_t)((this->field_0xf8->getDifficulty()) >> 32);
        if ((iVar3 == global_0x104DCB28))
        {
            piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE10)));
            pcVar5 = LD32((uint32_t)((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)));
            dVar6 = (double)(this->virt_getMaxHealth());
            (uint32_t)(guest::indirect((uint32_t)(pcVar5), (double)((dVar6 * 0.75)), (uint32_t)(piVar2)) >> 32);
            this->field_0x738 = (int32_t)(((float)(((double)(int32_t)(this->field_0x738))) * 0.75f));
        }
        dVar6 = (double)(this->virt_getMaxHealth());
        pcVar5 = LD32((uint32_t)((this->field_0x348 + 0x934)));
        this->field_0x7c4 = (int32_t)(dVar6);
        (uint32_t)(guest::indirect((uint32_t)(pcVar5), (double)(dVar7), (uint32_t)(gaddr(this))) >> 32);
        dVar7 = (double)(this->virt_getMaxHealth());
        pcVar5 = LD32((uint32_t)((this->field_0x348 + 0x9fc)));
        this->field_0x7c8 = (int32_t)(dVar7);
        dVar7 = (double)((uint32_t)(guest::indirect((uint32_t)(pcVar5), (uint32_t)(gaddr(this))) >> 32));
        uVar4 = (uint32_t)(dVar7);
        pcVar5 = LD32((uint32_t)((this->field_0x348 + 0x9fc)));
        this->field_0x7e4 = (((int32_t)(uVar4) >> 1) + (uint32_t)((((int32_t)(uVar4) < 0) && ((uVar4 & 1) != 0))));
        dVar7 = (double)((uint32_t)(guest::indirect((uint32_t)(pcVar5), (uint32_t)(gaddr(this))) >> 32));
        this->field_0x7e8 = ((int32_t)(dVar7) / 6);
    }
    { uint32_t t1 = data;
    native_frame_close(_frame, 32);
    return t1; }
    native_frame_close(_frame, 32);
}
#endif

/* createNavigation(Level*) [0x02AB3E8C] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
gptr<FlyingPathNavigation> WitherBoss::createNavigation(gptr<Level> a1)
{
    return (gptr<FlyingPathNavigation>)(guest::stub(0x02AB3E8Cu, gaddr(this), (uint32_t)a1) >> 32);
}
#else
gptr<FlyingPathNavigation> WitherBoss::createNavigation(gptr<Level> a1)
{
    gptr<FlyingPathNavigation> navigation = 0u;
    navigation = guest::FlyingPathNavigation_ct((uint32_t)(0), (uint32_t)(gaddr(this)), (uint32_t)(a1));
    navigation->setCanOpenDoors((bool)(0));
    navigation->setCanFloat((bool)(1));
    navigation->setCanPassDoors((bool)(1));
    return navigation;
}
#endif

/* registerGoals() [0x02AB4114] - boundary stub; body not reconstructed */
void WitherBoss::registerGoals()
{
    guest::stub(0x02AB4114u, gaddr(this));
}

/* getAmbientSound() [0x02AB43AC] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB43AC
uint32_t WitherBoss::getAmbientSound()
{
    return global_0x104C46F4;
}
#else
uint32_t WitherBoss::getAmbientSound()
{
    return (uint32_t)(guest::stub(0x02AB43ACu, gaddr(this)) >> 32);
}
#endif

/* getHurtSound(const DamageSource*) [0x02AB43B8] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB43B8
uint32_t WitherBoss::getHurtSound(gptr<DamageSource> a1)
{
    return global_0x104C4700;
}
#else
uint32_t WitherBoss::getHurtSound(gptr<DamageSource> a1)
{
    return (uint32_t)(guest::stub(0x02AB43B8u, gaddr(this), (uint32_t)a1) >> 32);
}
#endif

/* getDeathSound() [0x02AB43C4] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB43C4
uint32_t WitherBoss::getDeathSound()
{
    return global_0x104C46FC;
}
#else
uint32_t WitherBoss::getDeathSound()
{
    return (uint32_t)(guest::stub(0x02AB43C4u, gaddr(this)) >> 32);
}
#endif

/* destroyBlocks(Level*, const AABB*, int) [0x02AB4518] - boundary stub; body not reconstructed */
uint64_t WitherBoss::destroyBlocks(gptr<Level> a1, gptr<AABB> a2, int32_t a3)
{
    return (uint64_t)guest::stub(0x02AB4518u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
}

/* customServerAiStep() [0x02AB47B8] - boundary stub; body not reconstructed */
void WitherBoss::customServerAiStep()
{
    guest::stub(0x02AB47B8u, gaddr(this));
}

/* dropDeathLoot(bool, int) [0x02AB6DCC] - boundary stub; body not reconstructed */
void WitherBoss::dropDeathLoot(bool a1, int32_t a2)
{
    guest::stub(0x02AB6DCCu, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}

/* checkDespawn() [0x02AB70A0] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB70A0
void WitherBoss::checkDespawn()
{
    this->field_0x484 = 0;
    return;
}
#else
void WitherBoss::checkDespawn()
{
    guest::stub(0x02AB70A0u, gaddr(this));
}
#endif

/* canRide(boost::shared_ptr<Entity>) [0x02AB70AC] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB70AC
uint32_t WitherBoss::canRide(gptr<sp_Entity> a1)
{
    return 0;
}
#else
uint32_t WitherBoss::canRide(gptr<sp_Entity> a1)
{
    return (uint32_t)(guest::stub(0x02AB70ACu, gaddr(this), (uint32_t)a1) >> 32);
}
#endif

/* canChangeDimensions() [0x02AB70B4] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB70B4
uint32_t WitherBoss::canChangeDimensions()
{
    return 0;
}
#else
uint32_t WitherBoss::canChangeDimensions()
{
    return (uint32_t)(guest::stub(0x02AB70B4u, gaddr(this)) >> 32);
}
#endif

/* getHeadX(int) [0x02AB70BC] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB70BC
double WitherBoss::getHeadX(int32_t a1)
{
    double dVar1 = 0;
    if ((a1 < 1))
    {
        return this->field_0x118;
    }
    dVar1 = (double)(Mth::cos(((double)(((this->field_0x3e0 + (float)(((double)(int32_t)(((a1 * 0xb4) - 0xb4U))))) * gref<float>(0x101A464Cu))))));
    return ((dVar1 * 1.3) + this->field_0x118);
}
#else
double WitherBoss::getHeadX(int32_t a1)
{
    guest::stub(0x02AB70BCu, gaddr(this), (uint32_t)a1);
    return native_fret();
}
#endif

/* getHeadY(int) [0x02AB7164] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB7164
double WitherBoss::getHeadY(int32_t a1)
{
    if ((a1 < 1))
    {
        return (this->field_0x120 + 3.0);
    }
    return (this->field_0x120 + 2.2);
}
#else
double WitherBoss::getHeadY(int32_t a1)
{
    guest::stub(0x02AB7164u, gaddr(this), (uint32_t)a1);
    return native_fret();
}
#endif

/* getHeadZ(int) [0x02AB7190] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB7190
double WitherBoss::getHeadZ(int32_t a1)
{
    double dVar1 = 0;
    if ((a1 < 1))
    {
        return this->field_0x128;
    }
    dVar1 = (double)(Mth::sin(((double)(((this->field_0x3e0 + (float)(((double)(int32_t)(((a1 * 0xb4) - 0xb4U))))) * gref<float>(0x101A464Cu))))));
    return ((dVar1 * 1.3) + this->field_0x128);
}
#else
double WitherBoss::getHeadZ(int32_t a1)
{
    guest::stub(0x02AB7190u, gaddr(this), (uint32_t)a1);
    return native_fret();
}
#endif

/* rotlerp(float, float, float) [0x02AB7238] - boundary stub; body not reconstructed */
double WitherBoss::rotlerp(float a1, float a2, float a3)
{
    guest::stub(0x02AB7238u, gaddr(this), (double)a1, (double)a2, (double)a3);
    return native_fret();
}

/* performRangedAttack(int, boost::shared_ptr<LivingEntity>) [0x02AB72EC] - boundary stub; body not reconstructed */
uint64_t WitherBoss::performRangedAttack(int32_t a1, gptr<sp_LivingEntity> a2)
{
    return (uint64_t)guest::stub(0x02AB72ECu, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}

/* performRangedAttack(int, double, double, double, bool) [0x02AB7600] - boundary stub; body not reconstructed */
uint64_t WitherBoss::performRangedAttack(int32_t a1, double a2, double a3, double a4, bool a5)
{
    return (uint64_t)guest::stub(0x02AB7600u, gaddr(this), (uint32_t)a4, (uint32_t)a5, (double)a1, (double)a2, (double)a3);
}

/* getMaxHealth() [0x02ACDC00] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02ACDC00
void WitherBoss::getMaxHealth()
{
    this->LivingEntity::getMaxHealth();
    return;
}
#else
void WitherBoss::getMaxHealth()
{
    guest::stub(0x02ACDC00u, gaddr(this));
}
#endif

/* getHealth() [0x02ACDC10] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02ACDC10
void WitherBoss::getHealth()
{
    this->LivingEntity::getHealth();
    return;
}
#else
void WitherBoss::getHealth()
{
    guest::stub(0x02ACDC10u, gaddr(this));
}
#endif

/* getDimension() [0x02ACDC20] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02ACDC20
uint32_t WitherBoss::getDimension()
{
    return this->field_0x240;
}
#else
uint32_t WitherBoss::getDimension()
{
    return (uint32_t)(guest::stub(0x02ACDC20u, gaddr(this)) >> 32);
}
#endif

/* GetType() [0x02AD1D98] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AD1D98
uint64_t WitherBoss::GetType()
{
    return 0x5d54004700000000;
}
#else
uint64_t WitherBoss::GetType()
{
    return (uint64_t)guest::stub(0x02AD1D98u, gaddr(this));
}
#endif

}  /* namespace recon */
\n#include "game/W/WitherBoss__WitherDoNothingGoal.hpp"
#include "game/G/Goal.hpp"
#include "game/W/WitherBoss.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(WitherBoss*) [0x02AB3F18] - reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WitherBoss_WitherDoNothingGoal::WitherBoss_WitherDoNothingGoal(gptr<WitherBoss> a1)
{
    guest::stub(0x02AB3F18u, gaddr(this), (uint32_t)a1);
}
#else
WitherBoss_WitherDoNothingGoal::WitherBoss_WitherDoNothingGoal(gptr<WitherBoss> a1) : Goal()
{
    this->field_0x8 = a1;
    this->Goal::setRequiredControlFlags(7);
}
#endif

/* {dtor}() [0x02AD1E40] - reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WitherBoss_WitherDoNothingGoal::~WitherBoss_WitherDoNothingGoal()
{
    guest::stub(0x02AD1E40u, gaddr(this), 0u);
}
#else
WitherBoss_WitherDoNothingGoal::~WitherBoss_WitherDoNothingGoal()
{
    if ((gaddr(this) == 0))
    {
        return;
    }
}
#endif

/* canUse() [0x02AB7F44] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02AB7F44
uint32_t WitherBoss_WitherDoNothingGoal::canUse()
{
    uint32_t uVar1 = 0;
    uVar1 = this->field_0x8->getInvulnerableTicks();
    return ((-(uVar1) & ~(uVar1)) >> 0x1f);
}
#else
uint32_t WitherBoss_WitherDoNothingGoal::canUse()
{
    return (uint32_t)(guest::stub(0x02AB7F44u, gaddr(this)) >> 32);
}
#endif

/* getName() [0x02AD1DA8] - LOWERED-DRAFT; not reconstructed (3 open edges, 3 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gresult<wstring> WitherBoss_WitherDoNothingGoal::getName()
{
    gresult<wstring> result;
    guest::stub(0x02AD1DA8u, gaddr(this), (uint32_t)result);
    return result;
}
#else
gresult<wstring> WitherBoss_WitherDoNothingGoal::getName(int32_t a1)
{
    uint32_t sVar1 = 0;
    if (((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0x20)), (a1 != 0))))
    {
        ST32((uint32_t)((a1 + 0x1c)), 7);
        ST32((uint32_t)((a1 + 0x18)), 0);
        ST16((uint32_t)((a1 + 4)), 0);
        sVar1 = ::recon::wcslen(GSTR("WitherDoNothing", 0x102ABAF4));
        ((wstring *)gmem(a1))->assign(GSTR("WitherDoNothing", 0x102ABAF4), (uint32_t)(sVar1));
    }
    return;
}
#endif

}  /* namespace recon */
