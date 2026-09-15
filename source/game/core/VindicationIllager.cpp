#include "game/V/VindicationIllager.hpp"
#include "game/A/AbstractIllager.hpp"
#include "game/C/CompoundTag.hpp"
#include "game/D/DamageSource.hpp"
#include "game/D/DifficultyInstance.hpp"
#include "game/E/Entity.hpp"
#include "game/L/Level.hpp"
#include "game/L/LivingEntity.hpp"
#include "game/M/Mob.hpp"
#include "game/M/MobGroupData.hpp"
#include "game/M/Monster.hpp"
#include "game/S/SynchedEntityData.hpp"
#include "_thirdparty/boost.hpp"
#include "_thirdparty/boost__shared_ptr_Entity_.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(Level*) [0x02A475D0] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
VindicationIllager::VindicationIllager(gptr<Level> a1)
{
    guest::stub(0x02A475D0u, gaddr(this), (uint32_t)a1);
}
#else
VindicationIllager::VindicationIllager(gptr<Level> a1)
{
    guest::AbstractIllager_ct((uint32_t)(gaddr(this)), (uint32_t)(a1));
    ST32((uint32_t)((gaddr(this) + 0x348)), GSTR(L"", 0x102C6F04));
    ST32((uint32_t)((gaddr(this) + 8)), 0x102C7D0Cu);
    this->Mob::fjDerivedCtorCalls();
    this->Entity::setSize((float)((double)(global_0x102B4358)), (float)((double)(global_0x102B5E9C)));
    this->field_0x730 = 0;
    this->field_0x190 = (this->field_0x190 | 0xc);
}
#endif

/* {dtor}() [0x02ACD2E0] - boundary stub */
VindicationIllager::~VindicationIllager()
{
    guest::stub(0x02ACD2E0u, gaddr(this), 0u);
}

/* create(Level*) [static] [0x023E63B8] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void VindicationIllager::create(gptr<Level> a1)
{
    guest::stub(0x023E63B8u, (uint32_t)a1);
}
#else
void VindicationIllager::create(gptr<Level> a1)
{
    guest::VindicationIllager_ct((uint32_t)(0), (uint32_t)(a1));
    return;
}
#endif

/* addDataWalkers(DataFixerUpper*) [static] [0x02A476A4] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A476A4
void VindicationIllager::addDataWalkers(uint32_t a1, uint32_t a2)
{
    Mob::addDataWalkers((a1), (a2), 0, 0x5d541041);
    return;
}
#else
void VindicationIllager::addDataWalkers(uint32_t a1, uint32_t a2)
{
    guest::stub(0x02A476A4u, (uint32_t)a1, (uint32_t)a2);
}
#endif

/* registerGoals() [0x02A47780] - boundary stub; body not reconstructed */
void VindicationIllager::registerGoals()
{
    guest::stub(0x02A47780u, gaddr(this));
}

/* registerAttributes() [0x02A47B10] - LOWERED-DRAFT; not reconstructed (5 open edges, 4 raw IR ops) */
#if defined(RECON_RUNTIME_BUILD)
void VindicationIllager::registerAttributes()
{
    guest::stub(0x02A47B10u, gaddr(this));
}
#else
void VindicationIllager::registerAttributes()
{
    uint32_t piVar1 = 0;
    guest::Monster_registerAttributes(gaddr(this));
    piVar1 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE1C)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar1), 0) + 0x24)), (uint32_t)(global_0x102B7F68)) >> 32);
    piVar1 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE14)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar1), 0) + 0x24)), (uint32_t)(global_0x102B7F70)) >> 32);
    piVar1 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE10)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar1), 0) + 0x24)), (uint32_t)(global_0x102B7F78)) >> 32);
    piVar1 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE24)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar1), 0) + 0x24)), (uint32_t)(global_0x102B5528)) >> 32);
    return;
}
#endif

