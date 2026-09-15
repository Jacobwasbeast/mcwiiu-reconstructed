#include "game/W/Wolf.hpp"
#include "game/A/AgableMob.hpp"
#include "game/A/Animal.hpp"
#include "game/B/Block.hpp"
#include "game/B/BlockPos.hpp"
#include "game/C/CompoundTag.hpp"
#include "game/D/DamageSource.hpp"
#include "game/D/DyeColor.hpp"
#include "game/E/Entity.hpp"
#include "game/E/EntityDamageSource.hpp"
#include "game/I/ItemInstance.hpp"
#include "game/L/Level.hpp"
#include "game/L/LivingEntity.hpp"
#include "game/M/Mob.hpp"
#include "game/M/Mth.hpp"
#include "game/R/Random.hpp"
#include "game/S/SynchedEntityData.hpp"
#include "game/T/TamableAnimal.hpp"
#include "_thirdparty/boost.hpp"
#include "_thirdparty/boost__shared_ptr_Entity_.hpp"
#include "_thirdparty/boost__shared_ptr_LivingEntity_.hpp"
#include "_thirdparty/boost__shared_ptr_Player_.hpp"
#include "_thirdparty/boost__unordered__unordered_map_unsigned_long_long__int__boost__hash_unsigned_long_long___std__equal_to_unsigned_long_lo.hpp"
#include "game/N/not_null_ptr_ItemInstance_.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(Level*) [0x02A81418] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Wolf::Wolf(gptr<Level> a1)
{
    guest::stub(0x02A81418u, gaddr(this), (uint32_t)a1);
}
#else
Wolf::Wolf(gptr<Level> a1)
{
    uint32_t uVar1 = 0;
    double dVar2 = 0;
    double dVar3 = 0;
    guest::TamableAnimal_ct((uint32_t)(gaddr(this)), (uint32_t)(a1));
    ST32((uint32_t)((gaddr(this) + 0x348)), GSTR(L"", 0x102CF10C));
    ST32((uint32_t)((gaddr(this) + 0x780)), 0x102C4044u);
    ST32((uint32_t)((gaddr(this) + 8)), 0x102CFFB4u);
    this->TamableAnimal::fjDerivedCtorCalls();
    uVar1 = global_0x102B5164;
    dVar2 = (double)(global_0x102B4358);
    this->field_0x790 = 0;
    this->field_0x791 = 0;
    dVar3 = (double)(global_0x102BDE18);
    ST32((uint32_t)((gaddr(this) + 0x794)), uVar1);
    ST32((uint32_t)((gaddr(this) + 0x78c)), uVar1);
    ST32((uint32_t)((gaddr(this) + 0x788)), uVar1);
    ST32((uint32_t)((gaddr(this) + 0x798)), uVar1);
    this->AgableMob::setSize((float)(dVar2), (float)(dVar3));
}
#endif

/* {dtor}() [0x02ACD8EC] - reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Wolf::~Wolf()
{
    guest::stub(0x02ACD8ECu, gaddr(this), 0u);
}
#else
Wolf::~Wolf()
{
}
#endif

/* create(Level*) [static] [0x023E66EC] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void Wolf::create(gptr<Level> a1)
{
    guest::stub(0x023E66ECu, (uint32_t)a1);
}
#else
void Wolf::create(gptr<Level> a1)
{
    guest::Wolf_ct((uint32_t)(0), (uint32_t)(a1));
    return;
}
#endif

/* addDataWalkers(DataFixerUpper*) [static] [0x02A8115C] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A8115C
void Wolf::addDataWalkers(uint32_t a1, uint32_t a2)
{
    Mob::addDataWalkers((a1), (a2), 0, 0xd6c0002);
    return;
}
#else
void Wolf::addDataWalkers(uint32_t a1, uint32_t a2)
{
    guest::stub(0x02A8115Cu, (uint32_t)a1, (uint32_t)a2);
}
#endif

/* staticCtor() [static] [0x02A81178] - LOWERED-DRAFT; not reconstructed (3 open edges, 17 raw IR ops, 7 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Wolf::staticCtor()
{
    guest::stub(0x02A81178u);
}
#else
void Wolf::staticCtor()
{
    uint32_t _frame = native_frame_open(24);
    int32_t iVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t uVar3 = 0;
    uint32_t puVar4 = 0;
    iVar1 = global_0x104DD254;
    ST32(_frame, 0);
    ST32((_frame + 4), 0xd6c0002);
    uVar3 = SynchedEntityData::defineIdInternal((uint32_t)(0), (uint32_t)(0xd6c0002), (uint32_t)(0), (uint32_t)(0xd6c0000));
    puVar4 = (uint32_t)(((map_unsigned_long_long_int_ *)gmem(0x104F0A34u))->operator[](_frame));
    gelem<uint32_t>((uint32_t)(puVar4), 0) = uVar3;
    iVar2 = global_0x104DD264;
    if ((iVar1 == 0))
    {
        global_0x104C5214 = 0;
        ST32((_frame + 8), 0);
        ST32((_frame + 12), 0xd6c0002);
        uVar3 = SynchedEntityData::defineIdInternal((uint32_t)(0), (uint32_t)(0xd6c0002), (uint32_t)(0), (uint32_t)(0xd6c0000));
        puVar4 = (uint32_t)(((map_unsigned_long_long_int_ *)gmem(0x104F0A34u))->operator[]((_frame + 8)));
        gelem<uint32_t>((uint32_t)(puVar4), 0) = uVar3;
        iVar1 = global_0x104DD250;
    }
    else
    {
        global_0x104C5214 = (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar1 + 0x1c))) + 0x1c)), (uint32_t)(iVar1), (uint32_t)(uVar3)) >> 32);
        iVar2 = global_0x104DD264;
        ST32((_frame + 8), 0);
        ST32((_frame + 12), 0xd6c0002);
        uVar3 = SynchedEntityData::defineIdInternal((uint32_t)(0), (uint32_t)(0xd6c0002), (uint32_t)(0), (uint32_t)(0xd6c0000));
        puVar4 = (uint32_t)(((map_unsigned_long_long_int_ *)gmem(0x104F0A34u))->operator[]((_frame + 8)));
        gelem<uint32_t>((uint32_t)(puVar4), 0) = uVar3;
        iVar1 = global_0x104DD250;
    }
    global_0x104DD250 = iVar1;
    if ((iVar2 == 0))
    {
        global_0x104C5218 = 0;
        ST32((_frame + 16), 0);
        ST32((_frame + 20), 0xd6c0002);
        uVar3 = SynchedEntityData::defineIdInternal((uint32_t)(0), (uint32_t)(0xd6c0002), (uint32_t)(0), (uint32_t)(0xd6c0000));
        puVar4 = (uint32_t)(((map_unsigned_long_long_int_ *)gmem(0x104F0A34u))->operator[]((_frame + 16)));
        gelem<uint32_t>((uint32_t)(puVar4), 0) = uVar3;
    }
    else
    {
        global_0x104C5218 = (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar2 + 0x1c))) + 0x1c)), (uint32_t)(iVar2), (uint32_t)(uVar3)) >> 32);
        iVar1 = global_0x104DD250;
        ST32((_frame + 16), 0);
        ST32((_frame + 20), 0xd6c0002);
        uVar3 = SynchedEntityData::defineIdInternal((uint32_t)(0), (uint32_t)(0xd6c0002), (uint32_t)(0), (uint32_t)(0xd6c0000));
        puVar4 = (uint32_t)(((map_unsigned_long_long_int_ *)gmem(0x104F0A34u))->operator[]((_frame + 16)));
        gelem<uint32_t>((uint32_t)(puVar4), 0) = uVar3;
    }
    if ((iVar1 == 0))
    {
        global_0x104C521C = 0;
    }
    else
    {
        global_0x104C521C = (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar1 + 0x1c))) + 0x1c)), (uint32_t)(iVar1), (uint32_t)(uVar3)) >> 32);
    }
    native_frame_close(_frame, 24);
    return;
    native_frame_close(_frame, 24);
}
#endif

/* registerGoals() [0x02A815F0] - boundary stub; body not reconstructed */
void Wolf::registerGoals()
{
    guest::stub(0x02A815F0u, gaddr(this));
}

