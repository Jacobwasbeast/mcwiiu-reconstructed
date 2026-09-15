/* WeighedTreasure - generated reconstruction header. */
#ifndef RECON_C_GAME_W_WEIGHEDTREASURE_HPP
#define RECON_C_GAME_W_WEIGHEDTREASURE_HPP 1

#include "guest_ns.hpp"

namespace recon {
struct Item;
struct Random;
struct not_null_ptr_ItemInstance;
struct sp_Container;

#pragma pack(push, 1)
struct WeighedTreasure {
    be<int32_t> field_0x0;
    be<int32_t> vfptr;
    be<int32_t> field_0x8;
    be<uint32_t> field_0xc;   /* ptr */
    be<int32_t> field_0x10;
    be<int32_t> field_0x14;

    /* interface: 2 methods recovered, 1 without a recoverable signature;
     * implementation: 0 reconstructed bodies, 2 lowered drafts, 0 closed */
    WeighedTreasure(gptr<Item> a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5);   /* LOWERED-DRAFT; not reconstructed */
    WeighedTreasure(gptr<not_null_ptr_ItemInstance> a1, int32_t a2, int32_t a3, int32_t a4);   /* LOWERED-DRAFT; not reconstructed */
    ~WeighedTreasure();   /* LOWERED-DRAFT; not reconstructed */
    WeighedTreasure() {}   /* generated shell; the real ctor is dispatch-only */
    void getItem();   /* LOWERED-DRAFT; not reconstructed */
    static void addChestItems(gptr<Random> a1, uint32_t a2, gptr<sp_Container> a3, int32_t a4, bool a5);   /* LOWERED-DRAFT; not reconstructed */
    /* unrecovered: addToTreasure(arrayWithLength<WeighedTreasure*>, WeighedTreasure*) [static] [0x00000000] */
};
static_assert(sizeof(WeighedTreasure) == 0x18, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
