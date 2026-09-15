#include "game/V/Vex.hpp"
#include "game/B/BlockPos.hpp"
#include "game/C/CompoundTag.hpp"
#include "game/D/DamageSource.hpp"
#include "game/D/DifficultyInstance.hpp"
#include "game/E/Entity.hpp"
#include "game/G/GoalSelector.hpp"
#include "game/L/Level.hpp"
#include "game/L/LivingEntity.hpp"
#include "game/M/Mob.hpp"
#include "game/M/MobGroupData.hpp"
#include "game/M/Monster.hpp"
#include "game/S/SynchedEntityData.hpp"
#include "game/V/Vex__VexMoveControl.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(Level*) [0x02A4454C] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Vex::Vex(gptr<Level> a1)
{
    guest::stub(0x02A4454Cu, gaddr(this), (uint32_t)a1);
}
#else
Vex::Vex(gptr<Level> a1)
{
    uint32_t puVar1 = 0;
    int32_t iVar2 = 0;
    gptr<Vex_VexMoveControl> control = 0u;
    double dVar4 = 0;
    guest::Monster_ct((uint32_t)(gaddr(this)), (uint32_t)(a1));
    puVar1 = (uint32_t)((gaddr(this) + 0x730));
    ST32((uint32_t)((gaddr(this) + 0x348)), GSTR(L"", 0x102C60A4));
    ST32((uint32_t)((gaddr(this) + 8)), 0x102C6EA4u);
    if (((puVar1 != (uint32_t)(0x0)) || (puVar1 = (uint32_t)(guest::guest_new((uint32_t)(8))), (puVar1 != (uint32_t)(0x0)))))
    {
        gelem<uint32_t>((uint32_t)(puVar1) + 1u, 0) = 0;
        gelem<uint32_t>((uint32_t)(puVar1), 0) = 0;
    }
    this->Mob::fjDerivedCtorCalls();
    iVar2 = this->field_0x5fc;
    this->field_0x1f5 = 1;
    if ((iVar2 != 0))
    {
        (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar2 + 0x34))) + 0xc)), (uint32_t)(iVar2), (uint32_t)(3)) >> 32);
    }
    control = guest::Vex_VexMoveControl_ct((uint32_t)(0), (uint32_t)(gaddr(this)));
    dVar4 = (double)(global_0x102B1E1C);
    this->field_0x5fc = control;
    this->Entity::setSize((float)(dVar4), (float)((double)(global_0x102B1E20)));
    this->field_0x73c = 0;
    this->field_0x738 = 0;
    this->field_0x5f4 = 3;
    this->field_0x740 = 0;
}
#endif

/* {dtor}() [0x02A1B744] - LOWERED-DRAFT; not reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Vex::~Vex()
{
    guest::stub(0x02A1B744u, gaddr(this), 0u);
}
#else
Vex::~Vex()
{
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t piVar5 = 0;
    ::recon::guest_delete((int32_t)(this->field_0x738));
    if (((uint32_t)((gaddr(this) + 0x734)) != (uint32_t)(0x0)))
    {
        iVar3 = this->field_0x734;
        bVar1 = (iVar3 == 0);
        if (!(bVar1))
        {
            piVar5 = (uint32_t)((iVar3 + 4));
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar5), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar3 + 8))) + 0x1c)), (uint32_t)(iVar3)) >> 32);
            }
        }
    }
}
#endif

/* create(Level*) [static] [0x023E63A0] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void Vex::create(gptr<Level> a1)
{
    guest::stub(0x023E63A0u, (uint32_t)a1);
}
#else
void Vex::create(gptr<Level> a1)
{
    guest::Vex_ct((uint32_t)(0), (uint32_t)(a1));
    return;
}
#endif

/* staticCtor() [static] [0x02A4440C] - boundary stub; body not reconstructed */
uint64_t Vex::staticCtor()
{
    return (uint64_t)guest::stub(0x02A4440Cu);
}

