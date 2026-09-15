#include "game/W/WeaponDamageStatTracker.hpp"
#include "game/D/DamageEvent.hpp"
#include "game/S/Scoreboard__Weapon.hpp"
#include "_thirdparty/std___Tree_std___Tmap_traits_UifQ2_3std14less__tm__3_UiQ2_3std43allocator__tm__26_Q2_3std16pair__tm__5_CUifXCbL_1_0__.hpp"
#include "_thirdparty/std__map_unsigned_int__float__std__less_unsigned_int___std__allocator_std__pair_const_unsigned_int__float___.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(const StatsUID&, WeaponDamageStatTracker::ETrackingType) [0x02C1AE8C] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WeaponDamageStatTracker::WeaponDamageStatTracker(gptr<StatsUID> a1, uint32_t a2)
{
    guest::stub(0x02C1AE8Cu, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}
#else
WeaponDamageStatTracker::WeaponDamageStatTracker(gptr<StatsUID> a1, uint32_t a2)
{
    uint32_t _frame = native_frame_open(24);
    uint32_t uVar1 = 0;
    uint32_t puVar2 = 0;
    int32_t iVar3 = 0;
    puVar2 = (uint32_t)(guest::Scoreboard_Weapon_ct((uint32_t)((_frame + 12)), (uint32_t)(0), (uint32_t)(0)));
    ST32((_frame + 4), gelem<uint32_t>((uint32_t)(puVar2), 0));
    ST32((_frame + 8), gelem<uint32_t>((uint32_t)(puVar2) + 1u, 0));
    iVar3 = gaddr(this);
    if (((gaddr(this) != 0) || (iVar3 = guest::guest_new((uint32_t)(0x20)), (iVar3 != 0))))
    {
        guest::UntypedStatTracker_ct((uint32_t)(iVar3), (uint32_t)(a1));
        ST32((uint32_t)((iVar3 + 0x14)), GSTR(L"", 0x1030D0BC));
        ST32((uint32_t)((iVar3 + 0x18)), LD32((_frame + 4)));
        ST32((uint32_t)((iVar3 + 0x1c)), LD32((_frame + 8)));
    }
    ST32((uint32_t)((gaddr(this) + 0x14)), GSTR(L"", 0x10337078));
    if ((gaddr(this) == -(0x20)))
    {
        ::recon::guest_new((uint32_t)(3));
    }
    ((std_Tree_std_Tmap_traits_UifQ2_3std14less_tm_3_UiQ2_3std43allocator_tm_26_Q2_3st_c64e0ce8 *)gmem((gaddr(this) + 0x20)))->_Init();
    ST32(_frame, ((Scoreboard_Weapon *)gmem(gaddr(&this->field_0x18)))->GetKey());
    puVar2 = (uint32_t)(((std_map_unsigned_int_float_ *)gmem((gaddr(this) + 0x20)))->operator[](_frame));
    uVar1 = global_0x1031F598;
    this->field_0x2c = a2;
    gelem<uint32_t>((uint32_t)(puVar2), 0) = uVar1;
    native_frame_close(_frame, 24);
}
#endif

/* {dtor}() [0x02CDE6D8] - LOWERED-DRAFT; not reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WeaponDamageStatTracker::~WeaponDamageStatTracker()
{
    guest::stub(0x02CDE6D8u, gaddr(this), 0u);
}
#else
WeaponDamageStatTracker::~WeaponDamageStatTracker()
{
}
#endif

/* update(const DamageEvent*) [0x02C28CF4] - LOWERED-DRAFT; not reconstructed (2 open edges, 15 raw IR ops, 8 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WeaponDamageStatTracker::update(gptr<DamageEvent> a1)
{
    guest::stub(0x02C28CF4u, gaddr(this), (uint32_t)a1);
}
#else
void WeaponDamageStatTracker::update(gptr<DamageEvent> a1)
{
    uint32_t _frame = native_frame_open(48);
    float fVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t pfVar3 = 0;
    uint32_t pfVar4 = 0;
    iVar2 = this->field_0x2c;
    if ((iVar2 == 1))
    {
        iVar2 = guest::UntypedStatTracker_isTracking((uint32_t)(gaddr(this)), (uint32_t)(a1));
        if ((iVar2 == 0))
        {
            iVar2 = this->field_0x2c;
            goto LAB_02c28d4c;
        }
    }
    else
    {
LAB_02c28d4c:;
        if (((iVar2 != 0) || (iVar2 = guest::UntypedStatTracker_isTracking((uint32_t)(gaddr(this)), (uint32_t)(gaddr(&a1->field_0x14))), (iVar2 == 0))))
        {
            goto LAB_02c28eec;
        }
    }
    ST32((_frame + 8), a1->field_0x44);
    ST32((_frame + 4), a1->field_0x40);
    ST32(_frame, ((Scoreboard_Weapon *)gmem((_frame + 4)))->GetKey());
    iVar2 = ((Scoreboard_Weapon *)gmem((_frame + 4)))->HasItem();
    if ((iVar2 != 0))
    {
        fVar1 = a1->field_0x3c;
        iVar2 = ((std_Tree_std_Tmap_traits_UifQ2_3std14less_tm_3_UiQ2_3std43allocator_tm_26_Q2_3st_c64e0ce8 *)gmem((gaddr(this) + 0x20)))->find(_frame);
        if ((iVar2 == this->field_0x24))
        {
            pfVar3 = (uint32_t)(((std_map_unsigned_int_float_ *)gmem((gaddr(this) + 0x20)))->operator[](_frame));
            STF(pfVar3, fVar1);
            ST32((_frame + 12), ((Scoreboard_Weapon *)gmem(gaddr(&this->field_0x18)))->GetKey());
            pfVar3 = (uint32_t)(((std_map_unsigned_int_float_ *)gmem((gaddr(this) + 0x20)))->operator[]((_frame + 12)));
            pfVar4 = (uint32_t)(((std_map_unsigned_int_float_ *)gmem((gaddr(this) + 0x20)))->operator[](_frame));
            if ((LDF(pfVar4) < LDF(pfVar3)))
            {
                goto LAB_02c28eec;
            }
        }
        else
        {
            pfVar3 = (uint32_t)(((std_map_unsigned_int_float_ *)gmem((gaddr(this) + 0x20)))->operator[](_frame));
            STF(pfVar3, (LDF(pfVar3) + fVar1));
            ST32((_frame + 12), ((Scoreboard_Weapon *)gmem(gaddr(&this->field_0x18)))->GetKey());
            pfVar3 = (uint32_t)(((std_map_unsigned_int_float_ *)gmem((gaddr(this) + 0x20)))->operator[]((_frame + 12)));
            pfVar4 = (uint32_t)(((std_map_unsigned_int_float_ *)gmem((gaddr(this) + 0x20)))->operator[](_frame));
            if ((LDF(pfVar4) < LDF(pfVar3)))
            {
                goto LAB_02c28eec;
            }
        }
        this->field_0x18 = LD32((_frame + 4));
        this->field_0x1c = LD32((_frame + 8));
    }
LAB_02c28eec:;
    native_frame_close(_frame, 48);
    return;
    native_frame_close(_frame, 48);
}
#endif

}  /* namespace recon */