/* registerAttributes() [0x02A81AE0] - LOWERED-DRAFT; not reconstructed (11 open edges, 11 raw IR ops, 5 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Wolf::registerAttributes()
{
    guest::stub(0x02A81AE0u, gaddr(this));
}
#else
void Wolf::registerAttributes()
{
    uint32_t pcVar1 = 0;
    uint32_t piVar2 = 0;
    int32_t iVar3 = 0;
    guest::Mob_registerAttributes(gaddr(this));
    piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE1C)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102B9398)) >> 32);
    iVar3 = (uint32_t)(guest::indirect(LD32((this->field_0x348 + 0xe4c)), (uint32_t)(gaddr(this))) >> 32);
    pcVar1 = LD32((uint32_t)((this->field_0x348 + 0xa24)));
    if ((iVar3 == 0))
    {
        piVar2 = (uint32_t)((uint32_t)(guest::indirect((uint32_t)(pcVar1), (uint32_t)(gaddr(this)), (uint32_t)(global_0x104EFE10)) >> 32));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102BDE38)) >> 32);
        iVar3 = this->virt_getAttributes();
        piVar2 = (uint32_t)((uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar3 + 0x24))) + 0x24)), (uint32_t)(iVar3), (uint32_t)(global_0x104EFE24)) >> 32));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102B5EB8)) >> 32);
    }
    else
    {
        piVar2 = (uint32_t)((uint32_t)(guest::indirect((uint32_t)(pcVar1), (uint32_t)(gaddr(this)), (uint32_t)(global_0x104EFE10)) >> 32));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102BDE30)) >> 32);
        iVar3 = this->virt_getAttributes();
        piVar2 = (uint32_t)((uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar3 + 0x24))) + 0x24)), (uint32_t)(iVar3), (uint32_t)(global_0x104EFE24)) >> 32));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102B5EB8)) >> 32);
    }
    return;
}
#endif

/* setAngry(bool) [0x02A81C44] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Wolf::setAngry(bool a1)
{
    guest::stub(0x02A81C44u, gaddr(this), (uint32_t)a1);
}
#else
void Wolf::setAngry(bool a1)
{
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    iVar2 = this->field_0x1f8;
    if ((iVar2 == 0))
    {
        iVar2 = this->field_0x1f8;
    }
    bVar1 = LD8((uint32_t)((LD32((uint32_t)(((iVar2 + (LD32(LD32(0x104C4D44u)) * 4)) + 4))) + 0xc)));
    if ((a1 == 0))
    {
        if ((iVar2 == 0))
        {
            iVar2 = this->field_0x1f8;
        }
        ((SynchedEntityData *)gmem(iVar2))->set__tm__3_Uc(global_0x104C4D44, (uint32_t)((bVar1 & 0xfd)), (bool)(1));
    }
    else
    {
        if ((iVar2 == 0))
        {
            iVar2 = this->field_0x1f8;
        }
        ((SynchedEntityData *)gmem(iVar2))->set__tm__3_Uc(global_0x104C4D44, (uint32_t)((bVar1 | 2)), (bool)(1));
    }
    return;
}
#endif

/* setTarget(boost::shared_ptr<LivingEntity>) [0x02A81D68] - LOWERED-DRAFT; not reconstructed (3 open edges, 25 raw IR ops, 7 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Wolf::setTarget(gptr<sp_LivingEntity> a1)
{
    guest::stub(0x02A81D68u, gaddr(this), (uint32_t)a1);
}
#else
void Wolf::setTarget(gptr<sp_LivingEntity> a1)
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t piVar3 = 0;
    uint32_t piVar4 = 0;
    int32_t iVar5 = 0;
    uint8_t bVar6 = 0;
    ST32(_frame, a1->field_0x0);
    ST32((_frame + 4), (uint32_t)(a1->field_0x4));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    while (!(bVar1))
    {
        if (('\x01' != '\0'))
        {
            iVar5 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
            ST32(LD32((_frame + 4)), iVar5);
            bVar1 = 1;
        }
    }
    this->Mob::setTarget((gptr<sp_LivingEntity>)(_frame));
    piVar3 = LD32((_frame + 4));
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
        bVar6 = ((iVar5 == 0) << 1);
        if ((iVar5 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            piVar4 = (piVar3 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar5 = gelem<int32_t>((uint32_t)(piVar4), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar5 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar4), 0) = iVar2;
                    bVar6 = 2;
                }
            }
            while (!((uint8_t)(((bVar6 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar5 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) + 0x1c)), (uint32_t)(piVar3)) >> 32);
            }
        }
    }
    if ((a1->field_0x0 == 0))
    {
        this->setAngry((bool)(0));
    }
    else
    {
        iVar5 = (uint32_t)(guest::indirect(LD32((this->field_0x348 + 0xe4c)), (uint32_t)(gaddr(this))) >> 32);
        if ((iVar5 == 0))
        {
            this->setAngry((bool)(1));
        }
    }
    native_frame_close(_frame, 8);
    return;
    native_frame_close(_frame, 8);
}
#endif

/* customServerAiStep() [0x02A81EC8] - boundary stub; body not reconstructed */
void Wolf::customServerAiStep()
{
    guest::stub(0x02A81EC8u, gaddr(this));
}

/* defineSynchedData() [0x02A81F68] - boundary stub; body not reconstructed */
void Wolf::defineSynchedData()
{
    guest::stub(0x02A81F68u, gaddr(this));
}

/* playStepSound(const BlockPos&, Block*) [0x02A820CC] - LOWERED-DRAFT; not reconstructed (1 open edges, 1 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Wolf::playStepSound(gptr<BlockPos> a1, gptr<Block> a2)
{
    guest::stub(0x02A820CCu, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}
#else
void Wolf::playStepSound(gptr<BlockPos> a1, gptr<Block> a2)
{
    (uint32_t)(guest::indirect(LD32((this->field_0x348 + 0xf4)), (double)((double)(global_0x102B6538)), (double)((double)(global_0x102B1E18)), (uint32_t)(gaddr(this)), (uint32_t)(global_0x104C4738)) >> 32);
    return;
}
#endif

/* isAngry() [0x02A820F4] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint8_t Wolf::isAngry()
{
    return (uint8_t)(guest::stub(0x02A820F4u, gaddr(this)) >> 32);
}
#else
uint8_t Wolf::isAngry()
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return ((LD8((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C4D44u)) * 4)) + 4))) + 0xc))) >> 1) & 1);
}
#endif

/* getCollarColor() [0x02A82178] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Wolf::getCollarColor()
{
    return (uint64_t)guest::stub(0x02A82178u, gaddr(this));
}
#else
uint64_t Wolf::getCollarColor()
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return ((uint64_t)(uint32_t)(DyeColor::byItemData((int32_t)((LD32((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C521Cu)) * 4)) + 4))) + 0xc))) & 0xf)))) << 32);
}
#endif

/* addAdditonalSaveData(CompoundTag*) [0x02A8220C] - boundary stub; body not reconstructed */
void Wolf::addAdditonalSaveData(gptr<CompoundTag> a1)
{
    guest::stub(0x02A8220Cu, gaddr(this), (uint32_t)a1);
}

/* setCollarColor(const DyeColor*) [0x02A823A0] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A823A0
void Wolf::setCollarColor(gptr<DyeColor> a1)
{
    uint32_t uVar1 = 0;
    int32_t iVar2 = 0;
    iVar2 = this->field_0x1f8;
    if ((iVar2 == 0))
    {
        iVar2 = this->field_0x1f8;
    }
    uVar1 = a1->getItemData();
    ((SynchedEntityData *)gmem(iVar2))->set__tm__2_i(global_0x104C521C, ((uVar1 & 0xf)), 1);
    return;
}
#else
void Wolf::setCollarColor(gptr<DyeColor> a1)
{
    guest::stub(0x02A823A0u, gaddr(this), (uint32_t)a1);
}
#endif

/* readAdditionalSaveData(CompoundTag*) [0x02A82450] - boundary stub; body not reconstructed */
void Wolf::readAdditionalSaveData(gptr<CompoundTag> a1)
{
    guest::stub(0x02A82450u, gaddr(this), (uint32_t)a1);
}