/* move(MoverType, double, double, double, bool) [0x02A44694] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void Vex::move(uint32_t a1, double a2, double a3, double a4, bool a5)
{
    guest::stub(0x02A44694u, gaddr(this), (uint32_t)a1, (uint32_t)a5, (double)a2, (double)a3, (double)a4);
}
#else
void Vex::move(uint32_t a1, double a2, double a3, double a4, bool a5)
{
    guest::Entity_move((uint32_t)(gaddr(this)), (uint32_t)(a1), (uint32_t)(0));
    this->virt_checkInsideBlocks();
    return;
}
#endif

/* tick() [0x02A446E4] - LOWERED-DRAFT; not reconstructed (2 open edges, 3 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Vex::tick()
{
    guest::stub(0x02A446E4u, gaddr(this));
}
#else
void Vex::tick()
{
    uint32_t uVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t pcVar3 = 0;
    double dVar4 = 0;
    this->virt_setNoGravity((bool)(1));
    ST8((uint32_t)((gaddr(this) + 0x1dc)), 1);
    this->Monster::tick();
    ST8((uint32_t)((gaddr(this) + 0x1dc)), 0);
    uVar1 = global_0x104DAC38;
    if (((uint8_t)this->field_0x73c != '\0'))
    {
        iVar2 = (this->field_0x740 + -(1));
        if ((0 < iVar2))
        {
            this->field_0x740 = iVar2;
            return;
        }
        pcVar3 = LD32((uint32_t)((this->field_0x348 + 0x1dc)));
        dVar4 = (double)(global_0x102B1E18);
        this->field_0x740 = 0x14;
        (uint32_t)(guest::indirect((uint32_t)(pcVar3), (double)(dVar4), (uint32_t)(gaddr(this)), (uint32_t)(uVar1)) >> 32);
    }
    return;
}
#endif

/* registerGoals() [0x02A44948] - LOWERED-DRAFT; not reconstructed (10 open edges, 11 raw IR ops, 5 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Vex::registerGoals()
{
    guest::stub(0x02A44948u, gaddr(this));
}
#else
void Vex::registerGoals()
{
    uint32_t _frame = native_frame_open(8);
    uint32_t uVar1 = 0;
    guest::Mob_registerGoals(gaddr(this));
    uVar1 = guest::FloatGoal_ct((uint32_t)(0), (uint32_t)(gaddr(this)));
    ((GoalSelector *)gmem((gaddr(this) + 0x60c)))->addGoal((int32_t)(0), (gptr<Goal>)(uVar1), (bool)(1));
    uVar1 = guest::Vex_VexChargeAttackGoal_ct((uint32_t)(0), (uint32_t)(gaddr(this)));
    ((GoalSelector *)gmem((gaddr(this) + 0x60c)))->addGoal((int32_t)(4), (gptr<Goal>)(uVar1), (bool)(1));
    uVar1 = guest::Vex_VexRandomMoveGoal_ct((uint32_t)(0), (uint32_t)(gaddr(this)));
    ((GoalSelector *)gmem((gaddr(this) + 0x60c)))->addGoal((int32_t)(8), (gptr<Goal>)(uVar1), (bool)(1));
    uVar1 = guest::LookAtPlayerGoal_ct_2((double)((double)(global_0x102B1E04)), (double)((double)(global_0x102B1E18)), (uint32_t)(0), (uint32_t)(gaddr(this)), (uint32_t)(0x101E0218u));
    ((GoalSelector *)gmem((gaddr(this) + 0x60c)))->addGoal((int32_t)(9), (gptr<Goal>)(uVar1), (bool)(1));
    uVar1 = guest::LookAtPlayerGoal_ct((double)((double)(global_0x102B1E00)), (uint32_t)(0), (uint32_t)(gaddr(this)), (uint32_t)(0x101A4B6Cu));
    ((GoalSelector *)gmem((gaddr(this) + 0x60c)))->addGoal((int32_t)(10), (gptr<Goal>)(uVar1), (bool)(1));
    ST32(_frame, (uint32_t)(guest::guest_new_array((uint32_t)(8))));
    ST32(LD32(_frame) + (1) * 4, 0);
    ST32(LD32(_frame), 0);
    ST32(LD32(_frame), 0);
    ST32(LD32(_frame) + (1) * 4, 0x5d540048);
    ST32((_frame + 4), 1);
    uVar1 = guest::HurtByTargetGoal_ct((uint32_t)(0), (uint32_t)(gaddr(this)), (uint32_t)(1), (uint32_t)(_frame));
    ((GoalSelector *)gmem((gaddr(this) + 0x638)))->addGoal((int32_t)(1), (gptr<Goal>)(uVar1), (bool)(1));
    uVar1 = guest::Vex_VexCopyOwnerTargetGoal_ct((uint32_t)(0), (uint32_t)(gaddr(this)));
    ((GoalSelector *)gmem((gaddr(this) + 0x638)))->addGoal((int32_t)(2), (gptr<Goal>)(uVar1), (bool)(1));
    uVar1 = guest::NearestAttackableTargetGoal_ct((uint32_t)(0), (uint32_t)(gaddr(this)), (uint32_t)(0x101E0218u), (uint32_t)(1), (uint32_t)(0), (uint32_t)(0));
    ((GoalSelector *)gmem((gaddr(this) + 0x638)))->addGoal((int32_t)(3), (gptr<Goal>)(uVar1), (bool)(1));
    native_frame_close(_frame, 8);
    return;
    native_frame_close(_frame, 8);
}
#endif

/* registerAttributes() [0x02A44BDC] - LOWERED-DRAFT; not reconstructed (3 open edges, 2 raw IR ops) */
#if defined(RECON_RUNTIME_BUILD)
void Vex::registerAttributes()
{
    guest::stub(0x02A44BDCu, gaddr(this));
}
#else
void Vex::registerAttributes()
{
    uint32_t piVar1 = 0;
    guest::Monster_registerAttributes(gaddr(this));
    piVar1 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE10)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar1), 0) + 0x24)), (uint32_t)(global_0x102B77A0)) >> 32);
    piVar1 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE24)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar1), 0) + 0x24)), (uint32_t)(global_0x102B77A8)) >> 32);
    return;
}
#endif

/* defineSynchedData() [0x02A44C7C] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A44C7C
void Vex::defineSynchedData()
{
    int32_t iVar1 = 0;
    this->Mob::defineSynchedData();
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar1))->createDateItem__tm__3_Uc(global_0x104C51D8, 0);
    return;
}
#else
void Vex::defineSynchedData()
{
    guest::stub(0x02A44C7Cu, gaddr(this));
}
#endif

/* addDataWalkers(DataFixerUpper*) [static] [0x02A44D0C] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A44D0C
void Vex::addDataWalkers(uint32_t a1, uint32_t a2)
{
    Mob::addDataWalkers((a1), (a2), 0, 0x5d540048);
    return;
}
#else
void Vex::addDataWalkers(uint32_t a1, uint32_t a2)
{
    guest::stub(0x02A44D0Cu, (uint32_t)a1, (uint32_t)a2);
}
#endif

/* setLimitedLife(int) [0x02A44D28] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A44D28
void Vex::setLimitedLife(int32_t a1)
{
    this->field_0x740 = a1;
    this->field_0x73c = 1;
    return;
}
#else
void Vex::setLimitedLife(int32_t a1)
{
    guest::stub(0x02A44D28u, gaddr(this), (uint32_t)a1);
}
#endif

/* readAdditionalSaveData(CompoundTag*) [0x02A44D38] - boundary stub; body not reconstructed */
void Vex::readAdditionalSaveData(gptr<CompoundTag> a1)
{
    guest::stub(0x02A44D38u, gaddr(this), (uint32_t)a1);
}

