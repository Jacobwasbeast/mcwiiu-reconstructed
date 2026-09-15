#include "game/W/Witch.hpp"
#include "game/D/DamageSource.hpp"
#include "game/E/Entity.hpp"
#include "game/L/Level.hpp"
#include "game/L/LivingEntity.hpp"
#include "game/M/Mob.hpp"
#include "game/M/Monster.hpp"
#include "game/S/SynchedEntityData.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(Level*) [0x02A420B4] - reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Witch::Witch(gptr<Level> a1)
{
    guest::stub(0x02A420B4u, gaddr(this), (uint32_t)a1);
}
#else
Witch::Witch(gptr<Level> a1) : Monster(a1)
{
    this->Mob::fjDerivedCtorCalls();
    this->Entity::setSize((float)(gref<double>(0x102B4358u)), (float)(gref<double>(0x102B5E9Cu)));
    this->field_0x734 = 0;
    this->field_0x190 = (this->field_0x190 | 0xc);
}
#endif

/* {dtor}() [0x02ACD25C] - reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Witch::~Witch()
{
    guest::stub(0x02ACD25Cu, gaddr(this), 0u);
}
#else
Witch::~Witch()
{
}
#endif

/* create(Level*) [static] [0x023E6624] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
void Witch::create(gptr<Level> a1)
{
    guest::stub(0x023E6624u, (uint32_t)a1);
}
#else
void Witch::create(gptr<Level> a1)
{
    guest::Witch_ct((uint32_t)(0), (uint32_t)(a1));
    return;
}
#endif

/* performRangedAttack(boost::shared_ptr<LivingEntity>, float) [0x02A1A67C] - boundary stub; body not reconstructed */
uint64_t Witch::performRangedAttack(uint32_t a1)
{
    return (uint64_t)guest::stub(0x02A1A67Cu, gaddr(this), (uint32_t)a1);
}

/* isAggressive() [0x02A1B710] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A1B710
uint32_t Witch::isAggressive()
{
    return 0;
}
#else
uint32_t Witch::isAggressive()
{
    return (uint32_t)(guest::stub(0x02A1B710u, gaddr(this)) >> 32);
}
#endif

/* setAggressive(bool) [0x02A1B72C] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A1B72C
void Witch::setAggressive(bool a1)
{
    return;
}
#else
void Witch::setAggressive(bool a1)
{
    guest::stub(0x02A1B72Cu, gaddr(this), (uint32_t)a1);
}
#endif

/* staticCtor() [static] [0x02A41FF0] - boundary stub; body not reconstructed */
uint64_t Witch::staticCtor()
{
    return (uint64_t)guest::stub(0x02A41FF0u);
}

/* addDataWalkers(DataFixerUpper*) [static] [0x02A42194] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A42194
void Witch::addDataWalkers(uint32_t a1, uint32_t a2)
{
    Mob::addDataWalkers((a1), (a2), 0, 0x5d540046);
    return;
}
#else
void Witch::addDataWalkers(uint32_t a1, uint32_t a2)
{
    guest::stub(0x02A42194u, (uint32_t)a1, (uint32_t)a2);
}
#endif

/* registerGoals() [0x02A42258] - boundary stub; body not reconstructed */
void Witch::registerGoals()
{
    guest::stub(0x02A42258u, gaddr(this));
}