/* getAmbientSound() [0x02A8266C] - LOWERED-DRAFT; not reconstructed (2 open edges, 5 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Wolf::getAmbientSound()
{
    return (uint32_t)(guest::stub(0x02A8266Cu, gaddr(this)) >> 32);
}
#else
uint32_t Wolf::getAmbientSound()
{
    int32_t iVar1 = 0;
    iVar1 = guest::Wolf_isAngry(gaddr(this));
    if ((iVar1 != 0))
    {
        return global_0x104C4724;
    }
    iVar1 = this->field_0x1e4->nextInt((int32_t)(3));
    if ((iVar1 == 0))
    {
        iVar1 = (uint32_t)(guest::indirect(LD32((this->field_0x348 + 0xe4c)), (uint32_t)(gaddr(this))) >> 32);
        if ((iVar1 != 0))
        {
            iVar1 = this->field_0x1f8;
            if ((iVar1 == 0))
            {
                iVar1 = this->field_0x1f8;
            }
            if ((LDF((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C5214u)) * 4)) + 4))) + 0xc))) < 10.0f))
            {
                return global_0x104C473C;
            }
        }
        return global_0x104C4730;
    }
    return global_0x104C471C;
}
#endif

/* getHurtSound(const DamageSource*) [0x02A827AC] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A827AC
uint32_t Wolf::getHurtSound(gptr<DamageSource> a1)
{
    return global_0x104C472C;
}
#else
uint32_t Wolf::getHurtSound(gptr<DamageSource> a1)
{
    return (uint32_t)(guest::stub(0x02A827ACu, gaddr(this), (uint32_t)a1) >> 32);
}
#endif

/* getDeathSound() [0x02A827B8] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A827B8
uint32_t Wolf::getDeathSound()
{
    return global_0x104C4720;
}
#else
uint32_t Wolf::getDeathSound()
{
    return (uint32_t)(guest::stub(0x02A827B8u, gaddr(this)) >> 32);
}
#endif

/* getSoundVolume() [0x02A827C4] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A827C4
double Wolf::getSoundVolume()
{
    return gref<double>(0x102B1E1Cu);
}
#else
double Wolf::getSoundVolume()
{
    guest::stub(0x02A827C4u, gaddr(this));
    return native_fret();
}
#endif

/* getDefaultLootTable() [0x02A827D0] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A827D0
uint32_t Wolf::getDefaultLootTable()
{
    return 0x104D7120u;
}
#else
uint32_t Wolf::getDefaultLootTable()
{
    return (uint32_t)(guest::stub(0x02A827D0u, gaddr(this)) >> 32);
}
#endif

/* aiStep() [0x02A827DC] - LOWERED-DRAFT; not reconstructed (17 open edges, 92 raw IR ops, 32 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Wolf::aiStep()
{
    guest::stub(0x02A827DCu, gaddr(this));
}
#else
void Wolf::aiStep()
{
    uint32_t _frame = native_frame_open(32);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint8_t bVar3 = 0;
    uint8_t bVar4 = 0;
    uint8_t cVar5 = 0;
    int32_t iVar6 = 0;
    int32_t iVar7 = 0;
    uint32_t piVar8 = 0;
    uint32_t piVar9 = 0;
    uint8_t bVar10 = 0;
    guest::Animal_aiStep(gaddr(this));
    cVar5 = LD8((uint32_t)((this->field_0xf8 + 0x168)));
    if ((((cVar5 == '\0') && ((uint8_t)this->field_0x790 != '\0')) && (LD8((uint32_t)(((int32_t)(gaddr(this)) + 0x791))) == '\0')))
    {
        iVar6 = (uint32_t)(guest::indirect(LD32((this->field_0x348 + 0xda4)), (uint32_t)(gaddr(this))) >> 32);
        iVar7 = global_0x102B5164;
        if (((iVar6 == 0) && ((uint8_t)this->field_0x15c != '\0')))
        {
            ST32(gaddr(this) + (0x1e5) * 4, global_0x102B5164);
            iVar6 = LD32((uint32_t)(this->field_0xf8));
            ST32(gaddr(this) + (0x1e6) * 4, iVar7);
            ST8((uint32_t)(((int32_t)(gaddr(this)) + 0x791)), 1);
            guest::boost_detail_shared_count_ct((uint32_t)(_frame), (uint32_t)(gaddr(&this->field_0x4)));
            ST32((_frame + 12), this->field_0x0);
            if (((uint32_t)(LD32((_frame + 12))) != gaddr(this)))
            {
                guest::boost_assertion_failed((uint32_t)(GSTR("p_get______this_102be3d0_s_boost__shared_ptr_T__boost__enab", 0x102BE3E0)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BE440)), (uint32_t)(0x32), 0u, 0u);
            }
            bVar1 = (LD32(_frame) == (uint32_t)(0x0));
            ST32((_frame + 16), LD32(_frame));
            if (!(bVar1))
            {
                do
                {
                    if (('\x01' != '\0'))
                    {
                        iVar7 = (ST32((uint32_t)(LD32(_frame)) + (uint32_t)(0), (LD32(LD32(_frame)) + 1)), 1);
                        ST32(LD32(_frame), iVar7);
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                bVar1 = (LD32(_frame) == (uint32_t)(0x0));
                if (!(bVar1))
                {
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar7 = (LD32(LD32(_frame)) + -(1));
                        if (('\x01' != '\0'))
                        {
                            iVar2 = (ST32((uint32_t)(LD32(_frame)) + (uint32_t)(0), iVar7), 1);
                            ST32(LD32(_frame), iVar2);
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    ((void)0 /* instructionSynchronize dropped */);
                    bVar10 = ((iVar7 == 0) << 1);
                    if ((iVar7 == 0))
                    {
                        (uint32_t)(guest::indirect(LD32((LD32(LD32(_frame) + (2) * 4) + 0x14)), (uint32_t)(LD32(_frame))) >> 32);
                        piVar8 = (LD32(_frame) + (1) * 4);
                        ((void)0 /* sync dropped */);
                        do
                        {
                            iVar7 = gelem<int32_t>((uint32_t)(piVar8), 0);
                            if (('\x01' != '\0'))
                            {
                                iVar2 = (ST32((uint32_t)(piVar8) + (uint32_t)(0), (iVar7 + -(1))), 1);
                                gelem<int32_t>((uint32_t)(piVar8), 0) = iVar2;
                                bVar10 = 2;
                            }
                        }
                        while (!((uint8_t)(((bVar10 >> 1) & 1))));
                        ((void)0 /* instructionSynchronize dropped */);
                        if (((iVar7 + -(1)) == 0))
                        {
                            (uint32_t)(guest::indirect(LD32((LD32(LD32(_frame) + (2) * 4) + 0x1c)), (uint32_t)(LD32(_frame))) >> 32);
                        }
                    }
                }
            }
            (uint32_t)(guest::indirect(LD32((iVar6 + 0x2c4)), (uint32_t)(this->field_0xf8), (uint32_t)((_frame + 12)), (uint32_t)(8), (uint32_t)(0)) >> 32);
            piVar8 = LD32((_frame + 16));
            bVar1 = (LD32((_frame + 16)) == (uint32_t)(0x0));
            if (!(bVar1))
            {
                ((void)0 /* sync dropped */);
                do
                {
                    iVar7 = (LD32(LD32((_frame + 16))) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar6 = (ST32((uint32_t)(LD32((_frame + 16))) + (uint32_t)(0), iVar7), 1);
                        ST32(LD32((_frame + 16)), iVar6);
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                ((void)0 /* instructionSynchronize dropped */);
                bVar10 = ((iVar7 == 0) << 1);
                if ((iVar7 == 0))
                {
                    (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 16)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 16)))) >> 32);
                    piVar9 = (piVar8 + (1) * 4);
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar7 = gelem<int32_t>((uint32_t)(piVar9), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar6 = (ST32((uint32_t)(piVar9) + (uint32_t)(0), (iVar7 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar9), 0) = iVar6;
                            bVar10 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar10 >> 1) & 1))));
                    ((void)0 /* instructionSynchronize dropped */);
                    if (((iVar7 + -(1)) == 0))
                    {
                        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar8) + 2u, 0) + 0x1c)), (uint32_t)(piVar8)) >> 32);
                    }
                }
            }
        }
        cVar5 = LD8((uint32_t)((this->field_0xf8 + 0x168)));
    }
    bVar1 = 0;
    bVar3 = 0;
    bVar4 = 0;
    if ((cVar5 == '\0'))
    {
        this->virt_getTarget((_frame + 20));
        bVar3 = 1;
        if ((LD32((_frame + 20)) != 0))
        {
            this->virt_getTarget((_frame + 4));
            bVar1 = 1;
            if ((LD32((_frame + 4)) == 0))
            {
                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102be3c8_s_boost__detail__sp_member_access_", 0x102BE494)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BE500)), (uint32_t)(0x28d), 0u, 0u);
            }
            iVar7 = (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((LD32((_frame + 4)) + 0x348))) + 0x274)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            if ((iVar7 != 0))
            {
                goto LAB_02a82af8;
            }
        }
        iVar7 = this->isAngry();
        if ((iVar7 != 0))
        {
            bVar4 = 1;
        }
    }
LAB_02a82af8:;
    if ((bVar1 && (bVar1 = (LD32((_frame + 8)) == (uint32_t)(0x0)), !(bVar1))))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar7 = (LD32(LD32((_frame + 8))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar6 = (ST32((uint32_t)(LD32((_frame + 8))) + (uint32_t)(0), iVar7), 1);
                ST32(LD32((_frame + 8)), iVar6);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar10 = ((iVar7 == 0) << 1);
        if ((iVar7 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 8)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 8)))) >> 32);
            piVar8 = (LD32((_frame + 8)) + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar7 = gelem<int32_t>((uint32_t)(piVar8), 0);
                if (('\x01' != '\0'))
                {
                    iVar6 = (ST32((uint32_t)(piVar8) + (uint32_t)(0), (iVar7 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar8), 0) = iVar6;
                    bVar10 = 2;
                }
            }
            while (!((uint8_t)(((bVar10 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar7 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 8)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 8)))) >> 32);
            }
        }
    }
    if ((bVar3 && (bVar1 = (LD32((_frame + 24)) == (uint32_t)(0x0)), !(bVar1))))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar7 = (LD32(LD32((_frame + 24))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar6 = (ST32((uint32_t)(LD32((_frame + 24))) + (uint32_t)(0), iVar7), 1);
                ST32(LD32((_frame + 24)), iVar6);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar10 = ((iVar7 == 0) << 1);
        if ((iVar7 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 24)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 24)))) >> 32);
            piVar8 = (LD32((_frame + 24)) + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar7 = gelem<int32_t>((uint32_t)(piVar8), 0);
                if (('\x01' != '\0'))
                {
                    iVar6 = (ST32((uint32_t)(piVar8) + (uint32_t)(0), (iVar7 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar8), 0) = iVar6;
                    bVar10 = 2;
                }
            }
            while (!((uint8_t)(((bVar10 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar7 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 24)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 24)))) >> 32);
            }
        }
    }
    if (bVar4)
    {
        this->setAngry((bool)(0));
    }
    native_frame_close(_frame, 32);
    return;
    native_frame_close(_frame, 32);
}
#endif

/* isInterested() [0x02A82C50] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint8_t Wolf::isInterested()
{
    return (uint8_t)(guest::stub(0x02A82C50u, gaddr(this)) >> 32);
}
#else
uint8_t Wolf::isInterested()
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    return LD8((uint32_t)((LD32((uint32_t)(((iVar1 + (LD32(LD32(0x104C5218u)) * 4)) + 4))) + 0xc)));
}
#endif

/* tick() [0x02A82CD0] - boundary stub; body not reconstructed */
void Wolf::tick()
{
    guest::stub(0x02A82CD0u, gaddr(this));
}

/* getBodyRollAngle(float, float) [0x02A83578] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A83578
double Wolf::getBodyRollAngle(float a1, float a2)
{
    float fVar1 = 0;
    double dVar3 = 0;
    double dVar4 = 0;
    fVar1 = ((float)(((double)((float)((((double)((float)(((double)(this->field_0x794) - (double)(this->field_0x798)))) * a1) + (double)(this->field_0x798)))) + a2)) / 1.7999999523162842f);
    if ((0.0f <= fVar1))
    {
        if ((1.0f < fVar1))
        {
            fVar1 = gref<float>(0x102B1E18u);
        }
        fVar1 = (fVar1 * 3.1415927410125732f);
        dVar3 = (double)(Mth::sin(((double)(fVar1))));
        dVar4 = (double)(Mth::sin(((double)((float)(((double)(fVar1) * gref<double>(0x102BE61Cu)))))));
        fVar1 = ((float)((dVar3 * dVar4)) * gref<float>(0x102B6538u));
    }
    else
    {
        fVar1 = (global_0x102B5164 * gref<float>(0x102B2FB8u));
        dVar3 = (double)(Mth::sin(((double)(fVar1))));
        dVar4 = (double)(Mth::sin(((double)((float)(((double)(fVar1) * gref<double>(0x102BE61Cu)))))));
        fVar1 = ((float)((dVar3 * dVar4)) * gref<float>(0x102B6538u));
    }
    { double t1 = (double)((fVar1 * gref<float>(0x102B2FB8u)));
    return t1; }
}
#else
double Wolf::getBodyRollAngle(float a1, float a2)
{
    guest::stub(0x02A83578u, gaddr(this), (double)a1, (double)a2);
    return native_fret();
}
#endif

/* getHeadRollAngle(float) [0x02A836C4] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A836C4
double Wolf::getHeadRollAngle(float a1)
{
    return (double)((((float)((((double)((float)(((double)(this->field_0x788) - (double)(this->field_0x78c)))) * a1) + (double)(this->field_0x78c))) * gref<float>(0x102B6538u)) * 3.1415927410125732f));
}
#else
double Wolf::getHeadRollAngle(float a1)
{
    guest::stub(0x02A836C4u, gaddr(this), (double)a1);
    return native_fret();
}
#endif

/* getEyeHeight() [0x02A836F0] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A836F0
double Wolf::getEyeHeight()
{
    return (double)((this->field_0x19c * 0.800000011920929f));
}
#else
double Wolf::getEyeHeight()
{
    guest::stub(0x02A836F0u, gaddr(this));
    return native_fret();
}
#endif

/* getMaxHeadXRot() [0x02A83704] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A83704
uint32_t Wolf::getMaxHeadXRot()
{
    int32_t iVar1 = 0;
    uint32_t uVar2 = 0;
    uVar2 = 0x14;
    iVar1 = this->virt___ghs_thunk__0xfffff87c__handleStartJump();
    if ((iVar1 == 0))
    {
        uVar2 = this->Mob::getMaxHeadXRot();
    }
    return uVar2;
}
#else
uint32_t Wolf::getMaxHeadXRot()
{
    return (uint32_t)(guest::stub(0x02A83704u, gaddr(this)) >> 32);
}
#endif

/* hurt(const DamageSource*, float) [0x02A8376C] - boundary stub; body not reconstructed */
uint32_t Wolf::hurt(double a1, float a2)
{
    return (uint32_t)(guest::stub(0x02A8376Cu, gaddr(this), (uint32_t)a2, (double)a1) >> 32);
}