/* addAdditonalSaveData(CompoundTag*) [0x02A45150] - LOWERED-DRAFT; not reconstructed (9 open edges, 28 raw IR ops, 5 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Vex::addAdditonalSaveData(gptr<CompoundTag> a1)
{
    guest::stub(0x02A45150u, gaddr(this), (uint32_t)a1);
}
#else
void Vex::addAdditonalSaveData(gptr<CompoundTag> a1)
{
    uint32_t _frame = native_frame_open(64);
    uint32_t sVar1 = 0;
    guest::Mob_addAdditonalSaveData(gaddr(this), (uint32_t)a1);
    if ((this->field_0x738 != 0))
    {
        ST32((_frame + 28), 7);
        ST16((_frame + 4), 0);
        ST32((_frame + 24), 0);
        sVar1 = ::recon::wcslen(GSTR("BoundX", 0x102B78C8));
        ((wstring *)gmem(_frame))->assign(GSTR("BoundX", 0x102B78C8), (uint32_t)(sVar1));
        a1->putInt((gptr<wstring>)(_frame), (int32_t)(LD32(LD32((uint32_t)((gaddr(this) + 0x738))))));
        if ((7 < LD32((_frame + 28))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
        }
        ST32((_frame + 28), 7);
        ST16((_frame + 4), 0);
        ST32((_frame + 24), 0);
        sVar1 = ::recon::wcslen(GSTR("BoundY", 0x102B78D8));
        ((wstring *)gmem(_frame))->assign(GSTR("BoundY", 0x102B78D8), (uint32_t)(sVar1));
        a1->putInt((gptr<wstring>)(_frame), (int32_t)(LD32((uint32_t)((this->field_0x738 + 4)))));
        if ((7 < LD32((_frame + 28))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
        }
        ST32((_frame + 28), 7);
        ST16((_frame + 4), 0);
        ST32((_frame + 24), 0);
        sVar1 = ::recon::wcslen(GSTR("BoundZ", 0x102B78E8));
        ((wstring *)gmem(_frame))->assign(GSTR("BoundZ", 0x102B78E8), (uint32_t)(sVar1));
        a1->putInt((gptr<wstring>)(_frame), (int32_t)(LD32((uint32_t)((this->field_0x738 + 8)))));
        if ((7 < LD32((_frame + 28))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
        }
        ST32((_frame + 24), 0);
        ST16((_frame + 4), 0);
        ST32((_frame + 28), 7);
    }
    if (((uint8_t)this->field_0x73c != '\0'))
    {
        ST32((_frame + 56), 0);
        ST16((_frame + 36), 0);
        ST32((_frame + 60), 7);
        sVar1 = ::recon::wcslen(GSTR("LifeTicks", 0x102B78B4));
        ((wstring *)gmem((_frame + 32)))->assign(GSTR("LifeTicks", 0x102B78B4), (uint32_t)(sVar1));
        a1->putInt((gptr<wstring>)((_frame + 32)), (int32_t)(this->field_0x740));
        if ((7 < LD32((_frame + 60))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 52))));
        }
    }
    native_frame_close(_frame, 64);
    return;
    native_frame_close(_frame, 64);
}
#endif

/* getOwner() [0x02A453F0] - boundary stub; body not reconstructed */
uint64_t Vex::getOwner()
{
    return (uint64_t)guest::stub(0x02A453F0u, gaddr(this));
}