/* defineSynchedData() [0x02A47C18] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A47C18
void VindicationIllager::defineSynchedData()
{
    int32_t iVar1 = 0;
    this->Mob::defineSynchedData();
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar1))->createDateItem__tm__3_Uc(global_0x104C0F70, 0);
    return;
}
#else
void VindicationIllager::defineSynchedData()
{
    guest::stub(0x02A47C18u, gaddr(this));
}
#endif

/* getMobType() [0x02A47CA8] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A47CA8
uint32_t VindicationIllager::getMobType()
{
    return 3;
}
#else
uint32_t VindicationIllager::getMobType()
{
    return (uint32_t)(guest::stub(0x02A47CA8u, gaddr(this)) >> 32);
}
#endif

/* getDefaultLootTable() [0x02A47CB0] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A47CB0
uint32_t VindicationIllager::getDefaultLootTable()
{
    return 0x104D6F40u;
}
#else
uint32_t VindicationIllager::getDefaultLootTable()
{
    return (uint32_t)(guest::stub(0x02A47CB0u, gaddr(this)) >> 32);
}
#endif

/* getDefaultGearTable() [0x02A47CBC] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A47CBC
uint32_t VindicationIllager::getDefaultGearTable()
{
    return 0x104D6F90u;
}
#else
uint32_t VindicationIllager::getDefaultGearTable()
{
    return (uint32_t)(guest::stub(0x02A47CBCu, gaddr(this)) >> 32);
}
#endif

/* isAgressive() [0x02A47CC8] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A47CC8
void VindicationIllager::isAgressive()
{
    this->AbstractIllager::getIllagerFlag(1);
    return;
}
#else
void VindicationIllager::isAgressive()
{
    guest::stub(0x02A47CC8u, gaddr(this));
}
#endif

/* setAggressive(bool) [0x02A47CDC] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A47CDC
void VindicationIllager::setAggressive(bool a1)
{
    this->AbstractIllager::setIllagerFlag(1, (a1));
    return;
}
#else
void VindicationIllager::setAggressive(bool a1)
{
    guest::stub(0x02A47CDCu, gaddr(this), (uint32_t)a1);
}
#endif

/* addAdditonalSaveData(CompoundTag*) [0x02A47CF4] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void VindicationIllager::addAdditonalSaveData(gptr<CompoundTag> a1)
{
    guest::stub(0x02A47CF4u, gaddr(this), (uint32_t)a1);
}
#else
void VindicationIllager::addAdditonalSaveData(gptr<CompoundTag> a1)
{
    guest::Mob_addAdditonalSaveData(gaddr(this), (uint32_t)a1);
    if (((uint8_t)this->field_0x730 != '\0'))
    {
        a1->putBoolean((gptr<wstring>)(0x104F14B4u), (bool)(1));
    }
    return;
}
#endif

/* getArmPose() [0x02A47D64] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void VindicationIllager::getArmPose()
{
    guest::stub(0x02A47D64u, gaddr(this));
}
#else
void VindicationIllager::getArmPose()
{
    guest::VindicationIllager_isAgressive(gaddr(this));
    return;
}
#endif

/* readAdditionalSaveData(CompoundTag*) [0x02A47D74] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void VindicationIllager::readAdditionalSaveData(gptr<CompoundTag> a1)
{
    guest::stub(0x02A47D74u, gaddr(this), (uint32_t)a1);
}
#else
void VindicationIllager::readAdditionalSaveData(gptr<CompoundTag> a1)
{
    int32_t iVar1 = 0;
    uint8_t uVar2 = 0;
    guest::Mob_readAdditionalSaveData(gaddr(this), (uint32_t)a1);
    iVar1 = a1->contains((gptr<wstring>)(0x104F14B4u), (int32_t)(99));
    if ((iVar1 != 0))
    {
        uVar2 = a1->getBoolean((gptr<wstring>)(0x104F14B4u));
        this->field_0x730 = uVar2;
    }
    return;
}
#endif

/* finalizeMobSpawn(const DifficultyInstance&, MobGroupData*, const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) [0x02A47E04] - LOWERED-DRAFT; not reconstructed (3 open edges, 9 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gptr<MobGroupData> VindicationIllager::finalizeMobSpawn(gptr<DifficultyInstance> a1, gptr<MobGroupData> a2, gptr<wstring> a3)
{
    return (gptr<MobGroupData>)(guest::stub(0x02A47E04u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3) >> 32);
}
#else
gptr<MobGroupData> VindicationIllager::finalizeMobSpawn(gptr<DifficultyInstance> a1, gptr<MobGroupData> a2, gptr<wstring> a3)
{
    uint32_t _frame = native_frame_open(32);
    uint32_t sVar1 = 0;
    gptr<MobGroupData> data = 0u;
    ST32((_frame + 24), 0);
    ST16((_frame + 4), 0);
    ST32((_frame + 28), 7);
    sVar1 = ::recon::wcslen((uint32_t)(GSTR(L"", 0x102B8040)));
    ((wstring *)gmem(_frame))->assign(GSTR(L"", 0x102B8040), (uint32_t)(sVar1));
    data = this->Mob::finalizeMobSpawn((gptr<DifficultyInstance>)(a1), (gptr<MobGroupData>)(a2), (gptr<wstring>)(_frame));
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    ST32((_frame + 24), 0);
    ST16((_frame + 4), 0);
    ST32((_frame + 28), 7);
    this->virt_populateDefaultEquipmentSlots((gptr<DifficultyInstance>)(a1));
    this->virt_populateDefaultEquipmentEnchantments((gptr<DifficultyInstance>)(a1));
    { uint32_t t1 = data;
    native_frame_close(_frame, 32);
    return t1; }
    native_frame_close(_frame, 32);
}
#endif

/* customServerAiStep() [0x02A47F08] - LOWERED-DRAFT; not reconstructed (4 open edges, 19 raw IR ops, 5 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void VindicationIllager::customServerAiStep()
{
    guest::stub(0x02A47F08u, gaddr(this));
}
#else
void VindicationIllager::customServerAiStep()
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    uint32_t piVar4 = 0;
    uint8_t bVar5 = 0;
    guest::Mob_customServerAiStep(gaddr(this));
    this->virt_getTarget(_frame);
    this->setAggressive((bool)((LD32(_frame) != 0)));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar3 = (LD32(LD32((_frame + 4))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar3), 1);
                ST32(LD32((_frame + 4)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar5 = ((iVar3 == 0) << 1);
        if ((iVar3 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            piVar4 = (LD32((_frame + 4)) + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar3 = gelem<int32_t>((uint32_t)(piVar4), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar3 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar4), 0) = iVar2;
                    bVar5 = 2;
                }
            }
            while (!((uint8_t)(((bVar5 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar3 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            }
        }
    }
    native_frame_close(_frame, 8);
    return;
    native_frame_close(_frame, 8);
}
#endif

/* isAlliedTo(boost::shared_ptr<Entity>) [0x02A47FFC] - LOWERED-DRAFT; not reconstructed (10 open edges, 50 raw IR ops, 20 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t VindicationIllager::isAlliedTo(gptr<sp_Entity> a1)
{
    return (uint32_t)(guest::stub(0x02A47FFCu, gaddr(this), (uint32_t)a1) >> 32);
}
#else
uint32_t VindicationIllager::isAlliedTo(gptr<sp_Entity> a1)
{
    uint32_t _frame = native_frame_open(16);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint8_t bVar3 = 0;
    int32_t iVar4 = 0;
    int32_t iVar5 = 0;
    uint32_t piVar6 = 0;
    uint32_t piVar7 = 0;
    uint32_t uVar8 = 0;
    uint8_t bVar9 = 0;
    uint64_t __pair = 0;
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
    iVar4 = (uint32_t)(((__pair = (uint64_t)(this->Entity::isAlliedTo((gptr<sp_Entity>)(_frame))))) >> 32);
    piVar6 = LD32((_frame + 4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar5 = (LD32(LD32((_frame + 4))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar5), 1);
                ST32(LD32((_frame + 4)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar9 = ((iVar5 == 0) << 1);
        if ((iVar5 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
            piVar7 = (piVar6 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar5 = gelem<int32_t>((uint32_t)(piVar7), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar7) + (uint32_t)(0), (iVar5 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar7), 0) = iVar2;
                    bVar9 = 2;
                }
            }
            while (!((uint8_t)(((bVar9 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar5 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)))) >> 32);
            }
        }
    }
    if ((iVar4 == 0))
    {
        bVar1 = 0;
        bVar3 = 0;
        if ((a1->field_0x0 == 0))
        {
            guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102b8044_s_boost__detail__sp_member_access_", 0x102B804C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102B80B0)), (uint32_t)(0x28d), 0u, 0u);
        }
        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((a1->field_0x0 + 0x348))) + 0xc)), (uint32_t)(a1->field_0x0)))) >> 32);
        if (((((uint32_t)__pair) & 0xc000000) == 0xc000000))
        {
            ((boost *)gmem(a1))->static_pointer_cast__tm__22_12LivingEntity6Entity((_frame + 8));
            bVar1 = 1;
            if ((LD32((_frame + 8)) == 0))
            {
                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102b8044_s_boost__detail__sp_member_access_", 0x102B80F8)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102B80B0)), (uint32_t)(0x28d), 0u, 0u);
            }
            iVar4 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((LD32((_frame + 8)) + 0x348))) + 0xa34)), (uint32_t)(LD32((_frame + 8)))))) >> 32);
            if ((iVar4 == 3))
            {
                bVar3 = 1;
            }
        }
        if ((bVar1 && (bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0)), !(bVar1))))
        {
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = (LD32(LD32((_frame + 12))) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar5 = (ST32((uint32_t)(LD32((_frame + 12))) + (uint32_t)(0), iVar4), 1);
                    ST32(LD32((_frame + 12)), iVar5);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            bVar9 = ((iVar4 == 0) << 1);
            if ((iVar4 == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 12)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 12)))))) >> 32);
                piVar6 = (LD32((_frame + 12)) + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar4 = gelem<int32_t>((uint32_t)(piVar6), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar5 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar4 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar6), 0) = iVar5;
                        bVar9 = 2;
                    }
                }
                while (!((uint8_t)(((bVar9 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar4 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 12)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 12)))))) >> 32);
                }
            }
        }
        if (!(bVar3))
        {
            { int32_t t1 = 0;
            native_frame_close(_frame, 16);
            return t1; }
        }
        uVar8 = 0;
        iVar4 = this->virt_getTeam();
        if ((iVar4 == 0))
        {
            if ((a1->field_0x0 == 0))
            {
                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102b8044_s_boost__detail__sp_member_access_", 0x102B804C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102B80B0)), (uint32_t)(0x28d), 0u, 0u);
            }
            iVar4 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((a1->field_0x0 + 0x348))) + 0x434)), (uint32_t)(a1->field_0x0)))) >> 32);
            if ((iVar4 == 0))
            {
                uVar8 = 1;
            }
        }
        { uint32_t t2 = uVar8;
        native_frame_close(_frame, 16);
        return t2; }
    }
    { int32_t t3 = 1;
    native_frame_close(_frame, 16);
    return t3; }
    native_frame_close(_frame, 16);
}
#endif

/* setCustomName(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&, const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&, bool) [0x02A48310] - boundary stub; body not reconstructed */
void VindicationIllager::setCustomName(gptr<wstring> a1, gptr<wstring> a2, bool a3)
{
    guest::stub(0x02A48310u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
}

/* getAmbientSound() [0x02A483C8] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A483C8
uint32_t VindicationIllager::getAmbientSound()
{
    return global_0x104C46C4;
}
#else
uint32_t VindicationIllager::getAmbientSound()
{
    return (uint32_t)(guest::stub(0x02A483C8u, gaddr(this)) >> 32);
}
#endif

/* getDeathSound() [0x02A483D4] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A483D4
uint32_t VindicationIllager::getDeathSound()
{
    return global_0x104C46C8;
}
#else
uint32_t VindicationIllager::getDeathSound()
{
    return (uint32_t)(guest::stub(0x02A483D4u, gaddr(this)) >> 32);
}
#endif

/* getHurtSound(const DamageSource*) [0x02A483E0] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A483E0
uint32_t VindicationIllager::getHurtSound(gptr<DamageSource> a1)
{
    return global_0x104C46CC;
}
#else
uint32_t VindicationIllager::getHurtSound(gptr<DamageSource> a1)
{
    return (uint32_t)(guest::stub(0x02A483E0u, gaddr(this), (uint32_t)a1) >> 32);
}
#endif

/* GetType() [0x02ACE788] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02ACE788
uint64_t VindicationIllager::GetType()
{
    return 0x5d54104100000000;
}
#else
uint64_t VindicationIllager::GetType()
{
    return (uint64_t)guest::stub(0x02ACE788u, gaddr(this));
}
#endif

}  /* namespace recon */