/* doHurtTarget(boost::shared_ptr<Entity>) [0x02A83CD4] - LOWERED-DRAFT; not reconstructed (24 open edges, 177 raw IR ops, 47 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
int32_t Wolf::doHurtTarget(gptr<sp_Entity> a1)
{
    return (int32_t)(guest::stub(0x02A83CD4u, gaddr(this), (uint32_t)a1) >> 32);
}
#else
int32_t Wolf::doHurtTarget(gptr<sp_Entity> a1)
{
    uint32_t _frame = native_frame_open(64);
    uint8_t bVar1 = 0;
    uint32_t piVar2 = 0;
    uint32_t extraout_r4 = 0;
    uint32_t extraout_r4_00 = 0;
    uint32_t uVar3 = 0;
    uint32_t piVar4 = 0;
    int32_t iVar5 = 0;
    int32_t iVar6 = 0;
    int32_t iVar7 = 0;
    uint8_t bVar8 = 0;
    double dVar9 = 0;
    gptr<EntityDamageSource> source = 0u;
    uint64_t __pair = 0;
    guest::boost_detail_shared_count_ct((uint32_t)(_frame), (uint32_t)(gaddr(&this->field_0x4)));
    ST32((_frame + 28), this->field_0x0);
    uVar3 = 0u /* unmodelled extraout_r4 */;
    if (((uint32_t)(LD32((_frame + 28))) != gaddr(this)))
    {
        guest::boost_assertion_failed((uint32_t)(GSTR("p_get______this_102be740_s_boost__shared_ptr_T__boost__enab", 0x102BE750)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BE7B0)), (uint32_t)(0x32), 0u, 0u);
        uVar3 = 0u /* unmodelled extraout_r4_00 */;
    }
    bVar1 = (LD32(_frame) == (uint32_t)(0x0));
    ST32((_frame + 32), LD32(_frame));
    if (!(bVar1))
    {
        do
        {
            if (('\x01' != '\0'))
            {
                iVar5 = (ST32((uint32_t)(LD32(_frame)) + (uint32_t)(0), (LD32(LD32(_frame)) + 1)), 1);
                ST32(LD32(_frame), iVar5);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        bVar1 = (LD32(_frame) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            ((void)0 /* sync dropped */);
            do
            {
                iVar5 = (LD32(LD32(_frame)) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar6 = (ST32((uint32_t)(LD32(_frame)) + (uint32_t)(0), iVar5), 1);
                    ST32(LD32(_frame), iVar6);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            bVar8 = ((iVar5 == 0) << 1);
            if ((iVar5 == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32(_frame) + (2) * 4) + 0x14)), (uint32_t)(LD32(_frame)), (uint32_t)(uVar3), (uint32_t)(LD32(_frame))))) >> 32);
                piVar2 = (LD32(_frame) + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar5 = gelem<int32_t>((uint32_t)(piVar2), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar6 = (ST32((uint32_t)(piVar2) + (uint32_t)(0), (iVar5 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar2), 0) = iVar6;
                        bVar8 = 2;
                    }
                }
                while (!((uint8_t)(((bVar8 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar5 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32(_frame) + (2) * 4) + 0x1c)), (uint32_t)(LD32(_frame))))) >> 32);
                }
            }
        }
    }
    ((boost *)gmem((_frame + 28)))->static_pointer_cast__tm__12_3Mob6Entity((_frame + 20));
    ST32((_frame + 12), LD32((_frame + 20)));
    bVar1 = (LD32((_frame + 24)) == (uint32_t)(0x0));
    ST32((_frame + 16), LD32((_frame + 24)));
    while (!(bVar1))
    {
        if (('\x01' != '\0'))
        {
            iVar5 = (ST32((uint32_t)(LD32((_frame + 24))) + (uint32_t)(0), (LD32(LD32((_frame + 24))) + 1)), 1);
            ST32(LD32((_frame + 24)), iVar5);
            bVar1 = 1;
        }
    }
    source = DamageSource::CreateMobAttack((gptr<sp_LivingEntity>)((_frame + 12)));
    piVar2 = LD32((_frame + 16));
    iVar5 = (int32_t)(((uint64_t)(source) >> 0x20));
    uVar3 = (uint32_t)(source);
    bVar1 = (LD32((_frame + 16)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar6 = (LD32(LD32((_frame + 16))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar7 = (ST32((uint32_t)(LD32((_frame + 16))) + (uint32_t)(0), iVar6), 1);
                ST32(LD32((_frame + 16)), iVar7);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar8 = ((iVar6 == 0) << 1);
        if ((iVar6 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 16)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 16))), (uint32_t)(uVar3), (uint32_t)(LD32((_frame + 16)))))) >> 32);
            piVar4 = (piVar2 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar6 = gelem<int32_t>((uint32_t)(piVar4), 0);
                if (('\x01' != '\0'))
                {
                    iVar7 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar6 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar4), 0) = iVar7;
                    bVar8 = 2;
                }
            }
            while (!((uint8_t)(((bVar8 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            uVar3 = ((uint32_t)__pair);
            if (((iVar6 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2) + 2u, 0) + 0x1c)), (uint32_t)(piVar2)))) >> 32);
                uVar3 = ((uint32_t)__pair);
            }
        }
    }
    bVar1 = (LD32((_frame + 24)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar6 = (LD32(LD32((_frame + 24))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar7 = (ST32((uint32_t)(LD32((_frame + 24))) + (uint32_t)(0), iVar6), 1);
                ST32(LD32((_frame + 24)), iVar7);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar8 = ((iVar6 == 0) << 1);
        if ((iVar6 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 24)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 24)))))) >> 32);
            piVar2 = (LD32((_frame + 24)) + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar6 = gelem<int32_t>((uint32_t)(piVar2), 0);
                if (('\x01' != '\0'))
                {
                    iVar7 = (ST32((uint32_t)(piVar2) + (uint32_t)(0), (iVar6 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar2), 0) = iVar7;
                    bVar8 = 2;
                }
            }
            while (!((uint8_t)(((bVar8 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            uVar3 = ((uint32_t)__pair);
            if (((iVar6 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 24)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 24)))))) >> 32);
                uVar3 = ((uint32_t)__pair);
            }
        }
    }
    piVar2 = LD32((_frame + 32));
    bVar1 = (LD32((_frame + 32)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar6 = (LD32(LD32((_frame + 32))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar7 = (ST32((uint32_t)(LD32((_frame + 32))) + (uint32_t)(0), iVar6), 1);
                ST32(LD32((_frame + 32)), iVar7);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar8 = ((iVar6 == 0) << 1);
        if ((iVar6 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 32)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 32))), (uint32_t)(uVar3), (uint32_t)(LD32((_frame + 32)))))) >> 32);
            piVar4 = (piVar2 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar6 = gelem<int32_t>((uint32_t)(piVar4), 0);
                if (('\x01' != '\0'))
                {
                    iVar7 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar6 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar4), 0) = iVar7;
                    bVar8 = 2;
                }
            }
            while (!((uint8_t)(((bVar8 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar6 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2) + 2u, 0) + 0x1c)), (uint32_t)(piVar2)))) >> 32);
            }
        }
    }
    if ((a1->field_0x0 == 0))
    {
        guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102be738_s_boost__detail__sp_member_access_", 0x102BE804)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BE868)), (uint32_t)(0x28d), 0u, 0u);
    }
    iVar6 = a1->field_0x0;
    iVar7 = LD32((uint32_t)((iVar6 + 0x348)));
    piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE24)));
    dVar9 = (double)((uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x2c))))) >> 32));
    iVar6 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar7 + 0x1dc)), (double)((double)((float)(dVar9))), (uint32_t)(iVar6), (uint32_t)(iVar5)))) >> 32);
    if ((iVar5 != 0))
    {
        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((iVar5 + 0x18))) + 0xc)), (uint32_t)(iVar5), (uint32_t)(3)))) >> 32);
    }
    if ((iVar6 != 0))
    {
        guest::boost_detail_shared_count_ct((uint32_t)((_frame + 8)), (uint32_t)(gaddr(&this->field_0x4)));
        ST32((_frame + 4), (uint32_t)(this->field_0x0));
        if ((LD32((_frame + 4)) != gaddr(this)))
        {
            guest::boost_assertion_failed((uint32_t)(GSTR("p_get______this_102be740_s_boost__shared_ptr_T__boost__enab", 0x102BE750)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BE7B0)), (uint32_t)(0x32), 0u, 0u);
        }
        bVar1 = (LD32((_frame + 8)) == (uint32_t)(0x0));
        ST32((_frame + 40), LD32((_frame + 8)));
        ST32((_frame + 36), LD32((_frame + 4)));
        if (!(bVar1))
        {
            do
            {
                if (('\x01' != '\0'))
                {
                    iVar5 = (ST32((uint32_t)(LD32((_frame + 8))) + (uint32_t)(0), (LD32(LD32((_frame + 8))) + 1)), 1);
                    ST32(LD32((_frame + 8)), iVar5);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            bVar1 = (LD32((_frame + 8)) == (uint32_t)(0x0));
            if (!(bVar1))
            {
                ((void)0 /* sync dropped */);
                do
                {
                    iVar5 = (LD32(LD32((_frame + 8))) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar7 = (ST32((uint32_t)(LD32((_frame + 8))) + (uint32_t)(0), iVar5), 1);
                        ST32(LD32((_frame + 8)), iVar7);
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                ((void)0 /* instructionSynchronize dropped */);
                bVar8 = ((iVar5 == 0) << 1);
                if ((iVar5 == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 8)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 8)))))) >> 32);
                    piVar2 = (LD32((_frame + 8)) + (1) * 4);
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar5 = gelem<int32_t>((uint32_t)(piVar2), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar7 = (ST32((uint32_t)(piVar2) + (uint32_t)(0), (iVar5 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar2), 0) = iVar7;
                            bVar8 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar8 >> 1) & 1))));
                    ((void)0 /* instructionSynchronize dropped */);
                    if (((iVar5 + -(1)) == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 8)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 8)))))) >> 32);
                    }
                }
            }
        }
        ((boost *)gmem((_frame + 36)))->static_pointer_cast__tm__22_12LivingEntity6Entity((_frame + 52));
        ST32((_frame + 44), a1->field_0x0);
        ST32((_frame + 48), (uint32_t)(a1->field_0x4));
        bVar1 = (LD32((_frame + 48)) == (uint32_t)(0x0));
        while (!(bVar1))
        {
            if (('\x01' != '\0'))
            {
                iVar5 = (ST32((uint32_t)(LD32((_frame + 48))) + (uint32_t)(0), (LD32(LD32((_frame + 48))) + 1)), 1);
                ST32(LD32((_frame + 48)), iVar5);
                bVar1 = 1;
            }
        }
        this->Entity::doEnchantDamageEffects((gptr<sp_LivingEntity>)((_frame + 52)), (gptr<sp_Entity>)((_frame + 44)));
        piVar2 = LD32((_frame + 48));
        bVar1 = (LD32((_frame + 48)) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            ((void)0 /* sync dropped */);
            do
            {
                iVar5 = (LD32(LD32((_frame + 48))) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar7 = (ST32((uint32_t)(LD32((_frame + 48))) + (uint32_t)(0), iVar5), 1);
                    ST32(LD32((_frame + 48)), iVar7);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            bVar8 = ((iVar5 == 0) << 1);
            if ((iVar5 == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 48)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 48)))))) >> 32);
                piVar4 = (piVar2 + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar5 = gelem<int32_t>((uint32_t)(piVar4), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar7 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar5 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar4), 0) = iVar7;
                        bVar8 = 2;
                    }
                }
                while (!((uint8_t)(((bVar8 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar5 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2) + 2u, 0) + 0x1c)), (uint32_t)(piVar2), (uint32_t)(((uint32_t)__pair)), (uint32_t)(piVar4)))) >> 32);
                }
            }
        }
        bVar1 = (LD32((_frame + 56)) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            ((void)0 /* sync dropped */);
            do
            {
                iVar5 = (LD32(LD32((_frame + 56))) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar7 = (ST32((uint32_t)(LD32((_frame + 56))) + (uint32_t)(0), iVar5), 1);
                    ST32(LD32((_frame + 56)), iVar7);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            bVar8 = ((iVar5 == 0) << 1);
            if ((iVar5 == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 56)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 56)))))) >> 32);
                piVar2 = (LD32((_frame + 56)) + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar5 = gelem<int32_t>((uint32_t)(piVar2), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar7 = (ST32((uint32_t)(piVar2) + (uint32_t)(0), (iVar5 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar2), 0) = iVar7;
                        bVar8 = 2;
                    }
                }
                while (!((uint8_t)(((bVar8 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar5 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 56)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 56)))))) >> 32);
                }
            }
        }
        piVar2 = LD32((_frame + 40));
        bVar1 = (LD32((_frame + 40)) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            ((void)0 /* sync dropped */);
            do
            {
                iVar5 = (LD32(LD32((_frame + 40))) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar7 = (ST32((uint32_t)(LD32((_frame + 40))) + (uint32_t)(0), iVar5), 1);
                    ST32(LD32((_frame + 40)), iVar7);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            bVar8 = ((iVar5 == 0) << 1);
            if ((iVar5 == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 40)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 40)))))) >> 32);
                piVar4 = (piVar2 + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar5 = gelem<int32_t>((uint32_t)(piVar4), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar7 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar5 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar4), 0) = iVar7;
                        bVar8 = 2;
                    }
                }
                while (!((uint8_t)(((bVar8 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar5 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2) + 2u, 0) + 0x1c)), (uint32_t)(piVar2)))) >> 32);
                }
            }
        }
    }
    { int32_t t1 = iVar6;
    native_frame_close(_frame, 64);
    return t1; }
    native_frame_close(_frame, 64);
}
#endif