/* getBoundOrigin() [0x02A45484] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A45484
gptr<BlockPos> Vex::getBoundOrigin()
{
    return this->field_0x738;
}
#else
gptr<BlockPos> Vex::getBoundOrigin()
{
    return (gptr<BlockPos>)(guest::stub(0x02A45484u, gaddr(this)) >> 32);
}
#endif

/* setBoundOrigin(BlockPos*) [0x02A4548C] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A4548C
void Vex::setBoundOrigin(gptr<BlockPos> a1)
{
    gdelete(this->field_0x738);
    this->field_0x738 = a1;
    return;
}
#else
void Vex::setBoundOrigin(gptr<BlockPos> a1)
{
    guest::stub(0x02A4548Cu, gaddr(this), (uint32_t)a1);
}
#endif

/* getVexFlag(int) [0x02A454D8] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
bool Vex::getVexFlag(int32_t a1)
{
    return (bool)(guest::stub(0x02A454D8u, gaddr(this), (uint32_t)a1) >> 32);
}
#else
bool Vex::getVexFlag(int32_t a1)
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return ((LD8((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C51D8u)) * 4)) + 4))) + 0xc))) & a1) != 0);
}
#endif

/* setVexFlag(int, bool) [0x02A45570] - LOWERED-DRAFT; not reconstructed (1 open edges, 4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Vex::setVexFlag(int32_t a1, bool a2)
{
    return (uint64_t)guest::stub(0x02A45570u, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}
#else
uint64_t Vex::setVexFlag(int32_t a1, bool a2)
{
    int32_t iVar1 = 0;
    uint8_t bVar2 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    bVar2 = LD8((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C51D8u)) * 4)) + 4))) + 0xc)));
    if ((a2 == 0))
    {
        bVar2 = (bVar2 & ~(a1));
    }
    else
    {
        bVar2 = (bVar2 | a1);
    }
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return ((uint64_t)(uint32_t)(guest::SynchedEntityData_set__tm__3_Uc((uint32_t)(iVar1), (uint32_t)(global_0x104C51D8), (uint32_t)(bVar2), (uint32_t)(1))) << 32);
}
#endif

/* isCharging() [0x02A4565C] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
bool Vex::isCharging()
{
    return (bool)(guest::stub(0x02A4565Cu, gaddr(this)) >> 32);
}
#else
bool Vex::isCharging()
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return ((LD8((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C51D8u)) * 4)) + 4))) + 0xc))) & 1) != 0);
}
#endif

/* setIsCharging(bool) [0x02A45670] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Vex::setIsCharging(bool a1)
{
    guest::stub(0x02A45670u, gaddr(this), (uint32_t)a1);
}
#else
void Vex::setIsCharging(bool a1)
{
    int32_t iVar1 = 0;
    uint8_t bVar2 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    bVar2 = LD8((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C51D8u)) * 4)) + 4))) + 0xc)));
    if ((a1 == 0))
    {
        bVar2 = (bVar2 & 0xfe);
    }
    else
    {
        bVar2 = (bVar2 | 1);
    }
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar1))->set__tm__3_Uc(global_0x104C51D8, (uint32_t)(bVar2), (bool)(1));
    return;
}
#endif

/* setOwner(const boost::shared_ptr<Mob>&) [0x02A45688] - LOWERED-DRAFT; not reconstructed (1 open edges, 4 raw IR ops, 2 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Vex::setOwner(uint32_t a1)
{
    return (uint64_t)guest::stub(0x02A45688u, gaddr(this), (uint32_t)a1);
}
#else
uint64_t Vex::setOwner(uint32_t a1)
{
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t piVar3 = 0;
    int32_t iVar4 = 0;
    int32_t iVar5 = 0;
    int32_t iVar6 = 0;
    iVar4 = this->field_0x734;
    iVar6 = gelem<uint32_t>((uint32_t)(a1) + 1u, 0);
    this->field_0x730 = gelem<uint32_t>((uint32_t)(a1), 0);
    if ((iVar6 != iVar4))
    {
        bVar1 = (iVar6 == 0);
        if (!(bVar1))
        {
            piVar3 = (uint32_t)((iVar6 + 4));
            do
            {
                if (('\x01' != '\0'))
                {
                    iVar4 = (ST32((uint32_t)(piVar3) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar3), 0) + 1)), 1);
                    gelem<int32_t>((uint32_t)(piVar3), 0) = iVar4;
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            iVar4 = this->field_0x734;
        }
        bVar1 = (iVar4 == 0);
        if (!(bVar1))
        {
            piVar3 = (uint32_t)((iVar4 + 4));
            ((void)0 /* sync dropped */);
            do
            {
                iVar5 = gelem<int32_t>((uint32_t)(piVar3), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar3) + (uint32_t)(0), (iVar5 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar3), 0) = iVar2;
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar5 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar4 + 8))) + 0x1c)), (uint32_t)(iVar4)) >> 32);
            }
        }
        this->field_0x734 = iVar6;
    }
    return ((uint64_t)(uint32_t)(gaddr(this)) << 32);
}
#endif

