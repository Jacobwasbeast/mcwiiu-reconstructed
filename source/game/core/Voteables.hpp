/* Voteables - generated reconstruction header. */
#ifndef RECON_C_GAME_V_VOTEABLES_HPP
#define RECON_C_GAME_V_VOTEABLES_HPP 1

#include "guest_ns.hpp"
#include "_thirdparty/boost__unordered__unordered_map_int__Voteable___boost__hash_int___std__equal_to_int___std__allocator_std__pair_const_int.hpp"

namespace recon {
struct Voteable;
struct sp_ServerPlayer;

#pragma pack(push, 1)
struct Voteables : map_int_Voteable_p {
    be<uint32_t> vfptr;   /* vtable */

    /* interface: 4 methods recovered, 1 without a recoverable signature;
     * implementation: 1 reconstructed bodies, 3 lowered drafts, 1 closed */
    Voteables();   /* boundary stub */
    ~Voteables();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t getVoteable(uint32_t a1);   /* LOWERED-DRAFT; not reconstructed */
    void init();   /* closed */
    void addVoteable(gptr<Voteable> a1);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t syncAllVoteablesToPlayer(gptr<sp_ServerPlayer> a1);   /* LOWERED-DRAFT; not reconstructed */
    /* unrecovered: removeVoteable(int) [0x00000000] */
};
static_assert(sizeof(Voteables) == 0x1c, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
