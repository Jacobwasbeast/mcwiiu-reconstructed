/* Voteable - generated reconstruction header. */
#ifndef RECON_C_GAME_V_VOTEABLE_HPP
#define RECON_C_GAME_V_VOTEABLE_HPP 1

#include "guest_ns.hpp"
#include "_thirdparty/boost__unordered__unordered_map_std__pair_int__int___char__Voteable__KeyHasher__std__equal_to_std__pair_int__int____std_.hpp"

namespace recon {
struct map_int_int_;
struct sp_MultiplayerLocalPlayer;
struct sp_ServerPlayer;

#pragma pack(push, 1)
struct Voteable {
    /* Voteable; member map_pair_int_int_char_Voteable_KeyHasher at 0x2 */
    uint8_t pad_0x0[1];
    be<uint8_t> field_0x1;
    uint8_t pad_0x2[2];
    be<int32_t> field_0x4;   /* boost::unordered::unordered_map<std::pair<int, int>, char, Voteable::KeyHasher, std::equal_to<std::pair<int, int>>, std::allocator<std::pair<const std::pair<int, int>, char>>> */
    map_pair_int_int_char_Voteable_KeyHasher m_0x8;

    /* interface: 14 methods recovered, 2 without a recoverable signature;
     * implementation: 2 reconstructed bodies, 12 lowered drafts, 1 closed */
    Voteable(int32_t a1, bool a2, bool a3);   /* LOWERED-DRAFT; not reconstructed */
    ~Voteable();   /* LOWERED-DRAFT; not reconstructed */
    Voteable() {}   /* generated shell; the real ctor is dispatch-only */
    uint64_t getVotesFromPlayer(int32_t a1, int32_t a2);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t getVotesToVoteable(int32_t a1, int32_t a2);   /* LOWERED-DRAFT; not reconstructed */
    gresult<map_int_int_> getTotalledVotes();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t getWinningVote(gptr<int32_t> a1);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t getOnlyVote(int32_t a1, gptr<int32_t> a2);   /* LOWERED-DRAFT; not reconstructed */
    static void getKey(int32_t a1, int32_t a2);   /* closed */
    int32_t getVote(int32_t a1, int32_t a2);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t getVote(gptr<sp_MultiplayerLocalPlayer> a1, int32_t a2);   /* LOWERED-DRAFT; not reconstructed */
    void setVote(int32_t a1, int32_t a2, int8_t a3);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t setVoteAndSync(int32_t a1, int32_t a2, int8_t a3);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t setIncludedVotesAndSync(int32_t a1, int8_t a2);   /* LOWERED-DRAFT; not reconstructed */
    void setIncludedVotesAndSync(int32_t a1, int32_t a2, int8_t a3);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t syncAllVotesToPlayer(gptr<sp_ServerPlayer> a1);   /* LOWERED-DRAFT; not reconstructed */
    void clearVotes();
    /* unrecovered: setVote(int, char) [0x00000000] */
    /* unrecovered: setVoteAndSync(int, char) [0x00000000] */
};
static_assert(sizeof(Voteable) == 0x20, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