/* getAmbientSound() [0x02A45740] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A45740
uint32_t Vex::getAmbientSound()
{
    return global_0x104C469C;
}
#else
uint32_t Vex::getAmbientSound()
{
    return (uint32_t)(guest::stub(0x02A45740u, gaddr(this)) >> 32);
}
#endif

/* getDeathSound() [0x02A4574C] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A4574C
uint32_t Vex::getDeathSound()
{
    return global_0x104C46A4;
}
#else
uint32_t Vex::getDeathSound()
{
    return (uint32_t)(guest::stub(0x02A4574Cu, gaddr(this)) >> 32);
}
#endif

/* getHurtSound(const DamageSource*) [0x02A45758] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A45758
uint32_t Vex::getHurtSound(gptr<DamageSource> a1)
{
    return global_0x104C46A8;
}
#else
uint32_t Vex::getHurtSound(gptr<DamageSource> a1)
{
    return (uint32_t)(guest::stub(0x02A45758u, gaddr(this), (uint32_t)a1) >> 32);
}
#endif

/* getDefaultLootTable() [0x02A45764] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A45764
uint32_t Vex::getDefaultLootTable()
{
    return 0x104D6E50u;
}
#else
uint32_t Vex::getDefaultLootTable()
{
    return (uint32_t)(guest::stub(0x02A45764u, gaddr(this)) >> 32);
}
#endif

/* getDefaultGearTable() [0x02A45770] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A45770
uint32_t Vex::getDefaultGearTable()
{
    return 0x104D6EA0u;
}
#else
uint32_t Vex::getDefaultGearTable()
{
    return (uint32_t)(guest::stub(0x02A45770u, gaddr(this)) >> 32);
}
#endif

/* getLightColor() [0x02A4577C] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A4577C
uint32_t Vex::getLightColor()
{
    return 0xf000f0;
}
#else
uint32_t Vex::getLightColor()
{
    return (uint32_t)(guest::stub(0x02A4577Cu, gaddr(this)) >> 32);
}
#endif

/* getBrightness() [0x02A45788] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A45788
double Vex::getBrightness()
{
    return gref<double>(0x102B1E18u);
}
#else
double Vex::getBrightness()
{
    guest::stub(0x02A45788u, gaddr(this));
    return native_fret();
}
#endif

/* finalizeMobSpawn(const DifficultyInstance&, MobGroupData*, const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) [0x02A45794] - LOWERED-DRAFT; not reconstructed (1 open edges, 1 raw IR ops) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Vex::finalizeMobSpawn(gptr<DifficultyInstance> a1, gptr<MobGroupData> a2, gptr<wstring> a3)
{
    return (uint32_t)(guest::stub(0x02A45794u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3) >> 32);
}
#else
uint32_t Vex::finalizeMobSpawn(gptr<DifficultyInstance> a1, gptr<MobGroupData> a2, gptr<wstring> a3)
{
    (uint32_t)(guest::indirect(LD32((this->field_0x348 + 0xd2c)), (uint32_t)(gaddr(this))) >> 32);
    this->virt_populateDefaultEquipmentEnchantments((gptr<DifficultyInstance>)(a1));
    return ((uint64_t)(uint32_t)(this->Mob::finalizeMobSpawn((gptr<DifficultyInstance>)(a1), (gptr<MobGroupData>)(a2), (gptr<wstring>)(a3))) << 32);
}
#endif

/* populateDefaultEquipmentSlots(const DifficultyInstance&) [0x02A45828] - LOWERED-DRAFT; not reconstructed (2 open edges, 1 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Vex::populateDefaultEquipmentSlots(gptr<DifficultyInstance> a1)
{
    guest::stub(0x02A45828u, gaddr(this), (uint32_t)a1);
}
#else
void Vex::populateDefaultEquipmentSlots(gptr<DifficultyInstance> a1)
{
    guest::Mob_populateDefaultEquipmentSlots(gaddr(this), (uint32_t)a1);
    (uint32_t)(guest::indirect(LD32((this->field_0x348 + 0xd54)), (double)((double)(global_0x102B5164)), (uint32_t)(gaddr(this)), (uint32_t)(global_0x104C2778)) >> 32);
    return;
}
#endif

/* GetType() [0x02ACE674] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02ACE674
uint64_t Vex::GetType()
{
    return 0x5d54004800000000;
}
#else
uint64_t Vex::GetType()
{
    return (uint64_t)guest::stub(0x02ACE674u, gaddr(this));
}
#endif

}  /* namespace recon */
\n#include "game/V/Vex__VexChargeAttackGoal.hpp"
#include "game/A/AABB.hpp"
#include "game/E/Entity.hpp"
#include "game/G/Goal.hpp"
#include "game/L/LivingEntity.hpp"
#include "game/S/SynchedEntityData.hpp"
#include "game/V/Vex.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(Vex*) [0x02A447A0] - reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Vex_VexChargeAttackGoal::Vex_VexChargeAttackGoal(gptr<Vex> a1)
{
    guest::stub(0x02A447A0u, gaddr(this), (uint32_t)a1);
}
#else
Vex_VexChargeAttackGoal::Vex_VexChargeAttackGoal(gptr<Vex> a1) : Goal()
{
    this->field_0x8 = a1;
    this->Goal::setRequiredControlFlags(1);
}
#endif

/* {dtor}() [0x02ACE6A4] - reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Vex_VexChargeAttackGoal::~Vex_VexChargeAttackGoal()
{
    guest::stub(0x02ACE6A4u, gaddr(this), 0u);
}
#else
Vex_VexChargeAttackGoal::~Vex_VexChargeAttackGoal()
{
    if ((gaddr(this) == 0))
    {
        return;
    }
}
#endif

/* canUse() [0x02A45D3C] - boundary stub; body not reconstructed */
bool Vex_VexChargeAttackGoal::canUse()
{
    return (bool)(guest::stub(0x02A45D3Cu, gaddr(this)) >> 32);
}

/* canContinueToUse() [0x02A46018] - boundary stub; body not reconstructed */
uint32_t Vex_VexChargeAttackGoal::canContinueToUse()
{
    return (uint32_t)(guest::stub(0x02A46018u, gaddr(this)) >> 32);
}