/* defineSynchedData() [0x02A42490] - LOWERED-DRAFT; not reconstructed (2 open edges, 3 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Witch::defineSynchedData()
{
    guest::stub(0x02A42490u, gaddr(this));
}
#else
uint64_t Witch::defineSynchedData()
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    guest::Mob_defineSynchedData(gaddr(this));
    this->Entity::getEntityData(_frame);
    if ((LD32(_frame) == 0))
    {
    }
    ((SynchedEntityData *)gmem(LD32(_frame)))->createDateItem__tm__2_b(global_0x104C51D4, (uint8_t)(0));
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
    { uint64_t t1 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
    native_frame_close(_frame, 8);
    return t1; }
    native_frame_close(_frame, 8);
}
#endif

/* getAmbientSound() [0x02A425BC] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A425BC
uint32_t Witch::getAmbientSound()
{
    return global_0x104C46E0;
}
#else
uint32_t Witch::getAmbientSound()
{
    return (uint32_t)(guest::stub(0x02A425BCu, gaddr(this)) >> 32);
}
#endif

/* getHurtSound(const DamageSource*) [0x02A425C8] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A425C8
uint32_t Witch::getHurtSound(gptr<DamageSource> a1)
{
    return global_0x104C46EC;
}
#else
uint32_t Witch::getHurtSound(gptr<DamageSource> a1)
{
    return (uint32_t)(guest::stub(0x02A425C8u, gaddr(this), (uint32_t)a1) >> 32);
}
#endif

/* getDeathSound() [0x02A425D4] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A425D4
uint32_t Witch::getDeathSound()
{
    return global_0x104C46E4;
}
#else
uint32_t Witch::getDeathSound()
{
    return (uint32_t)(guest::stub(0x02A425D4u, gaddr(this)) >> 32);
}
#endif

/* setUsingItem(bool) [0x02A425E0] - LOWERED-DRAFT; not reconstructed (3 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Witch::setUsingItem(bool a1)
{
    return (uint64_t)guest::stub(0x02A425E0u, gaddr(this), (uint32_t)a1);
}
#else
uint64_t Witch::setUsingItem(bool a1)
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    this->Entity::getEntityData(_frame);
    if ((LD32(_frame) == 0))
    {
    }
    ((SynchedEntityData *)gmem(LD32(_frame)))->set__tm__2_b(global_0x104C51D4, (uint32_t)(a1), (bool)(1));
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
    { uint64_t t1 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
    native_frame_close(_frame, 8);
    return t1; }
    native_frame_close(_frame, 8);
}
#endif

/* isDrinkingPotion() [0x02A426FC] - LOWERED-DRAFT; not reconstructed (7 raw IR ops, 4 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint8_t Witch::isDrinkingPotion()
{
    return (uint8_t)(guest::stub(0x02A426FCu, gaddr(this)) >> 32);
}
#else
uint8_t Witch::isDrinkingPotion()
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    uint8_t uVar2 = 0;
    this->Entity::getEntityData(_frame);
    if ((LD32(_frame) == 0))
    {
    }
    uVar2 = LD8((uint32_t)((LD32((uint32_t)(((LD32(_frame) + (LD32(LD32(0x104C51D4u)) * 4)) + 4))) + 0xc)));
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
    { uint8_t t1 = uVar2;
    native_frame_close(_frame, 8);
    return t1; }
    native_frame_close(_frame, 8);
}
#endif

/* registerAttributes() [0x02A4281C] - LOWERED-DRAFT; not reconstructed (5 open edges, 2 raw IR ops) */
#if defined(RECON_RUNTIME_BUILD)
void Witch::registerAttributes()
{
    guest::stub(0x02A4281Cu, gaddr(this));
}
#else
uint64_t Witch::registerAttributes()
{
    uint32_t piVar1 = 0;
    guest::Monster_registerAttributes(gaddr(this));
    piVar1 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE10)));
    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar1), 0) + 0x24)), (uint32_t)(global_0x102B74D0)) >> 32);
    piVar1 = (uint32_t)(this->virt_getAttribute((uint32_t)(global_0x104EFE1C)));
    return ((uint64_t)(uint32_t)((uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar1), 0) + 0x24)), (uint32_t)(global_0x102B6BB8)) >> 32)) << 32);
}
#endif

/* aiStep() [0x02A428BC] - boundary stub; body not reconstructed */
void Witch::aiStep()
{
    guest::stub(0x02A428BCu, gaddr(this));
}

/* handleEntityEvent(unsigned char, int) [0x02A43EF8] - boundary stub; body not reconstructed */
uint64_t Witch::handleEntityEvent(uint8_t a1, int32_t a2)
{
    return (uint64_t)guest::stub(0x02A43EF8u, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}

/* getDamageAfterMagicAbsorb(const DamageSource*, float) [0x02A44118] - LOWERED-DRAFT; not reconstructed (2 open edges, 7 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
double Witch::getDamageAfterMagicAbsorb(int32_t a1)
{
    guest::stub(0x02A44118u, gaddr(this), (uint32_t)a1);
    return native_fret();
}
#else
double Witch::getDamageAfterMagicAbsorb(int32_t a1)
{
    uint32_t _frame = native_frame_open(56);
    uint8_t bVar1 = 0;
    uint8_t bVar3 = 0;
    int32_t iVar4 = 0;
    double dVar8 = 0;
    dVar8 = (double)(guest::LivingEntity_getDamageAfterMagicAbsorb(gaddr(this), (uint32_t)a1));
    (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((a1 + 0x18))) + 0x1c)), (uint32_t)(a1), (uint32_t)((_frame + 12))) >> 32);
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
    bVar3 = (LD32((_frame + 12)) == LD32((_frame + 4)));
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
    if (bVar3)
    {
        dVar8 = (double)(global_0x102B5164);
    }
    iVar4 = ((DamageSource *)gmem(a1))->isMagic();
    if ((iVar4 != 0))
    {
        dVar8 = (double)((float)((dVar8 * (double)(global_0x102B6538))));
    }
    { double t1 = dVar8;
    native_frame_close(_frame, 56);
    return t1; }
    native_frame_close(_frame, 56);
}
#endif

/* getDefaultLootTable() [0x02A443F4] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A443F4
uint32_t Witch::getDefaultLootTable()
{
    return 0x104D6FE0u;
}
#else
uint32_t Witch::getDefaultLootTable()
{
    return (uint32_t)(guest::stub(0x02A443F4u, gaddr(this)) >> 32);
}
#endif

/* getEyeHeight() [0x02A44400] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02A44400
double Witch::getEyeHeight()
{
    return gref<double>(0x102B779Cu);
}
#else
double Witch::getEyeHeight()
{
    guest::stub(0x02A44400u, gaddr(this));
    return native_fret();
}
#endif

/* GetType() [0x02ACE664] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02ACE664
uint64_t Witch::GetType()
{
    return 0x5d54004600000000;
}
#else
uint64_t Witch::GetType()
{
    return (uint64_t)guest::stub(0x02ACE664u, gaddr(this));
}
#endif

}  /* namespace recon */
