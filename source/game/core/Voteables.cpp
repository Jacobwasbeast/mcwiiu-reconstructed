#include "game/V/Voteables.hpp"
#include "game/V/Voteable.hpp"
#include "_thirdparty/boost__shared_ptr_ServerPlayer_.hpp"
#include "_thirdparty/boost__unordered__detail__table_impl_boost__unordered__detail__map_std__allocator_std__pair_const_int__Voteable_____int_.hpp"
#include "_thirdparty/boost__unordered__unordered_map_int__Voteable___boost__hash_int___std__equal_to_int___std__allocator_std__pair_const_int.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}() [0x02C7780C] - boundary stub */
Voteables::Voteables()
{
    guest::stub(0x02C7780Cu, gaddr(this));
}

/* {dtor}() [0x02C77908] - LOWERED-DRAFT; not reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Voteables::~Voteables()
{
    guest::stub(0x02C77908u, gaddr(this), 0u);
}
#else
Voteables::~Voteables()
{
    int32_t iVar1 = 0;
    int32_t iVar2 = 0;
    if ((this->field_0x8 == 0))
    {
        this->map_int_Voteable_p::clear();
        guest::map_int_Voteable_p_dt((uint32_t)(gaddr(this)), (uint32_t)(2));
    }
    else
    {
        iVar1 = this->field_0x14;
        iVar2 = this->field_0x4;
        if ((iVar1 == 0))
        {
            iVar1 = this->field_0x14;
        }
        iVar1 = LD32((uint32_t)((iVar1 + (iVar2 * 4))));
        if ((iVar1 != 0))
        {
            iVar1 = (iVar1 + -(8));
        }
        while ((iVar1 != 0))
        {
            guest::Voteable_dt((uint32_t)(LD32((uint32_t)((iVar1 + 4)))), (uint32_t)(3));
            iVar1 = LD32((uint32_t)((iVar1 + 8)));
            if ((iVar1 != 0))
            {
                iVar1 = (iVar1 + -(8));
            }
        }
        this->map_int_Voteable_p::clear();
        guest::map_int_Voteable_p_dt((uint32_t)(gaddr(this)), (uint32_t)(2));
    }
}
#endif

/* getVoteable(unsigned int) [0x02C621F4] - LOWERED-DRAFT; not reconstructed (9 raw IR ops, 2 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Voteables::getVoteable(uint32_t a1)
{
    return (uint32_t)(guest::stub(0x02C621F4u, gaddr(this), (uint32_t)a1) >> 32);
}
#else
uint32_t Voteables::getVoteable(uint32_t a1)
{
    uint32_t _frame = native_frame_open(16);
    int32_t iVar1 = 0;
    ST32(_frame, a1);
    ST32((_frame + 8), (uint32_t)((this->map_int_Voteable_p::find(_frame)) >> 32));
    ST32((_frame + 4), 0);
    if ((LD32((_frame + 8)) == 0))
    {
        { int32_t t1 = 0;
        native_frame_close(_frame, 16);
        return t1; }
    }
    ST32(_frame, a1);
    ST32((_frame + 12), LD32((_frame + 8)));
    iVar1 = (uint32_t)((this->map_int_Voteable_p::find(_frame)) >> 32);
    { uint32_t t2 = LD32((uint32_t)((iVar1 + 4)));
    native_frame_close(_frame, 16);
    return t2; }
    native_frame_close(_frame, 16);
}
#endif

/* init() [0x02C77A74] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02C77A74
void Voteables::init()
{
    return;
}
#else
void Voteables::init()
{
    guest::stub(0x02C77A74u, gaddr(this));
}
#endif

/* addVoteable(Voteable*) [0x02C78234] - LOWERED-DRAFT; not reconstructed (1 open edges, 8 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Voteables::addVoteable(gptr<Voteable> a1)
{
    guest::stub(0x02C78234u, gaddr(this), (uint32_t)a1);
}
#else
void Voteables::addVoteable(gptr<Voteable> a1)
{
    uint32_t _frame = native_frame_open(16);
    int32_t iVar1 = 0;
    ST32(_frame, LD32(a1));
    iVar1 = (uint32_t)((this->map_int_Voteable_p::find(_frame)) >> 32);
    if ((iVar1 == 0))
    {
        ST32((_frame + 4), LD32(a1));
        ST32((_frame + 12), (_frame + 4));
        ST32((_frame + 8), a1);
        ((boost_unordered_detail_table_impl_boost_unordered_detail_map_int_Voteable_p *)gmem(gaddr(this)))->emplace_impl__tm__85_Q4_5boost9unordered6detail56emplace_args1__tm__35_Q2_3std25pair__tm__13_CiP8Voteable(LD32((_frame + 12)), (_frame + 12));
    }
    native_frame_close(_frame, 16);
    return;
    native_frame_close(_frame, 16);
}
#endif

/* syncAllVoteablesToPlayer(boost::shared_ptr<ServerPlayer>) [0x02C782B8] - LOWERED-DRAFT; not reconstructed (21 raw IR ops, 8 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Voteables::syncAllVoteablesToPlayer(gptr<sp_ServerPlayer> a1)
{
    return (uint64_t)guest::stub(0x02C782B8u, gaddr(this), (uint32_t)a1);
}
#else
uint64_t Voteables::syncAllVoteablesToPlayer(gptr<sp_ServerPlayer> a1)
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t piVar3 = 0;
    int32_t iVar4 = 0;
    int32_t iVar6 = 0;
    uint8_t bVar7 = 0;
    if ((this->field_0x8 == 0))
    {
        { uint64_t t1 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
        native_frame_close(_frame, 8);
        return t1; }
    }
    iVar4 = this->field_0x14;
    iVar6 = this->field_0x4;
    if ((iVar4 == 0))
    {
        iVar4 = this->field_0x14;
    }
    iVar4 = LD32((uint32_t)((iVar4 + (iVar6 * 4))));
    if ((iVar4 != 0))
    {
        iVar4 = (iVar4 + -(8));
    }
    while ((iVar4 != 0))
    {
        ST32(_frame, a1->field_0x0);
        ST32((_frame + 4), (uint32_t)(a1->field_0x4));
        bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
        while (!(bVar1))
        {
            if (('\x01' != '\0'))
            {
                iVar6 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
                ST32(LD32((_frame + 4)), iVar6);
                bVar1 = 1;
            }
        }
        ((Voteable *)gmem(LD32((uint32_t)((iVar4 + 4)))))->syncAllVotesToPlayer((gptr<sp_ServerPlayer>)(_frame));
        piVar3 = LD32((_frame + 4));
        bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            do
            {
                iVar6 = (LD32(LD32((_frame + 4))) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar6), 1);
                    ST32(LD32((_frame + 4)), iVar2);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            bVar7 = ((iVar6 == 0) << 1);
        }
        iVar4 = LD32((uint32_t)((iVar4 + 8)));
        if ((iVar4 != 0))
        {
            iVar4 = (iVar4 + -(8));
        }
    }
    { uint64_t t2 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
    native_frame_close(_frame, 8);
    return t2; }
    native_frame_close(_frame, 8);
}
#endif

}  /* namespace recon */