/* start() [0x02A46268] - LOWERED-DRAFT; not reconstructed (4 open edges, 8 raw IR ops, 6 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Vex_VexChargeAttackGoal::start()
{
    guest::stub(0x02A46268u, gaddr(this));
}
#else
void Vex_VexChargeAttackGoal::start()
{
    uint32_t _frame = native_frame_open(40);
    uint8_t bVar1 = 0;
    uint32_t puVar3 = 0;
    double dVar7 = 0;
    this->field_0x8->virt_getTarget(_frame);
    if ((LD32(_frame) == 0))
    {
    }
    dVar7 = (double)(global_0x102B1E18);
    puVar3 = (uint32_t)(((Entity *)gmem(LD32(_frame)))->getEyePosition((float)(dVar7)));
    (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((LD32((uint32_t)((this->field_0x8 + 0x5fc))) + 0x34))) + 0x14)), (uint32_t)((gelem<uint64_t>((uint32_t)(puVar3), 0)) >> 32), (uint32_t)(gelem<uint64_t>((uint32_t)(puVar3), 0)), (uint32_t)((gelem<uint64_t>((uint32_t)(puVar3) + 1u, 0)) >> 32), (uint32_t)(gelem<uint64_t>((uint32_t)(puVar3) + 1u, 0)), (uint32_t)((gelem<uint64_t>((uint32_t)(puVar3) + 2u, 0)) >> 32), (uint32_t)(gelem<uint64_t>((uint32_t)(puVar3) + 2u, 0)), (uint32_t)(global_0x102B2FB0)) >> 32);
    this->field_0x8->setIsCharging((bool)(1));
    (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((this->field_0x8 + 0x348))) + 0xf4)), (double)(dVar7), (double)(dVar7), (uint32_t)(this->field_0x8), (uint32_t)(global_0x104C46A0)) >> 32);
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
    native_frame_close(_frame, 40);
    return;
    native_frame_close(_frame, 40);
}
#endif

/* stop() [0x02A46408] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Vex_VexChargeAttackGoal::stop()
{
    guest::stub(0x02A46408u, gaddr(this));
}
#else
void Vex_VexChargeAttackGoal::stop()
{
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    gptr<Vex> vex = 0u;
    vex = this->field_0x8;
    iVar2 = vex->field_0x1f8;
    if ((iVar2 == 0))
    {
        iVar2 = vex->field_0x1f8;
    }
    bVar1 = LD8((uint32_t)((LD32((uint32_t)(((iVar2 + (LD32(LD32(0x104C51D8u)) * 4)) + 4))) + 0xc)));
    if ((iVar2 == 0))
    {
        iVar2 = vex->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar2))->set__tm__3_Uc(global_0x104C51D8, (uint32_t)((bVar1 & 0xfe)), (bool)(1));
    return;
}
#endif

/* tick() [0x02A46420] - LOWERED-DRAFT; not reconstructed (12 open edges, 77 raw IR ops, 26 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Vex_VexChargeAttackGoal::tick()
{
    guest::stub(0x02A46420u, gaddr(this));
}
#else
void Vex_VexChargeAttackGoal::tick()
{
    uint32_t _frame = native_frame_open(56);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint8_t cVar3 = 0;
    uint32_t piVar4 = 0;
    uint32_t puVar5 = 0;
    uint32_t piVar6 = 0;
    gptr<Vex> vex = 0u;
    int32_t iVar8 = 0;
    uint8_t bVar9 = 0;
    double dVar10 = 0;
    ((void)0 /* vectorMultiplyHighAndAddSignedHalfWordSa on ghost registers dropped */);
    this->field_0x8->virt_getTarget(_frame);
    iVar8 = this->field_0x8;
    if ((LD32(_frame) == 0))
    {
        guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102b7cc0_s_boost__detail__sp_member_access_", 0x102B7CC8)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102B7D34)), (uint32_t)(0x28d), 0u, 0u);
    }
    iVar2 = LD32(_frame);
    puVar5 = LD32((uint32_t)((LD32(_frame) + 0x158)));
    cVar3 = ((AABB *)gmem(LD32((uint32_t)((iVar8 + 0x158)))))->intersects((double)(gelem<uint64_t>((uint32_t)(puVar5), 0)), (double)(gelem<uint64_t>((uint32_t)(puVar5) + 1u, 0)), (double)(gelem<uint64_t>((uint32_t)(puVar5) + 2u, 0)), (double)(gelem<uint64_t>((uint32_t)(puVar5) + 3u, 0)), (double)(gelem<uint64_t>((uint32_t)(puVar5) + 4u, 0)), (double)(gelem<uint64_t>((uint32_t)(puVar5) + 5u, 0)));
    if ((cVar3 == '\0'))
    {
        ST32((_frame + 16), iVar2);
        bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
        ST32((_frame + 20), LD32((_frame + 4)));
        while (!(bVar1))
        {
            if (('\x01' != '\0'))
            {
                iVar8 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
                ST32(LD32((_frame + 4)), iVar8);
                bVar1 = 1;
            }
        }
        dVar10 = (double)(this->field_0x8->Entity::distanceToSqr((gptr<sp_Entity>)((_frame + 16))));
        piVar6 = LD32((_frame + 20));
        bVar1 = (LD32((_frame + 20)) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            ((void)0 /* sync dropped */);
            do
            {
                iVar8 = (LD32(LD32((_frame + 20))) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(LD32((_frame + 20))) + (uint32_t)(0), iVar8), 1);
                    ST32(LD32((_frame + 20)), iVar2);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            bVar9 = ((iVar8 == 0) << 1);
            if ((iVar8 == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 20)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 20)))) >> 32);
                piVar4 = (piVar6 + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar8 = gelem<int32_t>((uint32_t)(piVar4), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar2 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar8 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar4), 0) = iVar2;
                        bVar9 = 2;
                    }
                }
                while (!((uint8_t)(((bVar9 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar8 + -(1)) == 0))
                {
                    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)) >> 32);
                }
            }
        }
        if ((dVar10 < 9.0))
        {
            if ((LD32(_frame) == 0))
            {
                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102b7cc0_s_boost__detail__sp_member_access_", 0x102B7CC8)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102B7D34)), (uint32_t)(0x28d), 0u, 0u);
            }
            puVar5 = (uint32_t)(((Entity *)gmem(LD32(_frame)))->getEyePosition((float)((double)(global_0x102B1E18))));
            (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((LD32((uint32_t)((this->field_0x8 + 0x5fc))) + 0x34))) + 0x14)), (uint32_t)((gelem<uint64_t>((uint32_t)(puVar5), 0)) >> 32), (uint32_t)(gelem<uint64_t>((uint32_t)(puVar5), 0)), (uint32_t)((gelem<uint64_t>((uint32_t)(puVar5) + 1u, 0)) >> 32), (uint32_t)(gelem<uint64_t>((uint32_t)(puVar5) + 1u, 0)), (uint32_t)((gelem<uint64_t>((uint32_t)(puVar5) + 2u, 0)) >> 32), (uint32_t)(gelem<uint64_t>((uint32_t)(puVar5) + 2u, 0)), (uint32_t)(global_0x102B2FB0)) >> 32);
        }
    }
    else
    {
        vex = this->field_0x8;
        iVar8 = vex->field_0x348;
        bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
        ST32((_frame + 8), iVar2);
        ST32((_frame + 12), LD32((_frame + 4)));
        if (!(bVar1))
        {
            do
            {
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
                    ST32(LD32((_frame + 4)), iVar2);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            vex = this->field_0x8;
        }
        (uint32_t)(guest::indirect(LD32((iVar8 + 0xae4)), (uint32_t)(vex), (uint32_t)((_frame + 8))) >> 32);
        piVar6 = LD32((_frame + 12));
        bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            ((void)0 /* sync dropped */);
            do
            {
                iVar8 = (LD32(LD32((_frame + 12))) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(LD32((_frame + 12))) + (uint32_t)(0), iVar8), 1);
                    ST32(LD32((_frame + 12)), iVar2);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            bVar9 = ((iVar8 == 0) << 1);
            if ((iVar8 == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 12)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 12)))) >> 32);
                piVar4 = (piVar6 + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar8 = gelem<int32_t>((uint32_t)(piVar4), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar2 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar8 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar4), 0) = iVar2;
                        bVar9 = 2;
                    }
                }
                while (!((uint8_t)(((bVar9 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar8 + -(1)) == 0))
                {
                    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)) >> 32);
                }
            }
        }
        this->field_0x8->setIsCharging((bool)(0));
    }
    if ((LD32((_frame + 4)) != (uint32_t)(0x0)))
    {
        bVar1 = 0;
        ((void)0 /* sync dropped */);
        do
        {
            iVar8 = (LD32(LD32((_frame + 4))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar8), 1);
                ST32(LD32((_frame + 4)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar9 = ((iVar8 == 0) << 1);
        if ((iVar8 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            piVar6 = (LD32((_frame + 4)) + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar8 = gelem<int32_t>((uint32_t)(piVar6), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar8 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar6), 0) = iVar2;
                    bVar9 = 2;
                }
            }
            while (!((uint8_t)(((bVar9 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar8 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            }
        }
    }
    ((void)0 /* instructionSynchronize dropped */);
    native_frame_close(_frame, 56);
    return;
    native_frame_close(_frame, 56);
}
#endif

/* getName() [0x02A467CC] - LOWERED-DRAFT; not reconstructed (3 open edges, 3 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gresult<wstring> Vex_VexChargeAttackGoal::getName()
{
    gresult<wstring> result;
    guest::stub(0x02A467CCu, gaddr(this), (uint32_t)result);
    return result;
}
#else
gresult<wstring> Vex_VexChargeAttackGoal::getName(int32_t a1)
{
    uint32_t sVar1 = 0;
    if (((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0x20)), (a1 != 0))))
    {
        ST32((uint32_t)((a1 + 0x18)), 0);
        ST16((uint32_t)((a1 + 4)), 0);
        ST32((uint32_t)((a1 + 0x1c)), 7);
        sVar1 = ::recon::wcslen(GSTR("Vex_Charging_Attack", 0x102B7D7C));
        ((wstring *)gmem(a1))->assign(GSTR("Vex_Charging_Attack", 0x102B7D7C), (uint32_t)(sVar1));
    }
    return;
}
#endif

}  /* namespace recon */
\n#include "game/V/Vex__VexRandomMoveGoal.hpp"
#include "game/G/Goal.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(Vex*) [0x02A44834] - reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Vex_VexRandomMoveGoal::Vex_VexRandomMoveGoal(gptr<Vex> a1)
{
    guest::stub(0x02A44834u, gaddr(this), (uint32_t)a1);
}
#else
Vex_VexRandomMoveGoal::Vex_VexRandomMoveGoal(gptr<Vex> a1) : Goal()
{
    this->field_0x8 = a1;
    this->Goal::setRequiredControlFlags(1);
}
#endif

/* {dtor}() [0x02ACE6C4] - reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Vex_VexRandomMoveGoal::~Vex_VexRandomMoveGoal()
{
    guest::stub(0x02ACE6C4u, gaddr(this), 0u);
}
#else
Vex_VexRandomMoveGoal::~Vex_VexRandomMoveGoal()
{
    if ((gaddr(this) == 0))
    {
        return;
    }
}
#endif

/* canUse() [0x02A46864] - boundary stub; body not reconstructed */
uint32_t Vex_VexRandomMoveGoal::canUse()
{
    return (uint32_t)(guest::stub(0x02A46864u, gaddr(this)) >> 32);
}

/* canContinueToUse() [0x02A468F0] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A468F0
uint32_t Vex_VexRandomMoveGoal::canContinueToUse()
{
    return 0;
}
#else
uint32_t Vex_VexRandomMoveGoal::canContinueToUse()
{
    return (uint32_t)(guest::stub(0x02A468F0u, gaddr(this)) >> 32);
}
#endif

/* tick() [0x02A468F8] - boundary stub; body not reconstructed */
void Vex_VexRandomMoveGoal::tick()
{
    guest::stub(0x02A468F8u, gaddr(this));
}

/* getName() [0x02A4702C] - LOWERED-DRAFT; not reconstructed (3 open edges, 3 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gresult<wstring> Vex_VexRandomMoveGoal::getName()
{
    gresult<wstring> result;
    guest::stub(0x02A4702Cu, gaddr(this), (uint32_t)result);
    return result;
}
#else
gresult<wstring> Vex_VexRandomMoveGoal::getName(int32_t a1)
{
    uint32_t sVar1 = 0;
    if (((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0x20)), (a1 != 0))))
    {
        ST32((uint32_t)((a1 + 0x18)), 0);
        ST16((uint32_t)((a1 + 4)), 0);
        ST32((uint32_t)((a1 + 0x1c)), 7);
        sVar1 = ::recon::wcslen(GSTR("Vex_Random_Move", 0x102B7DAC));
        ((wstring *)gmem(a1))->assign(GSTR("Vex_Random_Move", 0x102B7DAC), (uint32_t)(sVar1));
    }
    return;
}
#endif

}  /* namespace recon */
\n#include "game/V/Vex__VexCopyOwnerTargetGoal.hpp"
#include "game/T/TargetGoal.hpp"
#include "game/V/Vex.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(PathfinderMob*) [0x02A448C8] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Vex_VexCopyOwnerTargetGoal::Vex_VexCopyOwnerTargetGoal(gptr<PathfinderMob> a1)
{
    guest::stub(0x02A448C8u, gaddr(this), (uint32_t)a1);
}
#else
Vex_VexCopyOwnerTargetGoal::Vex_VexCopyOwnerTargetGoal(gptr<PathfinderMob> a1)
{
    guest::TargetGoal_ct_2((uint32_t)(gaddr(this)), (uint32_t)(a1), (uint32_t)(0));
    ST32((uint32_t)((gaddr(this) + 4)), GSTR(L"", 0x102CE164));
}
#endif

/* {dtor}() [0x02ACE6E4] - LOWERED-DRAFT; not reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Vex_VexCopyOwnerTargetGoal::~Vex_VexCopyOwnerTargetGoal()
{
    guest::stub(0x02ACE6E4u, gaddr(this), 0u);
}
#else
Vex_VexCopyOwnerTargetGoal::~Vex_VexCopyOwnerTargetGoal()
{
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t piVar5 = 0;
    if (((uint32_t)((gaddr(this) + 0x20)) != (uint32_t)(0x0)))
    {
        iVar3 = this->field_0x20;
        bVar1 = (iVar3 == 0);
        if (!(bVar1))
        {
            piVar5 = (uint32_t)((iVar3 + 4));
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar5), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar3 + 8))) + 0x1c)), (uint32_t)(iVar3)) >> 32);
            }
        }
    }
}
#endif

/* canUse() [0x02A470C4] - LOWERED-DRAFT; not reconstructed (4 open edges, 13 raw IR ops, 5 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Vex_VexCopyOwnerTargetGoal::canUse()
{
    return (uint32_t)(guest::stub(0x02A470C4u, gaddr(this)) >> 32);
}
#else
uint32_t Vex_VexCopyOwnerTargetGoal::canUse()
{
    uint32_t _frame = native_frame_open(24);
    uint8_t bVar1 = 0;
    uint8_t bVar3 = 0;
    int32_t iVar5 = 0;
    uint32_t uVar6 = 0;
    uint64_t __pair = 0;
    (__pair = (uint64_t)(((Vex *)gmem(this->field_0x8))->getOwner(_frame)));
    bVar1 = 0;
    bVar3 = 0;
    uVar6 = 0;
    if ((LD32(_frame) != 0))
    {
        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((LD32(_frame) + 0x348))) + 0xc54)), (uint32_t)(LD32(_frame)), (uint32_t)((_frame + 8))))) >> 32);
        bVar3 = 1;
        if ((LD32((_frame + 8)) != 0))
        {
            iVar5 = this->field_0x4;
            if ((LD32(_frame) == 0))
            {
            }
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((LD32(_frame) + 0x348))) + 0xc54)), (uint32_t)(LD32(_frame)), (uint32_t)((_frame + 16))))) >> 32);
            bVar1 = 1;
            iVar5 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar5 + 0x6c)), (uint32_t)(gaddr(this)), (uint32_t)((_frame + 16)), (uint32_t)(0)))) >> 32);
            if ((iVar5 != 0))
            {
                uVar6 = 1;
            }
        }
    }
    if (bVar1)
    {
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
    if (bVar3)
    {
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
    { uint32_t t1 = uVar6;
    native_frame_close(_frame, 24);
    return t1; }
    native_frame_close(_frame, 24);
}
#endif

/* start() [0x02A47364] - LOWERED-DRAFT; not reconstructed (3 open edges, 8 raw IR ops, 4 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Vex_VexCopyOwnerTargetGoal::start()
{
    guest::stub(0x02A47364u, gaddr(this));
}
#else
void Vex_VexCopyOwnerTargetGoal::start()
{
    uint32_t _frame = native_frame_open(16);
    uint8_t bVar1 = 0;
    int32_t iVar3 = 0;
    ((Vex *)gmem(this->field_0x8))->getOwner(_frame);
    iVar3 = LD32((uint32_t)((this->field_0x8 + 0x348)));
    if ((LD32(_frame) == 0))
    {
    }
    (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((LD32(_frame) + 0x348))) + 0xc54)), (uint32_t)(LD32(_frame)), (uint32_t)((_frame + 8))) >> 32);
    (uint32_t)(guest::indirect(LD32((iVar3 + 0xc5c)), (uint32_t)(this->field_0x8), (uint32_t)((_frame + 8))) >> 32);
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
    this->TargetGoal::start();
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

/* getName() [0x02A47538] - LOWERED-DRAFT; not reconstructed (3 open edges, 3 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gresult<wstring> Vex_VexCopyOwnerTargetGoal::getName()
{
    gresult<wstring> result;
    guest::stub(0x02A47538u, gaddr(this), (uint32_t)result);
    return result;
}
#else
gresult<wstring> Vex_VexCopyOwnerTargetGoal::getName(int32_t a1)
{
    uint32_t sVar1 = 0;
    if (((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0x20)), (a1 != 0))))
    {
        ST32((uint32_t)((a1 + 0x18)), 0);
        ST16((uint32_t)((a1 + 4)), 0);
        ST32((uint32_t)((a1 + 0x1c)), 7);
        sVar1 = ::recon::wcslen(GSTR("Vex_Copy_Owner_Target", 0x102B7F34));
        ((wstring *)gmem(a1))->assign(GSTR("Vex_Copy_Owner_Target", 0x102B7F34), (uint32_t)(sVar1));
    }
    return;
}
#endif

}  /* namespace recon */
