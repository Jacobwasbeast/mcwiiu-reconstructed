#include "game/V/Voteable.hpp"
#include "game/C/ClientPacketListener.hpp"
#include "game/M/Minecraft.hpp"
#include "game/P/PlayerList.hpp"
#include "game/R/Random.hpp"
#include "game/S/ServerGamePacketListenerImpl.hpp"
#include "_thirdparty/boost__shared_ptr_MultiplayerLocalPlayer_.hpp"
#include "_thirdparty/boost__shared_ptr_ServerPlayer_.hpp"
#include "_thirdparty/boost__unordered__detail__table_impl_boost__unordered__detail__map_std__allocator_std__pair_const_std__pair_int__int___c.hpp"
#include "_thirdparty/boost__unordered__unordered_map_int__int__boost__hash_int___std__equal_to_int___std__allocator_std__pair_const_int__int_.hpp"
#include "_thirdparty/boost__unordered__unordered_map_std__pair_int__int___char__Voteable__KeyHasher__std__equal_to_std__pair_int__int____std_.hpp"
#include "_thirdparty/std___Tree_std___Tmap_traits_iiQ2_3std13less__tm__2_iQ2_3std42allocator__tm__25_Q2_3std15pair__tm__4_CiiXCbL_1_0__.hpp"
#include "_thirdparty/std__map_int__int__std__less_int___std__allocator_std__pair_const_int__int___.hpp"
#include "_thirdparty/std__vector_std__pair_int__int___std__allocator_std__pair_int__int___.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(int, bool, bool) [0x02C73C38] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
Voteable::Voteable(int32_t a1, bool a2, bool a3)
{
    guest::stub(0x02C73C38u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
}
#else
Voteable::Voteable(int32_t a1, bool a2, bool a3)
{
    uint32_t _frame = native_frame_open(8);
    ST8((uint32_t)((gaddr(this) + (1) * 4)), a2);
    ST32(gaddr(this), a1);
    ST8((uint32_t)(((int32_t)(gaddr(this)) + 5)), a3);
    native_frame_close(_frame, 8);
}
#endif

Voteable::~Voteable()
{
}

/* getVotesFromPlayer(int) [0x02C622F4] - LOWERED-DRAFT; not reconstructed (3 open edges, 7 raw IR ops, 7 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Voteable::getVotesFromPlayer(int32_t a1, int32_t a2)
{
    return (uint64_t)guest::stub(0x02C622F4u, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}
#else
uint64_t Voteable::getVotesFromPlayer(int32_t a1, int32_t a2)
{
    uint32_t _frame = native_frame_open(16);
    int8_t cVar1 = 0;
    uint32_t piVar2 = 0;
    int32_t iVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t piVar5 = 0;
    ((std_Tree_std_Tmap_traits_iiQ2_3std13less_tm_2_iQ2_3std42allocator_tm_25_Q2_3std1_d9c536db *)gmem(_frame))->_Init();
    if ((LD32((uint32_t)((gaddr(this) + 0x10))) != 0))
    {
        iVar3 = LD32((uint32_t)((gaddr(this) + 0x1c)));
        iVar4 = LD32((uint32_t)((gaddr(this) + 0xc)));
        if ((iVar3 == 0))
        {
            iVar3 = LD32((uint32_t)((gaddr(this) + 0x1c)));
        }
        piVar5 = LD32((uint32_t)((iVar3 + (iVar4 * 4))));
        if ((piVar5 != (uint32_t)(0x0)))
        {
            piVar5 = (piVar5 + (-(3)) * 4);
        }
        while ((piVar5 != (uint32_t)(0x0)))
        {
            cVar1 = (int8_t)LD8((uint32_t)((piVar5 + (2) * 4)));
            if ((gelem<int32_t>((uint32_t)(piVar5), 0) == a2))
            {
                piVar2 = (uint32_t)(((std_map_int_int_ *)gmem(_frame))->operator[]((piVar5 + (1) * 4)));
                gelem<int32_t>((uint32_t)(piVar2), 0) = (int32_t)(cVar1);
            }
            piVar5 = (uint32_t)(gelem<int32_t>((uint32_t)(piVar5) + 3u, 0));
            if ((piVar5 != (uint32_t)(0x0)))
            {
                piVar5 = (piVar5 + (-(3)) * 4);
            }
        }
    }
    if ((((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0xc)), (a1 != 0))) && ((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0xc)), (a1 != 0)))))
    {
        if ((a1 == 0))
        {
            ::recon::guest_new((uint32_t)(3));
        }
        ((std_Tree_std_Tmap_traits_iiQ2_3std13less_tm_2_iQ2_3std42allocator_tm_25_Q2_3std1_d9c536db *)gmem(a1))->_Init();
        ((std_Tree_std_Tmap_traits_iiQ2_3std13less_tm_2_iQ2_3std42allocator_tm_25_Q2_3std1_d9c536db *)gmem(a1))->_Copy((int32_t)(_frame));
    }
    { uint64_t t1 = ((uint64_t)(uint32_t)(guest::std_Tree_std_Tmap_traits_iiQ2_3std13less_tm_2_iQ2_3std42allocator_tm_25_Q2_3std1_d9c536db__Tidy((uint32_t)(_frame))) << 32);
    native_frame_close(_frame, 16);
    return t1; }
    native_frame_close(_frame, 16);
}
#endif

/* getVotesToVoteable(int) [0x02C624B8] - LOWERED-DRAFT; not reconstructed (3 open edges, 9 raw IR ops, 9 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Voteable::getVotesToVoteable(int32_t a1, int32_t a2)
{
    return (uint64_t)guest::stub(0x02C624B8u, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}
#else
uint64_t Voteable::getVotesToVoteable(int32_t a1, int32_t a2)
{
    uint32_t _frame = native_frame_open(16);
    int8_t cVar1 = 0;
    uint32_t piVar2 = 0;
    int32_t iVar3 = 0;
    int32_t iVar4 = 0;
    ((std_Tree_std_Tmap_traits_iiQ2_3std13less_tm_2_iQ2_3std42allocator_tm_25_Q2_3std1_d9c536db *)gmem(_frame))->_Init();
    if ((LD32((uint32_t)((gaddr(this) + 0x10))) != 0))
    {
        iVar3 = LD32((uint32_t)((gaddr(this) + 0x1c)));
        iVar4 = LD32((uint32_t)((gaddr(this) + 0xc)));
        if ((iVar3 == 0))
        {
            iVar3 = LD32((uint32_t)((gaddr(this) + 0x1c)));
        }
        iVar3 = LD32((uint32_t)((iVar3 + (iVar4 * 4))));
        if ((iVar3 != 0))
        {
            iVar3 = (iVar3 + -(0xc));
        }
        while ((iVar3 != 0))
        {
            cVar1 = (int8_t)LD8((uint32_t)((iVar3 + 8)));
            if ((LD32((uint32_t)((iVar3 + 4))) == a2))
            {
                piVar2 = (uint32_t)(((std_map_int_int_ *)gmem(_frame))->operator[]((uint32_t)(iVar3)));
                gelem<int32_t>((uint32_t)(piVar2), 0) = (int32_t)(cVar1);
            }
            iVar3 = LD32((uint32_t)((iVar3 + 0xc)));
            if ((iVar3 != 0))
            {
                iVar3 = (iVar3 + -(0xc));
            }
        }
    }
    if ((((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0xc)), (a1 != 0))) && ((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0xc)), (a1 != 0)))))
    {
        if ((a1 == 0))
        {
            ::recon::guest_new((uint32_t)(3));
        }
        ((std_Tree_std_Tmap_traits_iiQ2_3std13less_tm_2_iQ2_3std42allocator_tm_25_Q2_3std1_d9c536db *)gmem(a1))->_Init();
        ((std_Tree_std_Tmap_traits_iiQ2_3std13less_tm_2_iQ2_3std42allocator_tm_25_Q2_3std1_d9c536db *)gmem(a1))->_Copy((int32_t)(_frame));
    }
    { uint64_t t1 = ((uint64_t)(uint32_t)(guest::std_Tree_std_Tmap_traits_iiQ2_3std13less_tm_2_iQ2_3std42allocator_tm_25_Q2_3std1_d9c536db__Tidy((uint32_t)(_frame))) << 32);
    native_frame_close(_frame, 16);
    return t1; }
    native_frame_close(_frame, 16);
}
#endif

/* getTotalledVotes() [0x02C6275C] - LOWERED-DRAFT; not reconstructed (5 open edges, 10 raw IR ops, 9 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gresult<map_int_int_> Voteable::getTotalledVotes()
{
    gresult<map_int_int_> result;
    guest::stub(0x02C6275Cu, gaddr(this), (uint32_t)result);
    return result;
}
#else
gresult<map_int_int_> Voteable::getTotalledVotes(uint32_t a1)
{
    uint32_t _frame = native_frame_open(40);
    uint8_t cVar1 = 0;
    uint32_t piVar2 = 0;
    int32_t iVar3 = 0;
    int32_t iVar4 = 0;
    guest::map_int_int__ct_2((uint32_t)((_frame + 8)), (uint32_t)(0xb), (uint32_t)((_frame + 4)), (uint32_t)((_frame + 5)), (uint32_t)((_frame + 6)));
    if ((LD32((uint32_t)((gaddr(this) + 0x10))) == 0))
    {
        guest::map_int_int__ct((uint32_t)(a1), (uint32_t)((_frame + 8)));
        guest::map_int_int__dt((uint32_t)((_frame + 8)), (uint32_t)(2));
        native_frame_close(_frame, 40);
        return;
    }
    iVar3 = LD32((uint32_t)((gaddr(this) + 0x1c)));
    iVar4 = LD32((uint32_t)((gaddr(this) + 0xc)));
    if ((iVar3 == 0))
    {
        iVar3 = LD32((uint32_t)((gaddr(this) + 0x1c)));
    }
    iVar3 = LD32((uint32_t)((iVar3 + (iVar4 * 4))));
    if ((iVar3 != 0))
    {
        iVar3 = (iVar3 + -(0xc));
    }
    while ((iVar3 != 0))
    {
        cVar1 = LD8((uint32_t)((iVar3 + 8)));
        ST32(_frame, LD32((uint32_t)((iVar3 + 4))));
        if ((-(1) < cVar1))
        {
            piVar2 = (uint32_t)(((map_int_int_ *)gmem((_frame + 8)))->operator[](_frame));
            gelem<int32_t>((uint32_t)(piVar2), 0) = (gelem<int32_t>((uint32_t)(piVar2), 0) + (int32_t)(cVar1));
        }
        iVar3 = LD32((uint32_t)((iVar3 + 0xc)));
        if ((iVar3 != 0))
        {
            iVar3 = (iVar3 + -(0xc));
        }
    }
    guest::map_int_int__ct((uint32_t)(a1), (uint32_t)((_frame + 8)));
    guest::map_int_int__dt((uint32_t)((_frame + 8)), (uint32_t)(2));
    native_frame_close(_frame, 40);
    return;
    native_frame_close(_frame, 40);
}
#endif

/* getWinningVote(int&) [0x02C628F8] - LOWERED-DRAFT; not reconstructed (3 open edges, 6 raw IR ops, 2 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Voteable::getWinningVote(gptr<int32_t> a1)
{
    return (uint32_t)(guest::stub(0x02C628F8u, gaddr(this), (uint32_t)a1) >> 32);
}
#else
uint32_t Voteable::getWinningVote(gptr<int32_t> a1)
{
    uint32_t _frame = native_frame_open(56);
    int32_t iVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    uint32_t uVar4 = 0;
    uint32_t puVar5 = 0;
    this->getTotalledVotes((uint32_t)(_frame));
    guest::Random_ct((uint32_t)((_frame + 24)));
    uVar4 = 0xffffffff;
    iVar2 = 0;
    if ((LD32((_frame + 8)) != 0))
    {
        if ((LD32((_frame + 20)) == 0))
        {
        }
        puVar5 = LD32((uint32_t)((LD32((_frame + 20)) + (LD32((_frame + 4)) * 4))));
        if ((puVar5 != (uint32_t)(0x0)))
        {
            puVar5 = (puVar5 + (-(2)) * 4);
        }
        while ((puVar5 != (uint32_t)(0x0)))
        {
            iVar3 = gelem<uint32_t>((uint32_t)(puVar5) + 1u, 0);
            if (((0 < iVar3) && ((iVar2 < iVar3) || ((iVar2 <= iVar3) && (iVar1 = ((Random *)gmem((_frame + 24)))->nextBoolean(), (iVar1 != 0))))))
            {
                uVar4 = gelem<uint32_t>((uint32_t)(puVar5), 0);
                iVar2 = iVar3;
            }
            puVar5 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar5) + 2u, 0));
            if ((puVar5 != (uint32_t)(0x0)))
            {
                puVar5 = (puVar5 + (-(2)) * 4);
            }
        }
        gelem<int32_t>((uint32_t)(a1), 0) = iVar2;
        guest::map_int_int__dt((uint32_t)(_frame), (uint32_t)(2));
        { uint32_t t1 = uVar4;
        native_frame_close(_frame, 56);
        return t1; }
    }
    gelem<int32_t>((uint32_t)(a1), 0) = 0;
    guest::map_int_int__dt((uint32_t)(_frame), (uint32_t)(2));
    { int32_t t2 = 0xffffffff;
    native_frame_close(_frame, 56);
    return t2; }
    native_frame_close(_frame, 56);
}
#endif

/* getOnlyVote(int, int*) const [0x02C62A68] - LOWERED-DRAFT; not reconstructed (5 raw IR ops, 5 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Voteable::getOnlyVote(int32_t a1, gptr<int32_t> a2)
{
    return (uint32_t)(guest::stub(0x02C62A68u, gaddr(this), (uint32_t)a1, (uint32_t)a2) >> 32);
}
#else
uint32_t Voteable::getOnlyVote(int32_t a1, gptr<int32_t> a2)
{
    int32_t iVar1 = 0;
    uint32_t piVar2 = 0;
    int32_t iVar3 = 0;
    if ((LD32((uint32_t)((gaddr(this) + 0x10))) == 0))
    {
        return 0;
    }
    iVar1 = LD32((uint32_t)((gaddr(this) + 0x1c)));
    iVar3 = LD32((uint32_t)((gaddr(this) + 0xc)));
    if ((iVar1 == 0))
    {
        iVar1 = LD32((uint32_t)((gaddr(this) + 0x1c)));
    }
    piVar2 = LD32((uint32_t)((iVar1 + (iVar3 * 4))));
    if ((piVar2 != (uint32_t)(0x0)))
    {
        piVar2 = (piVar2 + (-(3)) * 4);
    }
    while (1)
    {
        if ((piVar2 == (uint32_t)(0x0)))
        {
            return 0;
        }
        if ((gelem<int32_t>((uint32_t)(piVar2), 0) == a1))
        {
            break;
        }
        piVar2 = (uint32_t)(gelem<int32_t>((uint32_t)(piVar2) + 3u, 0));
        if ((piVar2 != (uint32_t)(0x0)))
        {
            piVar2 = (piVar2 + (-(3)) * 4);
        }
    }
    gelem<int32_t>((uint32_t)(a2), 0) = gelem<int32_t>((uint32_t)(piVar2) + 1u, 0);
    return 1;
}
#endif

/* getKey(int, int) [static] [0x02C73F44] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02C73F44
void Voteable::getKey(int32_t a1, int32_t a2)
{
    return;
}
#else
void Voteable::getKey(int32_t a1, int32_t a2)
{
    guest::stub(0x02C73F44u, (uint32_t)a1, (uint32_t)a2);
}
#endif

/* getVote(int, int) [0x02C740A8] - LOWERED-DRAFT; not reconstructed (1 open edges, 3 raw IR ops, 2 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
int32_t Voteable::getVote(int32_t a1, int32_t a2)
{
    return (int32_t)(guest::stub(0x02C740A8u, gaddr(this), (uint32_t)a1, (uint32_t)a2) >> 32);
}
#else
int32_t Voteable::getVote(int32_t a1, int32_t a2)
{
    uint32_t _frame = native_frame_open(8);
    int32_t iVar1 = 0;
    ST64(_frame, guest::Voteable_getKey((uint32_t)(a1), (uint32_t)(a2)));
    iVar1 = (uint32_t)((((map_pair_int_int_char_Voteable_KeyHasher *)gmem((gaddr(this) + 8)))->find(_frame)) >> 32);
    if ((iVar1 != 0))
    {
        { int32_t t1 = (int32_t)(LD8((uint32_t)((iVar1 + 8))));
        native_frame_close(_frame, 8);
        return t1; }
    }
    { int32_t t2 = -(1);
    native_frame_close(_frame, 8);
    return t2; }
    native_frame_close(_frame, 8);
}
#endif

/* getVote(boost::shared_ptr<MultiplayerLocalPlayer>, int) [0x02C74130] - LOWERED-DRAFT; not reconstructed (1 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t Voteable::getVote(gptr<sp_MultiplayerLocalPlayer> a1, int32_t a2)
{
    return (uint32_t)(guest::stub(0x02C74130u, gaddr(this), (uint32_t)a1, (uint32_t)a2) >> 32);
}
#else
uint32_t Voteable::getVote(gptr<sp_MultiplayerLocalPlayer> a1, int32_t a2)
{
    uint32_t uVar1 = 0;
    int32_t iVar2 = 0;
    if ((a1->field_0x0 != 0))
    {
        iVar2 = a1->field_0x0;
        if ((iVar2 == 0))
        {
            iVar2 = a1->field_0x0;
        }
        uVar1 = this->getVote((int32_t)(LD32((uint32_t)((iVar2 + 0x7e4)))), (int32_t)(a2));
        return uVar1;
    }
    return 0xffffffff;
}
#endif

/* setVote(int, int, char) [0x02C74E60] - LOWERED-DRAFT; not reconstructed (6 open edges, 21 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Voteable::setVote(int32_t a1, int32_t a2, int8_t a3)
{
    guest::stub(0x02C74E60u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
}
#else
void Voteable::setVote(int32_t a1, int32_t a2, int8_t a3)
{
    uint32_t _frame = native_frame_open(72);
    int32_t iVar1 = 0;
    if ((a3 == -(1)))
    {
        ST64((_frame + 36), guest::Voteable_getKey((uint32_t)(a1), (uint32_t)(a2)));
        ST32((_frame + 8), (uint32_t)((((map_pair_int_int_char_Voteable_KeyHasher *)gmem((gaddr(this) + 8)))->find((_frame + 36))) >> 32));
        if ((LD32((_frame + 8)) != 0))
        {
            ((map_pair_int_int_char_Voteable_KeyHasher *)gmem((gaddr(this) + 8)))->erase((_frame + 8));
            native_frame_close(_frame, 72);
            return;
        }
    }
    else
    {
        if ((LD8((uint32_t)((gaddr(this) + 5))) != '\0'))
        {
            ST32(_frame, 0xffffffff);
            iVar1 = this->getOnlyVote((int32_t)(a1), (gptr<int32_t>)(_frame));
            if ((iVar1 != 0))
            {
                ST32((_frame + 48), LD32(_frame));
                ST32((_frame + 44), a1);
                ST32((_frame + 12), (uint32_t)((((map_pair_int_int_char_Voteable_KeyHasher *)gmem((gaddr(this) + 8)))->find((_frame + 44))) >> 32));
                ((map_pair_int_int_char_Voteable_KeyHasher *)gmem((gaddr(this) + 8)))->erase((_frame + 12));
            }
        }
        ST64((_frame + 28), guest::Voteable_getKey((uint32_t)(a1), (uint32_t)(a2)));
        iVar1 = (uint32_t)((((map_pair_int_int_char_Voteable_KeyHasher *)gmem((gaddr(this) + 8)))->find((_frame + 28))) >> 32);
        ST8((_frame + 24), (uint8_t)(a3));
        if ((iVar1 != 0))
        {
            ST8((uint32_t)((iVar1 + 8)), LD8((_frame + 24)));
            native_frame_close(_frame, 72);
            return;
        }
        ST64((_frame + 16), guest::Voteable_getKey((uint32_t)(a1), (uint32_t)(a2)));
        ST32((_frame + 4), (_frame + 16));
        ST64((_frame + 52), LD64((_frame + 16)));
        ST64((_frame + 60), LD64((_frame + 16)));
        ((boost_unordered_detail_table_impl_boost_unordered_detail_map_pair_int_int_char_V_dd89ff28 *)gmem((gaddr(this) + 8)))->emplace_impl__tm__98_Q4_5boost9unordered6detail69emplace_args1__tm__48_Q2_3std38pair__tm__26_CQ2_3std14pair__tm__3_iic(LD32((_frame + 4)), (_frame + 4));
    }
    native_frame_close(_frame, 72);
    return;
    native_frame_close(_frame, 72);
}
#endif

/* setVoteAndSync(int, int, char) [0x02C75070] - LOWERED-DRAFT; not reconstructed (17 open edges, 82 raw IR ops, 34 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Voteable::setVoteAndSync(int32_t a1, int32_t a2, int8_t a3)
{
    return (uint64_t)guest::stub(0x02C75070u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
}
#else
uint64_t Voteable::setVoteAndSync(int32_t a1, int32_t a2, int8_t a3)
{
    uint32_t _frame = native_frame_open(48);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint8_t cVar6 = 0;
    int32_t iVar3 = 0;
    uint32_t piVar4 = 0;
    uint32_t piVar5 = 0;
    uint32_t piVar7 = 0;
    int32_t iVar8 = 0;
    int32_t iVar9 = 0;
    uint32_t puVar10 = 0;
    uint32_t uVar11 = 0;
    uint8_t bVar12 = 0;
    uint64_t uVar13 = 0;
    uint64_t __pair = 0;
    cVar6 = guest::Voteable_getVote(gaddr(this), (uint32_t)a1, (uint32_t)a2);
    if ((cVar6 == a3))
    {
        { uint64_t t1 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
        native_frame_close(_frame, 48);
        return t1; }
    }
    this->setVote((int32_t)(a1), (int32_t)(a2), (int8_t)(a3));
    if ((LD8((uint32_t)((gaddr(this) + (1) * 4))) != '\0'))
    {
        uVar11 = 0;
        if ((global_0x104CB6FC != 0))
        {
            uVar11 = LD32((uint32_t)((global_0x104CB6FC + 0x14)));
        }
        iVar3 = guest::VotePacket_ct_2((uint32_t)(0), (uint32_t)(LD32(gaddr(this))), (uint32_t)(a3), (uint32_t)(a1), (uint32_t)(a2));
        piVar4 = (uint32_t)(guest::guest_new((uint32_t)(0x10)));
        if ((piVar4 != (uint32_t)(0x0)))
        {
            piVar5 = piVar4;
            if (((piVar4 != (uint32_t)(0x0)) || (piVar5 = (uint32_t)(guest::guest_new((uint32_t)(0xc))), (piVar5 != (uint32_t)(0x0)))))
            {
                gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) = (int32_t)(GSTR(L"", 0x1030CFBC));
                gelem<int32_t>((uint32_t)(piVar5) + 1u, 0) = 1;
                gelem<int32_t>((uint32_t)(piVar5), 0) = 1;
            }
            gelem<int32_t>((uint32_t)(piVar4) + 3u, 0) = iVar3;
            gelem<int32_t>((uint32_t)(piVar4) + 2u, 0) = (int32_t)(GSTR(L"", 0x1030D244));
        }
        if ((piVar4 == (uint32_t)(0x0)))
        {
            if ((iVar3 != 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((iVar3 + 0xc))) + 0xc)), (uint32_t)(iVar3), (uint32_t)(3)))) >> 32);
            }
            ST32((_frame + 36), GSTR(L"", 0x1030CF24));
            ST32((_frame + 32), GSTR("unknown", 0x1030CE80));
            guest::boost_throw_exception((uint32_t)((_frame + 32)), 0u);
        }
        if ((((iVar3 != 0) && ((uint32_t)((iVar3 + 0x10)) != (uint32_t)(0x0))) && ((LD32((uint32_t)((iVar3 + 0x14))) == (uint32_t)(0x0)) || (LD32(LD32((uint32_t)((iVar3 + 0x14)))) == 0))))
        {
            bVar1 = (piVar4 == (uint32_t)(0x0));
            while (!(bVar1))
            {
                if (('\x01' != '\0'))
                {
                    iVar9 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar4), 0) + 1)), 1);
                    gelem<int32_t>((uint32_t)(piVar4), 0) = iVar9;
                    bVar1 = 1;
                }
            }
            puVar10 = (uint32_t)((iVar3 + 0x14));
            piVar5 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar10), 0));
            ST32((uint32_t)((iVar3 + 0x10)), iVar3);
            ST32((_frame + 8), iVar3);
            ST32((_frame + 12), piVar4);
            if ((piVar4 != piVar5))
            {
                bVar1 = (piVar4 == (uint32_t)(0x0));
                if (!(bVar1))
                {
                    piVar5 = (piVar4 + (1) * 4);
                    do
                    {
                        if (('\x01' != '\0'))
                        {
                            iVar9 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar5), 0) + 1)), 1);
                            gelem<int32_t>((uint32_t)(piVar5), 0) = iVar9;
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    piVar5 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar10), 0));
                }
                bVar1 = (piVar5 == (uint32_t)(0x0));
                if (!(bVar1))
                {
                    piVar7 = (piVar5 + (1) * 4);
                    do
                    {
                        iVar9 = gelem<int32_t>((uint32_t)(piVar7), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar8 = (ST32((uint32_t)(piVar7) + (uint32_t)(0), (iVar9 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar7), 0) = iVar8;
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    if (((iVar9 + -(1)) == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) + 0x1c)), (uint32_t)(piVar5)))) >> 32);
                    }
                }
                gelem<uint32_t>((uint32_t)(puVar10), 0) = piVar4;
            }
            piVar5 = LD32((_frame + 12));
            bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0));
            if (!(bVar1))
            {
                do
                {
                    iVar9 = (LD32(LD32((_frame + 12))) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar8 = (ST32((uint32_t)(LD32((_frame + 12))) + (uint32_t)(0), iVar9), 1);
                        ST32(LD32((_frame + 12)), iVar8);
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                bVar12 = ((iVar9 == 0) << 1);
            }
        }
        bVar1 = (piVar4 == (uint32_t)(0x0));
        while (!(bVar1))
        {
            if (('\x01' != '\0'))
            {
                iVar9 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar4), 0) + 1)), 1);
                gelem<int32_t>((uint32_t)(piVar4), 0) = iVar9;
                bVar1 = 1;
            }
        }
        ST32(_frame, iVar3);
        ST32((_frame + 4), piVar4);
        ((PlayerList *)gmem(uVar11))->broadcastAll((gptr<sp_Packet>)(_frame));
        piVar5 = LD32((_frame + 4));
        bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            do
            {
                iVar3 = (LD32(LD32((_frame + 4))) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar9 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar3), 1);
                    ST32(LD32((_frame + 4)), iVar9);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            bVar12 = ((iVar3 == 0) << 1);
        }
        if ((piVar4 == (uint32_t)(0x0)))
        {
            { uint64_t t2 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
            native_frame_close(_frame, 48);
            return t2; }
        }
        bVar1 = 0;
        do
        {
            iVar3 = (gelem<int32_t>((uint32_t)(piVar4), 0) + -(1));
            if (('\x01' != '\0'))
            {
                iVar9 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), iVar3), 1);
                gelem<int32_t>((uint32_t)(piVar4), 0) = iVar9;
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        bVar12 = ((iVar3 == 0) << 1);
        if ((iVar3 != 0))
        {
            { uint64_t t3 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
            native_frame_close(_frame, 48);
            return t3; }
        }
        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar4) + 2u, 0) + 0x14)), (uint32_t)(piVar4)))) >> 32);
        piVar5 = (piVar4 + (1) * 4);
        do
        {
            iVar3 = gelem<int32_t>((uint32_t)(piVar5), 0);
            if (('\x01' != '\0'))
            {
                iVar9 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar3 + -(1))), 1);
                gelem<int32_t>((uint32_t)(piVar5), 0) = iVar9;
                bVar12 = 2;
            }
        }
        while (!((uint8_t)(((bVar12 >> 1) & 1))));
        if (((iVar3 + -(1)) != 0))
        {
            { uint64_t t4 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
            native_frame_close(_frame, 48);
            return t4; }
        }
        { uint64_t t5 = ((uint64_t)(uint32_t)((uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar4) + 2u, 0) + 0x1c)), (uint32_t)(piVar4)))) >> 32)) << 32);
        native_frame_close(_frame, 48);
        return t5; }
    }
    iVar3 = Minecraft::GetInstance();
    iVar9 = LD32((uint32_t)((iVar3 + 0x34)));
    if ((iVar9 == 0))
    {
        iVar9 = LD32((uint32_t)((iVar3 + 0x34)));
    }
    iVar3 = guest::VotePacket_ct_2((uint32_t)(0), (uint32_t)(LD32(gaddr(this))), (uint32_t)(a3), (uint32_t)(a1), (uint32_t)(a2));
    uVar13 = guest::guest_new((uint32_t)(0x10));
    piVar4 = (uint32_t)(((uint64_t)(uVar13) >> 0x20));
    uVar11 = (uint32_t)(uVar13);
    if ((piVar4 == (uint32_t)(0x0)))
    {
        goto LAB_02c7560c;
    }
    if ((piVar4 == (uint32_t)(0x0)))
    {
        uVar13 = guest::guest_new((uint32_t)(0xc));
        uVar11 = (uint32_t)(uVar13);
        if (((int32_t)(((uint64_t)(uVar13) >> 0x20)) != 0))
        {
            goto LAB_02c755f4;
        }
    }
    else
    {
LAB_02c755f4:;
        puVar10 = (uint32_t)(((uint64_t)(uVar13) >> 0x20));
        uVar11 = (uint32_t)(uVar13);
        gelem<uint32_t>((uint32_t)(puVar10) + 2u, 0) = GSTR(L"", 0x1030CFBC);
        gelem<uint32_t>((uint32_t)(puVar10) + 1u, 0) = 1;
        gelem<uint32_t>((uint32_t)(puVar10), 0) = 1;
    }
    gelem<int32_t>((uint32_t)(piVar4) + 3u, 0) = iVar3;
    gelem<int32_t>((uint32_t)(piVar4) + 2u, 0) = (int32_t)(GSTR(L"", 0x1030D244));
LAB_02c7560c:;
    if ((piVar4 == (uint32_t)(0x0)))
    {
        if ((iVar3 != 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)((iVar3 + 0xc))) + 0xc)), (uint32_t)(iVar3), (uint32_t)(3)))) >> 32);
        }
        ST32((_frame + 44), GSTR(L"", 0x1030CF24));
        ST32((_frame + 40), GSTR("unknown", 0x1030CE80));
        guest::boost_throw_exception((uint32_t)((_frame + 40)), 0u);
    }
    if ((((iVar3 != 0) && ((uint32_t)((iVar3 + 0x10)) != (uint32_t)(0x0))) && ((LD32((uint32_t)((iVar3 + 0x14))) == (uint32_t)(0x0)) || (LD32(LD32((uint32_t)((iVar3 + 0x14)))) == 0))))
    {
        bVar1 = (piVar4 == (uint32_t)(0x0));
        while (!(bVar1))
        {
            if (('\x01' != '\0'))
            {
                iVar8 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar4), 0) + 1)), 1);
                gelem<int32_t>((uint32_t)(piVar4), 0) = iVar8;
                bVar1 = 1;
            }
        }
        puVar10 = (uint32_t)((iVar3 + 0x14));
        piVar5 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar10), 0));
        ST32((uint32_t)((iVar3 + 0x10)), iVar3);
        ST32((_frame + 24), iVar3);
        ST32((_frame + 28), piVar4);
        if ((piVar4 != piVar5))
        {
            bVar1 = (piVar4 == (uint32_t)(0x0));
            if (!(bVar1))
            {
                piVar5 = (piVar4 + (1) * 4);
                do
                {
                    if (('\x01' != '\0'))
                    {
                        iVar8 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar5), 0) + 1)), 1);
                        gelem<int32_t>((uint32_t)(piVar5), 0) = iVar8;
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                piVar5 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar10), 0));
            }
            bVar1 = (piVar5 == (uint32_t)(0x0));
            if (!(bVar1))
            {
                piVar7 = (piVar5 + (1) * 4);
                do
                {
                    iVar8 = gelem<int32_t>((uint32_t)(piVar7), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar2 = (ST32((uint32_t)(piVar7) + (uint32_t)(0), (iVar8 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar7), 0) = iVar2;
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                if (((iVar8 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) + 0x1c)), (uint32_t)(piVar5), (uint32_t)(uVar11), (uint32_t)(piVar7)))) >> 32);
                }
            }
            gelem<uint32_t>((uint32_t)(puVar10), 0) = piVar4;
        }
        piVar5 = LD32((_frame + 28));
        bVar1 = (LD32((_frame + 28)) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            do
            {
                iVar8 = (LD32(LD32((_frame + 28))) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(LD32((_frame + 28))) + (uint32_t)(0), iVar8), 1);
                    ST32(LD32((_frame + 28)), iVar2);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            bVar12 = ((iVar8 == 0) << 1);
        }
    }
    bVar1 = (piVar4 == (uint32_t)(0x0));
    while (!(bVar1))
    {
        if (('\x01' != '\0'))
        {
            iVar8 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar4), 0) + 1)), 1);
            gelem<int32_t>((uint32_t)(piVar4), 0) = iVar8;
            bVar1 = 1;
        }
    }
    ST32((_frame + 16), iVar3);
    ST32((_frame + 20), piVar4);
    ((ClientPacketListener *)gmem(LD32((uint32_t)((iVar9 + 0x878)))))->send((gptr<sp_Packet>)((_frame + 16)));
    piVar5 = LD32((_frame + 20));
    bVar1 = (LD32((_frame + 20)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        do
        {
            iVar3 = (LD32(LD32((_frame + 20))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar9 = (ST32((uint32_t)(LD32((_frame + 20))) + (uint32_t)(0), iVar3), 1);
                ST32(LD32((_frame + 20)), iVar9);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        bVar12 = ((iVar3 == 0) << 1);
    }
    bVar1 = (piVar4 == (uint32_t)(0x0));
    if (!(bVar1))
    {
        do
        {
            iVar3 = (gelem<int32_t>((uint32_t)(piVar4), 0) + -(1));
            if (('\x01' != '\0'))
            {
                iVar9 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), iVar3), 1);
                gelem<int32_t>((uint32_t)(piVar4), 0) = iVar9;
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        bVar12 = ((iVar3 == 0) << 1);
        if ((iVar3 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar4) + 2u, 0) + 0x14)), (uint32_t)(piVar4)))) >> 32);
            piVar5 = (piVar4 + (1) * 4);
            do
            {
                iVar3 = gelem<int32_t>((uint32_t)(piVar5), 0);
                if (('\x01' != '\0'))
                {
                    iVar9 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar3 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar9;
                    bVar12 = 2;
                }
            }
            while (!((uint8_t)(((bVar12 >> 1) & 1))));
            if (((iVar3 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar4) + 2u, 0) + 0x1c)), (uint32_t)(piVar4), (uint32_t)(((uint32_t)__pair)), (uint32_t)(piVar5), (uint32_t)(piVar5)))) >> 32);
            }
        }
    }
    { uint64_t t6 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
    native_frame_close(_frame, 48);
    return t6; }
    native_frame_close(_frame, 48);
}
#endif

/* setIncludedVotesAndSync(int, char) [0x02C76220] - LOWERED-DRAFT; not reconstructed (1 open edges, 13 raw IR ops, 6 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Voteable::setIncludedVotesAndSync(int32_t a1, int8_t a2)
{
    return (uint64_t)guest::stub(0x02C76220u, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}
#else
uint64_t Voteable::setIncludedVotesAndSync(int32_t a1, int8_t a2)
{
    uint32_t _frame = native_frame_open(16);
    int32_t iVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t piVar3 = 0;
    uint32_t puVar4 = 0;
    ((vector_pair_int_int_ *)gmem(_frame))->_Buy((uint32_t)(0));
    if ((LD32((uint32_t)((gaddr(this) + 0x10))) == 0))
    {
        puVar4 = LD32((_frame + 4));
        if ((LD32((_frame + 4)) == LD32((_frame + 8))))
        {
            goto LAB_02c76360;
        }
    }
    else
    {
        iVar1 = LD32((uint32_t)((gaddr(this) + 0x1c)));
        iVar2 = LD32((uint32_t)((gaddr(this) + 0xc)));
        if ((iVar1 == 0))
        {
            iVar1 = LD32((uint32_t)((gaddr(this) + 0x1c)));
        }
        piVar3 = LD32((uint32_t)(((iVar2 * 4) + iVar1)));
        if ((piVar3 != (uint32_t)(0x0)))
        {
            piVar3 = (piVar3 + (-(3)) * 4);
        }
        while ((piVar3 != (uint32_t)(0x0)))
        {
            if ((gelem<int32_t>((uint32_t)(piVar3), 0) == a1))
            {
                ((vector_pair_int_int_ *)gmem(_frame))->push_back(piVar3);
            }
            piVar3 = (uint32_t)(gelem<int32_t>((uint32_t)(piVar3) + 3u, 0));
            if ((piVar3 != (uint32_t)(0x0)))
            {
                piVar3 = (piVar3 + (-(3)) * 4);
            }
        }
        puVar4 = LD32((_frame + 4));
        if ((LD32((_frame + 4)) == LD32((_frame + 8))))
        {
            goto LAB_02c76360;
        }
    }
    do
    {
        this->setVoteAndSync((int32_t)(gelem<uint32_t>((uint32_t)(puVar4), 0)), (int32_t)(gelem<uint32_t>((uint32_t)(puVar4) + 1u, 0)), (int8_t)(a2));
        puVar4 = (puVar4 + (2) * 4);
    }
    while ((puVar4 != LD32((_frame + 8))));
LAB_02c76360:;
    { uint64_t t1 = ((uint64_t)(uint32_t)(guest::vector_pair_int_int___Tidy((uint32_t)(_frame))) << 32);
    native_frame_close(_frame, 16);
    return t1; }
    native_frame_close(_frame, 16);
}
#endif

/* setIncludedVotesAndSync(int, int, char) [0x02C76394] - LOWERED-DRAFT; not reconstructed (13 raw IR ops, 6 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void Voteable::setIncludedVotesAndSync(int32_t a1, int32_t a2, int8_t a3)
{
    guest::stub(0x02C76394u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
}
#else
void Voteable::setIncludedVotesAndSync(int32_t a1, int32_t a2, int8_t a3)
{
    uint32_t _frame = native_frame_open(16);
    int32_t iVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t piVar3 = 0;
    uint32_t puVar4 = 0;
    ((vector_pair_int_int_ *)gmem(_frame))->_Buy((uint32_t)(0));
    if ((LD32((uint32_t)((gaddr(this) + 0x10))) == 0))
    {
        puVar4 = LD32((_frame + 4));
        if ((LD32((_frame + 4)) == LD32((_frame + 8))))
        {
            goto LAB_02c764d8;
        }
    }
    else
    {
        iVar1 = LD32((uint32_t)((gaddr(this) + 0x1c)));
        iVar2 = LD32((uint32_t)((gaddr(this) + 0xc)));
        if ((iVar1 == 0))
        {
            iVar1 = LD32((uint32_t)((gaddr(this) + 0x1c)));
        }
        piVar3 = LD32((uint32_t)((iVar1 + (iVar2 * 4))));
        if ((piVar3 != (uint32_t)(0x0)))
        {
            piVar3 = (piVar3 + (-(3)) * 4);
        }
        while ((piVar3 != (uint32_t)(0x0)))
        {
            if (((gelem<int32_t>((uint32_t)(piVar3), 0) == a1) || (gelem<int32_t>((uint32_t)(piVar3) + 1u, 0) == a2)))
            {
                ((vector_pair_int_int_ *)gmem(_frame))->push_back(piVar3);
            }
            piVar3 = (uint32_t)(gelem<int32_t>((uint32_t)(piVar3) + 3u, 0));
            if ((piVar3 != (uint32_t)(0x0)))
            {
                piVar3 = (piVar3 + (-(3)) * 4);
            }
        }
        puVar4 = LD32((_frame + 4));
        if ((LD32((_frame + 4)) == LD32((_frame + 8))))
        {
            goto LAB_02c764d8;
        }
    }
    do
    {
        this->setVoteAndSync((int32_t)(gelem<uint32_t>((uint32_t)(puVar4), 0)), (int32_t)(gelem<uint32_t>((uint32_t)(puVar4) + 1u, 0)), (int8_t)(a3));
        puVar4 = (puVar4 + (2) * 4);
    }
    while ((puVar4 != LD32((_frame + 8))));
LAB_02c764d8:;
    ((vector_pair_int_int_ *)gmem(_frame))->_Tidy();
    native_frame_close(_frame, 16);
    return;
    native_frame_close(_frame, 16);
}
#endif

/* syncAllVotesToPlayer(boost::shared_ptr<ServerPlayer>) [0x02C76500] - LOWERED-DRAFT; not reconstructed (8 open edges, 48 raw IR ops, 24 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t Voteable::syncAllVotesToPlayer(gptr<sp_ServerPlayer> a1)
{
    return (uint64_t)guest::stub(0x02C76500u, gaddr(this), (uint32_t)a1);
}
#else
uint64_t Voteable::syncAllVotesToPlayer(gptr<sp_ServerPlayer> a1)
{
    uint32_t _frame = native_frame_open(24);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t piVar3 = 0;
    uint32_t piVar4 = 0;
    uint32_t piVar5 = 0;
    int32_t iVar6 = 0;
    int32_t iVar7 = 0;
    uint32_t puVar8 = 0;
    int32_t iVar9 = 0;
    uint32_t puVar10 = 0;
    uint8_t bVar11 = 0;
    if ((LD8((uint32_t)((gaddr(this) + (1) * 4))) != '\0'))
    {
        if ((LD32(gaddr(this) + (4) * 4) == 0))
        {
            { uint64_t t1 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
            native_frame_close(_frame, 24);
            return t1; }
        }
        iVar6 = LD32(gaddr(this) + (7) * 4);
        iVar9 = LD32(gaddr(this) + (3) * 4);
        if ((iVar6 == 0))
        {
            iVar6 = LD32(gaddr(this) + (7) * 4);
        }
        puVar10 = LD32((uint32_t)((iVar6 + (iVar9 * 4))));
        if ((puVar10 != (uint32_t)(0x0)))
        {
            puVar10 = (puVar10 + (-(3)) * 4);
        }
        while ((puVar10 != (uint32_t)(0x0)))
        {
            if ((a1->field_0x0 == 0))
            {
            }
            iVar6 = a1->field_0x0;
            iVar9 = LD32((uint32_t)((iVar6 + 0x86c)));
            if ((iVar9 == 0))
            {
                iVar9 = LD32((uint32_t)((iVar6 + 0x86c)));
            }
            iVar6 = guest::VotePacket_ct_2((uint32_t)(0), (uint32_t)(LD32(gaddr(this))), (uint32_t)((int32_t)(LD8((uint32_t)((puVar10 + (2) * 4))))), (uint32_t)(gelem<uint32_t>((uint32_t)(puVar10), 0)), (uint32_t)(gelem<uint32_t>((uint32_t)(puVar10) + 1u, 0)));
            piVar3 = (uint32_t)(guest::guest_new((uint32_t)(0x10)));
            if ((piVar3 != (uint32_t)(0x0)))
            {
                piVar4 = piVar3;
                if (((piVar3 != (uint32_t)(0x0)) || (piVar4 = (uint32_t)(guest::guest_new((uint32_t)(0xc))), (piVar4 != (uint32_t)(0x0)))))
                {
                    gelem<int32_t>((uint32_t)(piVar4) + 1u, 0) = 1;
                    gelem<int32_t>((uint32_t)(piVar4), 0) = 1;
                    gelem<int32_t>((uint32_t)(piVar4) + 2u, 0) = (int32_t)(GSTR(L"", 0x1030CFBC));
                }
                gelem<int32_t>((uint32_t)(piVar3) + 3u, 0) = iVar6;
                gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) = (int32_t)(GSTR(L"", 0x1030D244));
            }
            if ((piVar3 == (uint32_t)(0x0)))
            {
                if ((iVar6 != 0))
                {
                    (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar6 + 0xc))) + 0xc)), (uint32_t)(iVar6), (uint32_t)(3)) >> 32);
                }
                ST32((_frame + 20), GSTR(L"", 0x1030CF24));
                ST32((_frame + 16), GSTR("unknown", 0x1030CE80));
                guest::boost_throw_exception((uint32_t)((_frame + 16)), 0u);
            }
            if ((((iVar6 != 0) && ((uint32_t)((iVar6 + 0x10)) != (uint32_t)(0x0))) && ((LD32((uint32_t)((iVar6 + 0x14))) == (uint32_t)(0x0)) || (LD32(LD32((uint32_t)((iVar6 + 0x14)))) == 0))))
            {
                bVar1 = (piVar3 == (uint32_t)(0x0));
                while (!(bVar1))
                {
                    if (('\x01' != '\0'))
                    {
                        iVar7 = (ST32((uint32_t)(piVar3) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar3), 0) + 1)), 1);
                        gelem<int32_t>((uint32_t)(piVar3), 0) = iVar7;
                        bVar1 = 1;
                    }
                }
                puVar8 = (uint32_t)((iVar6 + 0x14));
                piVar4 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar8), 0));
                ST32((uint32_t)((iVar6 + 0x10)), iVar6);
                ST32((_frame + 8), iVar6);
                ST32((_frame + 12), piVar3);
                if ((piVar3 != piVar4))
                {
                    bVar1 = (piVar3 == (uint32_t)(0x0));
                    if (!(bVar1))
                    {
                        piVar4 = (piVar3 + (1) * 4);
                        do
                        {
                            if (('\x01' != '\0'))
                            {
                                iVar7 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar4), 0) + 1)), 1);
                                gelem<int32_t>((uint32_t)(piVar4), 0) = iVar7;
                                bVar1 = 1;
                            }
                        }
                        while (!(bVar1));
                        piVar4 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar8), 0));
                    }
                    bVar1 = (piVar4 == (uint32_t)(0x0));
                    if (!(bVar1))
                    {
                        piVar5 = (piVar4 + (1) * 4);
                        do
                        {
                            iVar7 = gelem<int32_t>((uint32_t)(piVar5), 0);
                            if (('\x01' != '\0'))
                            {
                                iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar7 + -(1))), 1);
                                gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                                bVar1 = 1;
                            }
                        }
                        while (!(bVar1));
                        if (((iVar7 + -(1)) == 0))
                        {
                            (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar4) + 2u, 0) + 0x1c)), (uint32_t)(piVar4)) >> 32);
                        }
                    }
                    gelem<uint32_t>((uint32_t)(puVar8), 0) = piVar3;
                }
                piVar4 = LD32((_frame + 12));
                bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0));
                if (!(bVar1))
                {
                    do
                    {
                        iVar7 = (LD32(LD32((_frame + 12))) + -(1));
                        if (('\x01' != '\0'))
                        {
                            iVar2 = (ST32((uint32_t)(LD32((_frame + 12))) + (uint32_t)(0), iVar7), 1);
                            ST32(LD32((_frame + 12)), iVar2);
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    bVar11 = ((iVar7 == 0) << 1);
                }
            }
            bVar1 = (piVar3 == (uint32_t)(0x0));
            while (!(bVar1))
            {
                if (('\x01' != '\0'))
                {
                    iVar7 = (ST32((uint32_t)(piVar3) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar3), 0) + 1)), 1);
                    gelem<int32_t>((uint32_t)(piVar3), 0) = iVar7;
                    bVar1 = 1;
                }
            }
            ST32(_frame, iVar6);
            ST32((_frame + 4), piVar3);
            ((ServerGamePacketListenerImpl *)gmem(iVar9))->send((gptr<sp_Packet>)(_frame));
            piVar4 = LD32((_frame + 4));
            bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
            if (!(bVar1))
            {
                do
                {
                    iVar6 = (LD32(LD32((_frame + 4))) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar9 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar6), 1);
                        ST32(LD32((_frame + 4)), iVar9);
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                bVar11 = ((iVar6 == 0) << 1);
            }
            bVar1 = (piVar3 == (uint32_t)(0x0));
            if (!(bVar1))
            {
                do
                {
                    iVar6 = (gelem<int32_t>((uint32_t)(piVar3), 0) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar9 = (ST32((uint32_t)(piVar3) + (uint32_t)(0), iVar6), 1);
                        gelem<int32_t>((uint32_t)(piVar3), 0) = iVar9;
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                bVar11 = ((iVar6 == 0) << 1);
                if ((iVar6 == 0))
                {
                    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) + 0x14)), (uint32_t)(piVar3)) >> 32);
                    piVar4 = (piVar3 + (1) * 4);
                    do
                    {
                        iVar6 = gelem<int32_t>((uint32_t)(piVar4), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar9 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar6 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar4), 0) = iVar9;
                            bVar11 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar11 >> 1) & 1))));
                    if (((iVar6 + -(1)) == 0))
                    {
                        (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar3) + 2u, 0) + 0x1c)), (uint32_t)(piVar3)) >> 32);
                    }
                }
            }
            puVar10 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar10) + 3u, 0));
            if ((puVar10 != (uint32_t)(0x0)))
            {
                puVar10 = (puVar10 + (-(3)) * 4);
            }
        }
    }
    { uint64_t t2 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
    native_frame_close(_frame, 24);
    return t2; }
    native_frame_close(_frame, 24);
}
#endif

/* clearVotes() [0x02C76B14] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02C76B14
void Voteable::clearVotes()
{
    ((map_pair_int_int_char_Voteable_KeyHasher *)gmem((gaddr(this) + 8)))->clear();
    return;
}
#else
void Voteable::clearVotes()
{
    guest::stub(0x02C76B14u, gaddr(this));
}
#endif

}  /* namespace recon */