/* setTame(bool) [0x02A84390] - LOWERED-DRAFT; not reconstructed (7 open edges, 5 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Wolf::setTame(bool a1)
{
    guest::stub(0x02A84390u, gaddr(this), (uint32_t)a1);
}
#else
void Wolf::setTame(bool a1)
{
    uint32_t pcVar1 = 0;
    uint32_t piVar2 = 0;
    guest::TamableAnimal_setTame(gaddr(this), (uint32_t)a1);
    pcVar1 = LD32((uint32_t)((this->field_0x348 + 0xa24)));
    if ((a1 == 0))
    {
        piVar2 = (uint32_t)((uint32_t)(guest::indirect((uint32_t)(pcVar1), (uint32_t)(gaddr(this)), (uint32_t)(global_0x104EFE10)) >> 32));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102BDE38)) >> 32);
        piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE24)));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102B77A8)) >> 32);
    }
    else
    {
        piVar2 = (uint32_t)((uint32_t)(guest::indirect((uint32_t)(pcVar1), (uint32_t)(gaddr(this)), (uint32_t)(global_0x104EFE10)) >> 32));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102BDE30)) >> 32);
        piVar2 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE24)));
        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2), 0) + 0x24)), (uint32_t)(global_0x102B77A8)) >> 32);
    }
    return;
}
#endif

/* tame(boost::shared_ptr<Player>, bool, bool) [0x02A84498] - boundary stub; body not reconstructed */
uint64_t Wolf::tame(gptr<sp_Player> a1, bool a2, bool a3)
{
    return (uint64_t)guest::stub(0x02A84498u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
}

/* mobInteract(const boost::shared_ptr<Player>&, InteractionHand::EInteractionHand) [0x02A84690] - boundary stub; body not reconstructed */
uint32_t Wolf::mobInteract(gptr<sp_Player> a1, uint32_t a2)
{
    return (uint32_t)(guest::stub(0x02A84690u, gaddr(this), (uint32_t)a1, (uint32_t)a2) >> 32);
}

/* handleEntityEvent(unsigned char, int) [0x02A85D80] - boundary stub; body not reconstructed */
void Wolf::handleEntityEvent(uint8_t a1, int32_t a2)
{
    guest::stub(0x02A85D80u, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}

/* getTailAngle() [0x02A85DB4] - LOWERED-DRAFT; not reconstructed (3 open edges, 7 raw IR ops, 4 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
double Wolf::getTailAngle()
{
    guest::stub(0x02A85DB4u, gaddr(this));
    return native_fret();
}
#else
double Wolf::getTailAngle()
{
    uint32_t _frame = native_frame_open(32);
    float fVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    double dVar4 = 0;
    iVar2 = guest::Wolf_isAngry(gaddr(this));
    if ((iVar2 != 0))
    {
        { double t1 = (double)(global_0x102BEB6C);
        native_frame_close(_frame, 32);
        return t1; }
    }
    iVar2 = (uint32_t)(guest::indirect(LD32((this->field_0x348 + 0xe4c)), (uint32_t)(gaddr(this))) >> 32);
    if ((iVar2 != 0))
    {
        iVar2 = this->field_0x1f8;
        iVar3 = this->field_0x348;
        if ((iVar2 == 0))
        {
            iVar2 = this->field_0x1f8;
        }
        fVar1 = LDF((uint32_t)((LD32((uint32_t)(((iVar2 + (LD32(LD32(0x104C5214u)) * 4)) + 4))) + 0xc)));
        dVar4 = (double)((uint32_t)(guest::indirect(LD32((iVar3 + 0x9fc)), (uint32_t)(gaddr(this))) >> 32));
        { double t2 = (double)((-((((float)((dVar4 - (double)(fVar1))) * gref<float>(0x102B91BCu)) - 0.550000011920929f)) * 3.1415927410125732f));
        native_frame_close(_frame, 32);
        return t2; }
    }
    { double t3 = (double)(global_0x102BEB74);
    native_frame_close(_frame, 32);
    return t3; }
    native_frame_close(_frame, 32);
}
#endif

/* isFood(const not_null_ptr<ItemInstance>&) [0x02A85F1C] - LOWERED-DRAFT; not reconstructed (3 open edges, 4 raw IR ops, 4 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Wolf::isFood(gptr<not_null_ptr_ItemInstance> a1)
{
    return (uint32_t)(guest::stub(0x02A85F1Cu, gaddr(this), (uint32_t)a1) >> 32);
}
#else
uint32_t Wolf::isFood(gptr<not_null_ptr_ItemInstance> a1)
{
    int32_t iVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t uVar3 = 0;
    if ((a1->field_0x0 == 0))
    {
    }
    iVar1 = ((ItemInstance *)gmem(a1->field_0x0))->getItem();
    if (((iVar1 != 0) && (iVar1 = ::recon::__dynamic_cast((uint32_t)(iVar1), (uint32_t)(LD32((uint32_t)((iVar1 + 0x74)))), (int32_t)(0x1011DBFCu), (uint32_t)(iVar1), (int32_t)(0x1014DB38u)), (iVar1 != 0))))
    {
        if ((a1->field_0x0 == 0))
        {
        }
        iVar2 = ((ItemInstance *)gmem(a1->field_0x0))->getItem();
        iVar1 = 0;
        if ((iVar2 != 0))
        {
            iVar1 = ::recon::__dynamic_cast((uint32_t)(iVar2), (uint32_t)(LD32((uint32_t)((iVar2 + 0x74)))), (int32_t)(0x1011DBFCu), (uint32_t)(iVar2), (int32_t)(0x1014DB38u));
        }
        uVar3 = (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar1 + 0x74))) + 0x1ac)), (uint32_t)(iVar1)) >> 32);
        return uVar3;
    }
    return 0;
}
#endif

/* getMaxSpawnClusterSize() [0x02A86058] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A86058
uint32_t Wolf::getMaxSpawnClusterSize()
{
    return 4;
}
#else
uint32_t Wolf::getMaxSpawnClusterSize()
{
    return (uint32_t)(guest::stub(0x02A86058u, gaddr(this)) >> 32);
}
#endif

/* GetSynchedHealth() [0x02A86060] - LOWERED-DRAFT; not reconstructed (3 open edges, 25 raw IR ops, 10 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
int32_t Wolf::GetSynchedHealth()
{
    return (int32_t)(guest::stub(0x02A86060u, gaddr(this)) >> 32);
}
#else
int32_t Wolf::GetSynchedHealth()
{
    uint32_t _frame = native_frame_open(16);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    uint32_t piVar4 = 0;
    int32_t iVar5 = 0;
    uint8_t bVar6 = 0;
    this->Entity::getEntityData(_frame);
    if ((LD32(_frame) == 0))
    {
        guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102becf4_s_boost__detail__sp_member_access_", 0x102BECFC)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BED6C)), (uint32_t)(0x28d), 0u, 0u);
    }
    iVar2 = (int32_t)(LDF((uint32_t)((LD32((uint32_t)(((LD32(_frame) + (LD32(LD32(0x104C5214u)) * 4)) + 4))) + 0xc))));
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar5 = (LD32(LD32((_frame + 4))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar3 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar5), 1);
                ST32(LD32((_frame + 4)), iVar3);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar6 = ((iVar5 == 0) << 1);
        if ((iVar5 == 0))
        {
            ST32((_frame + 8), iVar2);
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            piVar4 = (LD32((_frame + 4)) + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar5 = gelem<int32_t>((uint32_t)(piVar4), 0);
                if (('\x01' != '\0'))
                {
                    iVar3 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar5 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar4), 0) = iVar3;
                    bVar6 = 2;
                }
            }
            while (!((uint8_t)(((bVar6 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar5 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 4)))) >> 32);
            }
        }
    }
    { int32_t t1 = iVar2;
    native_frame_close(_frame, 16);
    return t1; }
    native_frame_close(_frame, 16);
}
#endif

/* GetInteractTooltip(const EntityTooltipDataHolder&) [0x02A86190] - boundary stub; body not reconstructed */
uint32_t Wolf::GetInteractTooltip(uint32_t a1, uint32_t a2)
{
    return (uint32_t)(guest::stub(0x02A86190u, gaddr(this), (uint32_t)a1, (uint32_t)a2) >> 32);
}

/* getBreedOffspring(boost::shared_ptr<AgableMob>) [0x02A87034] - LOWERED-DRAFT; not reconstructed (17 open edges, 45 raw IR ops, 19 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Wolf::getBreedOffspring(uint32_t a1)
{
    return (uint64_t)guest::stub(0x02A87034u, gaddr(this), (uint32_t)a1);
}
#else
uint64_t Wolf::getBreedOffspring(uint32_t a1)
{
    uint32_t _frame = native_frame_open(72);
    int32_t iVar1 = 0;
    uint32_t piVar2 = 0;
    uint32_t piVar3 = 0;
    uint32_t uVar4 = 0;
    int32_t iVar5 = 0;
    int32_t iVar6 = 0;
    uint32_t piVar7 = 0;
    uint32_t puVar8 = 0;
    uint8_t bVar9 = 0;
    uint8_t bVar10 = 0;
    uint64_t uVar11 = 0;
    uint64_t __pair = 0;
    iVar1 = guest::Wolf_ct((uint32_t)(0), (uint32_t)(this->field_0xf8));
    piVar2 = (uint32_t)(guest::guest_new((uint32_t)(0x10)));
    if ((piVar2 != (uint32_t)(0x0)))
    {
        piVar3 = piVar2;
        if (((piVar2 != (uint32_t)(0x0)) || (piVar3 = (uint32_t)(guest::guest_new((uint32_t)(0xc))), (piVar3 != (uint32_t)(0x0)))))
        {
            gelem<int32_t>((uint32_t)(piVar3) + 1u, 0) = 1;
            gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) = (int32_t)(GSTR(L"", 0x102AAB8C));
            gelem<int32_t>((uint32_t)(piVar3), 0) = 1;
        }
        gelem<int32_t>((uint32_t)(piVar2) + 3u, 0) = iVar1;
        gelem<int32_t>((uint32_t)(piVar2) + 2u, 0) = (int32_t)(GSTR(L"", 0x102AB06C));
    }
    if ((piVar2 == (uint32_t)(0x0)))
    {
        if ((iVar1 != 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((iVar1 + 0x348))) + 0x14)), (uint32_t)(iVar1), (uint32_t)(3)))) >> 32);
        }
        ST32(_frame, GSTR("unknown", 0x102AAA80));
        ST32((_frame + 4), GSTR(L"", 0x102AAAF4));
        guest::boost_throw_exception((uint32_t)(_frame), 0u);
    }
    if (((iVar1 != 0) && ((LD32((uint32_t)((iVar1 + 4))) == (uint32_t)(0x0)) || (LD32(LD32((uint32_t)((iVar1 + 4)))) == 0))))
    {
        bVar9 = (piVar2 == (uint32_t)(0x0));
        while (!(bVar9))
        {
            if (('\x01' != '\0'))
            {
                iVar6 = (ST32((uint32_t)(piVar2) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar2), 0) + 1)), 1);
                gelem<int32_t>((uint32_t)(piVar2), 0) = iVar6;
                bVar9 = 1;
            }
        }
        puVar8 = (uint32_t)((iVar1 + 4));
        piVar3 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar8), 0));
        ST32((uint32_t)(iVar1), iVar1);
        if ((piVar2 != piVar3))
        {
            bVar9 = (piVar2 == (uint32_t)(0x0));
            if (!(bVar9))
            {
                piVar3 = (piVar2 + (1) * 4);
                do
                {
                    if (('\x01' != '\0'))
                    {
                        iVar6 = (ST32((uint32_t)(piVar3) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar3), 0) + 1)), 1);
                        gelem<int32_t>((uint32_t)(piVar3), 0) = iVar6;
                        bVar9 = 1;
                    }
                }
                while (!(bVar9));
                piVar3 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar8), 0));
            }
            bVar9 = (piVar3 == (uint32_t)(0x0));
            if (!(bVar9))
            {
                piVar7 = (piVar3 + (1) * 4);
                do
                {
                    iVar6 = gelem<int32_t>((uint32_t)(piVar7), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar5 = (ST32((uint32_t)(piVar7) + (uint32_t)(0), (iVar6 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar7), 0) = iVar5;
                        bVar9 = 1;
                    }
                }
                while (!(bVar9));
                if (((iVar6 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) + 0x1c)), (uint32_t)(piVar3)))) >> 32);
                }
            }
            gelem<uint32_t>((uint32_t)(puVar8), 0) = piVar2;
        }
        bVar9 = (piVar2 == (uint32_t)(0x0));
        if (!(bVar9))
        {
            do
            {
                iVar6 = (gelem<int32_t>((uint32_t)(piVar2), 0) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar5 = (ST32((uint32_t)(piVar2) + (uint32_t)(0), iVar6), 1);
                    gelem<int32_t>((uint32_t)(piVar2), 0) = iVar5;
                    bVar9 = 1;
                }
            }
            while (!(bVar9));
            bVar10 = ((iVar6 == 0) << 1);
            if ((iVar6 == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2) + 2u, 0) + 0x14)), (uint32_t)(piVar2)))) >> 32);
                piVar3 = (piVar2 + (1) * 4);
                do
                {
                    iVar6 = gelem<int32_t>((uint32_t)(piVar3), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar5 = (ST32((uint32_t)(piVar3) + (uint32_t)(0), (iVar6 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar3), 0) = iVar5;
                        bVar10 = 2;
                    }
                }
                while (!((uint8_t)(((bVar10 >> 1) & 1))));
                if (((iVar6 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2) + 2u, 0) + 0x1c)), (uint32_t)(piVar2)))) >> 32);
                }
            }
        }
    }
    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((this->field_0x348 + 0xe74)), (uint32_t)(gaddr(this)), (uint32_t)((_frame + 8))))) >> 32);
    iVar6 = LD32((_frame + 32));
    bVar9 = (LD32((_frame + 32)) == 0);
    iVar5 = (LD32((_frame + 32)) + -(1));
    if ((7 < LD32((_frame + 36))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 28))));
    }
    ST32((_frame + 32), 0);
    ST32((_frame + 36), 7);
    if ((iVar6 != (iVar5 + (uint32_t)(bVar9))))
    {
        if ((iVar1 == 0))
        {
        }
        iVar6 = LD32((uint32_t)((iVar1 + 0x348)));
        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((this->field_0x348 + 0xe74)), (uint32_t)(gaddr(this)), (uint32_t)((_frame + 40))))) >> 32);
        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar6 + 0xe7c)), (uint32_t)(iVar1), (uint32_t)((_frame + 40))))) >> 32);
        if ((7 < LD32((_frame + 68))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 60))));
        }
        ST32((_frame + 64), 0);
        ST16((_frame + 44), 0);
        ST32((_frame + 68), 7);
        if ((iVar1 == 0))
        {
        }
        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((iVar1 + 0x348))) + 0xe54)), (uint32_t)(iVar1), (uint32_t)(1)))) >> 32);
        if ((iVar1 == 0))
        {
        }
        iVar6 = LD32((uint32_t)((iVar1 + 0x348)));
        if ((iVar1 == 0))
        {
        }
        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((iVar1 + 0x348))) + 0x9fc)), (uint32_t)(iVar1)))) >> 32);
        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar6 + 0x934)), (uint32_t)(iVar1)))) >> 32);
        if ((iVar1 == 0))
        {
        }
        uVar4 = (uint32_t)(((__pair = (uint64_t)(this->getCollarColor()))) >> 32);
        ((Wolf *)gmem(iVar1))->setCollarColor((gptr<DyeColor>)(uVar4));
    }
    uVar11 = CONCAT44((a1 + (1) * 4), uVar4);
    gelem<int32_t>((uint32_t)(a1), 0) = iVar1;
    if (((a1 + (1) * 4) == (uint32_t)(0x0)))
    {
        uVar11 = guest::guest_new((uint32_t)(4));
        uVar4 = (uint32_t)(uVar11);
        if (((int32_t)(((uint64_t)(uVar11) >> 0x20)) == 0))
        {
            goto LAB_02a875c4;
        }
    }
    uVar4 = (uint32_t)(uVar11);
    bVar9 = (piVar2 == (uint32_t)(0x0));
    ST32((uint32_t)(((uint64_t)(uVar11) >> 0x20)), piVar2);
    while (!(bVar9))
    {
        if (('\x01' != '\0'))
        {
            iVar1 = (ST32((uint32_t)(piVar2) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar2), 0) + 1)), 1);
            gelem<int32_t>((uint32_t)(piVar2), 0) = iVar1;
            bVar9 = 1;
        }
    }
LAB_02a875c4:;
    bVar9 = (piVar2 == (uint32_t)(0x0));
    if (!(bVar9))
    {
        do
        {
            iVar1 = (gelem<int32_t>((uint32_t)(piVar2), 0) + -(1));
            if (('\x01' != '\0'))
            {
                iVar6 = (ST32((uint32_t)(piVar2) + (uint32_t)(0), iVar1), 1);
                gelem<int32_t>((uint32_t)(piVar2), 0) = iVar6;
                bVar9 = 1;
            }
        }
        while (!(bVar9));
        bVar10 = ((iVar1 == 0) << 1);
        if ((iVar1 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2) + 2u, 0) + 0x14)), (uint32_t)(piVar2), (uint32_t)(uVar4), (uint32_t)(0), (uint32_t)(piVar2), (uint32_t)(piVar2)))) >> 32);
            piVar3 = (piVar2 + (1) * 4);
            do
            {
                iVar1 = gelem<int32_t>((uint32_t)(piVar3), 0);
                if (('\x01' != '\0'))
                {
                    iVar6 = (ST32((uint32_t)(piVar3) + (uint32_t)(0), (iVar1 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar3), 0) = iVar6;
                    bVar10 = 2;
                }
            }
            while (!((uint8_t)(((bVar10 >> 1) & 1))));
            if (((iVar1 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar2) + 2u, 0) + 0x1c)), (uint32_t)(piVar2)))) >> 32);
            }
        }
    }
    { uint64_t t1 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
    native_frame_close(_frame, 72);
    return t1; }
    native_frame_close(_frame, 72);
}
#endif

/* setIsInterested(bool) [0x02A8765C] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A8765C
void Wolf::setIsInterested(bool a1)
{
    int32_t iVar1 = 0;
    iVar1 = this->field_0x1f8;
    if ((iVar1 == 0))
    {
        iVar1 = this->field_0x1f8;
    }
    ((SynchedEntityData *)gmem(iVar1))->set__tm__2_b(global_0x104C5218, (a1), 1);
    return;
}
#else
void Wolf::setIsInterested(bool a1)
{
    guest::stub(0x02A8765Cu, gaddr(this), (uint32_t)a1);
}
#endif

/* canMate(const boost::shared_ptr<Animal>&) [0x02A876EC] - LOWERED-DRAFT; not reconstructed (4 open edges, 21 raw IR ops, 7 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Wolf::canMate(uint32_t a1)
{
    return (uint32_t)(guest::stub(0x02A876ECu, gaddr(this), (uint32_t)a1) >> 32);
}
#else
uint32_t Wolf::canMate(uint32_t a1)
{
    uint32_t _frame = native_frame_open(24);
    uint8_t bVar1 = 0;
    uint8_t bVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t uVar7 = 0;
    int64_t lVar9 = 0;
    uint64_t __pair = 0;
    ST32((_frame + 8), this->field_0x0);
    if (((uint32_t)(LD32((_frame + 8))) != gaddr(this)))
    {
    }
    ST32((_frame + 16), LD32((_frame + 8)));
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
    bVar3 = (gelem<int32_t>((uint32_t)(a1), 0) != LD32((_frame + 16)));
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
    if ((bVar3 && (iVar4 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((this->field_0x348 + 0xe4c)), (uint32_t)(gaddr(this))))) >> 32), (iVar4 != 0))))
    {
        iVar4 = gelem<int32_t>((uint32_t)(a1), 0);
        if ((iVar4 == 0))
        {
            iVar4 = gelem<int32_t>((uint32_t)(a1), 0);
        }
        lVar9 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((iVar4 + 0x348))) + 0xc)), (uint32_t)(iVar4)))) >> 32);
        if ((lVar9 == 0xd6c0002))
        {
            ((boost *)gmem(a1))->static_pointer_cast__tm__13_4Wolf6Animal(_frame);
            if ((LD32(_frame) == 0))
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
            else
            {
                iVar4 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((LD32(_frame) + 0x348))) + 0xe4c)), (uint32_t)(LD32(_frame))))) >> 32);
                if ((iVar4 == 0))
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
                else
                {
                    if ((LD32(_frame) == 0))
                    {
                    }
                    iVar4 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((LD32(_frame) + 0x348))) + 0xe64)), (uint32_t)(LD32(_frame))))) >> 32);
                    if ((iVar4 == 0))
                    {
                        uVar7 = 0;
                        iVar4 = this->Animal::isInLove();
                        if ((iVar4 != 0))
                        {
                            if ((LD32(_frame) == 0))
                            {
                            }
                            iVar4 = ((Animal *)gmem(LD32(_frame)))->isInLove();
                            if ((iVar4 != 0))
                            {
                                uVar7 = 1;
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
                        { uint32_t t1 = uVar7;
                        native_frame_close(_frame, 24);
                        return t1; }
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
                }
            }
        }
    }
    { int32_t t2 = 0;
    native_frame_close(_frame, 24);
    return t2; }
    native_frame_close(_frame, 24);
}
#endif

/* wantsToAttack(boost::shared_ptr<LivingEntity>, boost::shared_ptr<LivingEntity>) [0x02A87CE8] - LOWERED-DRAFT; not reconstructed (32 open edges, 141 raw IR ops, 54 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Wolf::wantsToAttack(gptr<sp_LivingEntity> a1, gptr<sp_LivingEntity> a2)
{
    return (uint32_t)(guest::stub(0x02A87CE8u, gaddr(this), (uint32_t)a1, (uint32_t)a2) >> 32);
}
#else
uint32_t Wolf::wantsToAttack(gptr<sp_LivingEntity> a1, gptr<sp_LivingEntity> a2)
{
    uint32_t _frame = native_frame_open(40);
    uint8_t bVar1 = 0;
    uint8_t bVar2 = 0;
    uint8_t bVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t uVar5 = 0;
    uint32_t piVar6 = 0;
    int32_t iVar7 = 0;
    uint8_t bVar8 = 0;
    int64_t lVar9 = 0;
    uint64_t uVar10 = 0;
    uint64_t __pair = 0;
    if ((a1->field_0x0 == 0))
    {
        guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102bf1c0_s_boost__detail__sp_member_access_", 0x102BF1C8)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BF234)), (uint32_t)(0x28d), 0u, 0u);
    }
    lVar9 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((a1->field_0x0 + 0x348))) + 0xc)), (uint32_t)(a1->field_0x0)))) >> 32);
    if ((lVar9 != 0x5d540041))
    {
        if ((a1->field_0x0 == 0))
        {
            guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102bf1c0_s_boost__detail__sp_member_access_", 0x102BF1C8)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BF234)), (uint32_t)(0x28d), 0u, 0u);
        }
        lVar9 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((a1->field_0x0 + 0x348))) + 0xc)), (uint32_t)(a1->field_0x0)))) >> 32);
        if ((lVar9 != 0x5d0000c1))
        {
            if ((a1->field_0x0 == 0))
            {
                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102bf1c0_s_boost__detail__sp_member_access_", 0x102BF1C8)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BF234)), (uint32_t)(0x28d), 0u, 0u);
            }
            lVar9 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((a1->field_0x0 + 0x348))) + 0xc)), (uint32_t)(a1->field_0x0)))) >> 32);
            if ((lVar9 == 0xd6c0002))
            {
                ((boost *)gmem(a1))->static_pointer_cast__tm__20_4Wolf12LivingEntity(_frame);
                bVar1 = 0;
                bVar2 = 0;
                if ((LD32(_frame) == 0))
                {
                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102bf1c0_s_boost__detail__sp_member_access_", 0x102BF27C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BF234)), (uint32_t)(0x28d), 0u, 0u);
                }
                iVar4 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((LD32(_frame) + 0x348))) + 0xe4c)), (uint32_t)(LD32(_frame))))) >> 32);
                if ((iVar4 != 0))
                {
                    if ((LD32(_frame) == 0))
                    {
                        guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102bf1c0_s_boost__detail__sp_member_access_", 0x102BF27C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BF234)), (uint32_t)(0x28d), 0u, 0u);
                    }
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((LD32(_frame) + 0x348))) + 0xe8c)), (uint32_t)(LD32(_frame)), (uint32_t)((_frame + 32))))) >> 32);
                    bVar1 = 1;
                    if ((LD32((_frame + 32)) == a2->field_0x0))
                    {
                        bVar2 = 1;
                    }
                }
                if ((bVar1 && (bVar1 = (LD32((_frame + 36)) == (uint32_t)(0x0)), !(bVar1))))
                {
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar4 = (LD32(LD32((_frame + 36))) + -(1));
                        if (('\x01' != '\0'))
                        {
                            iVar7 = (ST32((uint32_t)(LD32((_frame + 36))) + (uint32_t)(0), iVar4), 1);
                            ST32(LD32((_frame + 36)), iVar7);
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    ((void)0 /* instructionSynchronize dropped */);
                    bVar8 = ((iVar4 == 0) << 1);
                    if ((iVar4 == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 36)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 36)))))) >> 32);
                        piVar6 = (LD32((_frame + 36)) + (1) * 4);
                        ((void)0 /* sync dropped */);
                        do
                        {
                            iVar4 = gelem<int32_t>((uint32_t)(piVar6), 0);
                            if (('\x01' != '\0'))
                            {
                                iVar7 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar4 + -(1))), 1);
                                gelem<int32_t>((uint32_t)(piVar6), 0) = iVar7;
                                bVar8 = 2;
                            }
                        }
                        while (!((uint8_t)(((bVar8 >> 1) & 1))));
                        ((void)0 /* instructionSynchronize dropped */);
                        if (((iVar4 + -(1)) == 0))
                        {
                            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 36)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 36)))))) >> 32);
                        }
                    }
                }
                if (bVar2)
                {
                    if ((LD32((_frame + 4)) == (uint32_t)(0x0)))
                    {
                        { int32_t t1 = 0;
                        native_frame_close(_frame, 40);
                        return t1; }
                    }
                    ((void)0 /* sync dropped */);
                    bVar1 = 0;
                    do
                    {
                        iVar4 = (LD32(LD32((_frame + 4))) + -(1));
                        if (('\x01' != '\0'))
                        {
                            iVar7 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar4), 1);
                            ST32(LD32((_frame + 4)), iVar7);
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    ((void)0 /* instructionSynchronize dropped */);
                    bVar8 = ((iVar4 == 0) << 1);
                    if ((iVar4 != 0))
                    {
                        { int32_t t2 = 0;
                        native_frame_close(_frame, 40);
                        return t2; }
                    }
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                    piVar6 = (LD32((_frame + 4)) + (1) * 4);
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar4 = gelem<int32_t>((uint32_t)(piVar6), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar7 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar4 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar6), 0) = iVar7;
                            bVar8 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar8 >> 1) & 1))));
                    ((void)0 /* instructionSynchronize dropped */);
                    if (((iVar4 + -(1)) != 0))
                    {
                        { int32_t t3 = 0;
                        native_frame_close(_frame, 40);
                        return t3; }
                    }
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 4))), (uint32_t)(((uint32_t)__pair)), (uint32_t)(piVar6), (uint32_t)(piVar6)))) >> 32);
                    { int32_t t4 = 0;
                    native_frame_close(_frame, 40);
                    return t4; }
                }
                bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
                if (!(bVar1))
                {
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar4 = (LD32(LD32((_frame + 4))) + -(1));
                        if (('\x01' != '\0'))
                        {
                            iVar7 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar4), 1);
                            ST32(LD32((_frame + 4)), iVar7);
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    ((void)0 /* instructionSynchronize dropped */);
                    bVar8 = ((iVar4 == 0) << 1);
                    if ((iVar4 == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                        piVar6 = (LD32((_frame + 4)) + (1) * 4);
                        ((void)0 /* sync dropped */);
                        do
                        {
                            iVar4 = gelem<int32_t>((uint32_t)(piVar6), 0);
                            if (('\x01' != '\0'))
                            {
                                iVar7 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar4 + -(1))), 1);
                                gelem<int32_t>((uint32_t)(piVar6), 0) = iVar7;
                                bVar8 = 2;
                            }
                        }
                        while (!((uint8_t)(((bVar8 >> 1) & 1))));
                        ((void)0 /* instructionSynchronize dropped */);
                        if (((iVar4 + -(1)) == 0))
                        {
                            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                        }
                    }
                }
            }
            bVar1 = 0;
            bVar3 = 0;
            bVar2 = 0;
            if ((a1->field_0x0 == 0))
            {
                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102bf1c0_s_boost__detail__sp_member_access_", 0x102BF1C8)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BF234)), (uint32_t)(0x28d), 0u, 0u);
            }
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((a1->field_0x0 + 0x348))) + 0xc)), (uint32_t)(a1->field_0x0)))) >> 32);
            if (((((uint32_t)__pair) & 0xc800000) == 0xc800000))
            {
                if ((a2->field_0x0 == 0))
                {
                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102bf1c0_s_boost__detail__sp_member_access_", 0x102BF1C8)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BF234)), (uint32_t)(0x28d), 0u, 0u);
                }
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((a2->field_0x0 + 0x348))) + 0xc)), (uint32_t)(a2->field_0x0)))) >> 32);
                if (((((uint32_t)__pair) & 0xc800000) == 0xc800000))
                {
                    ((boost *)gmem(a2))->static_pointer_cast__tm__22_6Player12LivingEntity((_frame + 8));
                    bVar2 = 1;
                    iVar4 = LD32((_frame + 8));
                    if ((LD32((_frame + 8)) == 0))
                    {
                        guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102bf1c0_s_boost__detail__sp_member_access_", 0x102BF2E0)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BF234)), (uint32_t)(0x28d), 0u, 0u);
                        iVar4 = LD32((_frame + 8));
                    }
                    iVar7 = LD32((uint32_t)((iVar4 + 0x348)));
                    ((boost *)gmem(a1))->static_pointer_cast__tm__22_6Player12LivingEntity((_frame + 24));
                    bVar1 = 1;
                    iVar4 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar7 + 0xc6c)), (uint32_t)(iVar4), (uint32_t)((_frame + 24))))) >> 32);
                    if ((iVar4 == 0))
                    {
                        bVar3 = 1;
                    }
                }
            }
            if ((bVar1 && (bVar1 = (LD32((_frame + 28)) == (uint32_t)(0x0)), !(bVar1))))
            {
                ((void)0 /* sync dropped */);
                do
                {
                    iVar4 = (LD32(LD32((_frame + 28))) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar7 = (ST32((uint32_t)(LD32((_frame + 28))) + (uint32_t)(0), iVar4), 1);
                        ST32(LD32((_frame + 28)), iVar7);
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                ((void)0 /* instructionSynchronize dropped */);
                bVar8 = ((iVar4 == 0) << 1);
                if ((iVar4 == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 28)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 28)))))) >> 32);
                    piVar6 = (LD32((_frame + 28)) + (1) * 4);
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar4 = gelem<int32_t>((uint32_t)(piVar6), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar7 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar4 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar6), 0) = iVar7;
                            bVar8 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar8 >> 1) & 1))));
                    ((void)0 /* instructionSynchronize dropped */);
                    if (((iVar4 + -(1)) == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 28)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 28))), (uint32_t)(((uint32_t)__pair)), (uint32_t)(LD32((uint32_t)((LD32(LD32((_frame + 28)) + (2) * 4) + 0x1c)))), (uint32_t)(piVar6)))) >> 32);
                    }
                }
            }
            if ((bVar2 && (bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0)), !(bVar1))))
            {
                ((void)0 /* sync dropped */);
                do
                {
                    iVar4 = (LD32(LD32((_frame + 12))) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar7 = (ST32((uint32_t)(LD32((_frame + 12))) + (uint32_t)(0), iVar4), 1);
                        ST32(LD32((_frame + 12)), iVar7);
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                ((void)0 /* instructionSynchronize dropped */);
                bVar8 = ((iVar4 == 0) << 1);
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
                            iVar7 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar4 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar6), 0) = iVar7;
                            bVar8 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar8 >> 1) & 1))));
                    ((void)0 /* instructionSynchronize dropped */);
                    if (((iVar4 + -(1)) == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 12)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 12)))))) >> 32);
                    }
                }
            }
            if (!(bVar3))
            {
                bVar1 = 0;
                bVar2 = 0;
                if ((a1->field_0x0 == 0))
                {
                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102bf1c0_s_boost__detail__sp_member_access_", 0x102BF1C8)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BF234)), (uint32_t)(0x28d), 0u, 0u);
                }
                lVar9 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((a1->field_0x0 + 0x348))) + 0xc)), (uint32_t)(a1->field_0x0)))) >> 32);
                uVar5 = (uint32_t)(lVar9);
                if ((lVar9 == 0xd690000))
                {
                    ((boost *)gmem(a1))->static_pointer_cast__tm__30_13AbstractHorse12LivingEntity((_frame + 16));
                    bVar1 = 1;
                    if ((LD32((_frame + 16)) == 0))
                    {
                        guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102bf1c0_s_boost__detail__sp_member_access_", 0x102BF344)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102BF234)), (uint32_t)(0x28d), 0u, 0u);
                    }
                    uVar10 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((LD32((_frame + 16)) + 0x348))) + 0xe44)), (uint32_t)(LD32((_frame + 16)))))) >> 32);
                    uVar5 = (uint32_t)(uVar10);
                    if (((int32_t)(((uint64_t)(uVar10) >> 0x20)) != 0))
                    {
                        bVar2 = 1;
                    }
                }
                if ((bVar1 && (bVar1 = (LD32((_frame + 20)) == (uint32_t)(0x0)), !(bVar1))))
                {
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar4 = (LD32(LD32((_frame + 20))) + -(1));
                        if (('\x01' != '\0'))
                        {
                            iVar7 = (ST32((uint32_t)(LD32((_frame + 20))) + (uint32_t)(0), iVar4), 1);
                            ST32(LD32((_frame + 20)), iVar7);
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    ((void)0 /* instructionSynchronize dropped */);
                    bVar8 = ((iVar4 == 0) << 1);
                    if ((iVar4 == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 20)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 20))), (uint32_t)(uVar5), (uint32_t)(LD32((_frame + 20))), (uint32_t)(LD32((_frame + 20)))))) >> 32);
                        piVar6 = (LD32((_frame + 20)) + (1) * 4);
                        ((void)0 /* sync dropped */);
                        do
                        {
                            iVar4 = gelem<int32_t>((uint32_t)(piVar6), 0);
                            if (('\x01' != '\0'))
                            {
                                iVar7 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar4 + -(1))), 1);
                                gelem<int32_t>((uint32_t)(piVar6), 0) = iVar7;
                                bVar8 = 2;
                            }
                        }
                        while (!((uint8_t)(((bVar8 >> 1) & 1))));
                        ((void)0 /* instructionSynchronize dropped */);
                        if (((iVar4 + -(1)) == 0))
                        {
                            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 20)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 20)))))) >> 32);
                        }
                    }
                }
                if (!(bVar2))
                {
                    { int32_t t5 = 1;
                    native_frame_close(_frame, 40);
                    return t5; }
                }
            }
        }
    }
    { int32_t t6 = 0;
    native_frame_close(_frame, 40);
    return t6; }
    native_frame_close(_frame, 40);
}
#endif

/* canBeLeashed(const boost::shared_ptr<Player>&) [0x02A88480] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Wolf::canBeLeashed(gptr<sp_Player> a1)
{
    return (uint32_t)(guest::stub(0x02A88480u, gaddr(this), (uint32_t)a1) >> 32);
}
#else
uint32_t Wolf::canBeLeashed(gptr<sp_Player> a1)
{
    int32_t iVar1 = 0;
    uint32_t uVar2 = 0;
    uVar2 = 0;
    iVar1 = guest::Wolf_isAngry(gaddr(this));
    if (((iVar1 == 0) && (iVar1 = this->TamableAnimal::canBeLeashed((gptr<sp_Player>)(a1)), (iVar1 != 0))))
    {
        uVar2 = 1;
    }
    return uVar2;
}
#endif

/* GetType() [0x02ACF790] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02ACF790
uint64_t Wolf::GetType()
{
    return 0xd6c000200000000;
}
#else
uint64_t Wolf::GetType()
{
    return (uint64_t)guest::stub(0x02ACF790u, gaddr(this));
}
#endif

}  /* namespace recon */
