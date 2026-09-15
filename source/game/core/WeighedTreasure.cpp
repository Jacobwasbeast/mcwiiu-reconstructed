#include "game/W/WeighedTreasure.hpp"
#include "game/C/Container.hpp"
#include "game/I/ItemInstance.hpp"
#include "game/R/Random.hpp"
#include "_thirdparty/boost__shared_ptr_Container_.hpp"
#include "game/N/not_null_ptr_ItemInstance_.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(Item*, int, int, int, int) [0x02AAA804] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WeighedTreasure::WeighedTreasure(gptr<Item> a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5)
{
    guest::stub(0x02AAA804u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, (uint32_t)a4, (uint32_t)a5);
}
#else
WeighedTreasure::WeighedTreasure(gptr<Item> a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5)
{
    uint32_t _frame = native_frame_open(8);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t puVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t piVar5 = 0;
    uint32_t piVar6 = 0;
    uint32_t extraout_r4 = 0;
    uint32_t uVar7 = 0;
    int32_t iVar8 = 0;
    uint32_t piVar9 = 0;
    uint8_t bVar10 = 0;
    uint64_t uVar11 = 0;
    uint64_t __pair = 0;
    puVar3 = (gaddr(this) + (2) * 4);
    this->field_0x0 = a5;
    this->vfptr = GSTR(L"", 0x102AAFCC);
    if (((puVar3 != (uint32_t)(0x0)) || (puVar3 = (uint32_t)(guest::guest_new((uint32_t)(8))), (puVar3 != (uint32_t)(0x0)))))
    {
        gelem<uint32_t>((uint32_t)(puVar3) + 1u, 0) = 0;
        gelem<uint32_t>((uint32_t)(puVar3), 0) = 0;
    }
    iVar4 = guest::ItemInstance_ct_3((uint32_t)(0), (uint32_t)(a1), (uint32_t)(1), (uint32_t)(a2));
    uVar11 = guest::guest_new((uint32_t)(0x10));
    piVar5 = (uint32_t)(((uint64_t)(uVar11) >> 0x20));
    uVar7 = (uint32_t)(uVar11);
    if ((piVar5 == (uint32_t)(0x0)))
    {
        goto LAB_02aaa924;
    }
    if ((piVar5 == (uint32_t)(0x0)))
    {
        uVar11 = guest::guest_new((uint32_t)(0xc));
        uVar7 = (uint32_t)(uVar11);
        if (((int32_t)(((uint64_t)(uVar11) >> 0x20)) != 0))
        {
            goto LAB_02aaa8fc;
        }
    }
    else
    {
LAB_02aaa8fc:;
        puVar3 = (uint32_t)(((uint64_t)(uVar11) >> 0x20));
        uVar7 = (uint32_t)(uVar11);
        gelem<uint32_t>((uint32_t)(puVar3) + 1u, 0) = 1;
        gelem<uint32_t>((uint32_t)(puVar3) + 2u, 0) = GSTR(L"", 0x102AAB8C);
        gelem<uint32_t>((uint32_t)(puVar3), 0) = 1;
    }
    gelem<int32_t>((uint32_t)(piVar5) + 3u, 0) = iVar4;
    gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) = (int32_t)(GSTR(L"", 0x102AACBC));
LAB_02aaa924:;
    if ((piVar5 == (uint32_t)(0x0)))
    {
        guest::ItemInstance_dt((uint32_t)(iVar4), (uint32_t)(3));
        ST32((_frame + 4), GSTR(L"", 0x102AAAF4));
        ST32(_frame, GSTR("unknown", 0x102AAA80));
        guest::boost_throw_exception((uint32_t)(_frame), 0u);
        uVar7 = 0u /* unmodelled extraout_r4 */;
    }
    if (((iVar4 != 0) && ((LD32((uint32_t)((iVar4 + 4))) == (uint32_t)(0x0)) || (LD32(LD32((uint32_t)((iVar4 + 4)))) == 0))))
    {
        bVar1 = (piVar5 == (uint32_t)(0x0));
        while (!(bVar1))
        {
            if (('\x01' != '\0'))
            {
                iVar8 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar5), 0) + 1)), 1);
                gelem<int32_t>((uint32_t)(piVar5), 0) = iVar8;
                bVar1 = 1;
            }
        }
        puVar3 = (uint32_t)((iVar4 + 4));
        piVar6 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar3), 0));
        ST32((uint32_t)(iVar4), iVar4);
        if ((piVar5 != piVar6))
        {
            bVar1 = (piVar5 == (uint32_t)(0x0));
            if (!(bVar1))
            {
                piVar6 = (piVar5 + (1) * 4);
                do
                {
                    if (('\x01' != '\0'))
                    {
                        iVar8 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar6), 0) + 1)), 1);
                        gelem<int32_t>((uint32_t)(piVar6), 0) = iVar8;
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                piVar6 = (uint32_t)(gelem<uint32_t>((uint32_t)(puVar3), 0));
            }
            bVar1 = (piVar6 == (uint32_t)(0x0));
            if (!(bVar1))
            {
                piVar9 = (piVar6 + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar8 = gelem<int32_t>((uint32_t)(piVar9), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar2 = (ST32((uint32_t)(piVar9) + (uint32_t)(0), (iVar8 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar9), 0) = iVar2;
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar8 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)))) >> 32);
                    uVar7 = ((uint32_t)__pair);
                }
            }
            gelem<uint32_t>((uint32_t)(puVar3), 0) = piVar5;
        }
        bVar1 = (piVar5 == (uint32_t)(0x0));
        if (!(bVar1))
        {
            ((void)0 /* sync dropped */);
            do
            {
                iVar8 = (gelem<int32_t>((uint32_t)(piVar5), 0) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), iVar8), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            bVar10 = ((iVar8 == 0) << 1);
            if ((iVar8 == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) + 0x14)), (uint32_t)(piVar5)))) >> 32);
                piVar6 = (piVar5 + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar8 = gelem<int32_t>((uint32_t)(piVar6), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar2 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar8 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar6), 0) = iVar2;
                        bVar10 = 2;
                    }
                }
                while (!((uint8_t)(((bVar10 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                uVar7 = ((uint32_t)__pair);
                if (((iVar8 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) + 0x1c)), (uint32_t)(piVar5)))) >> 32);
                    uVar7 = ((uint32_t)__pair);
                }
            }
        }
    }
    bVar1 = (piVar5 == (uint32_t)(0x0));
    while (!(bVar1))
    {
        if (('\x01' != '\0'))
        {
            iVar8 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar5), 0) + 1)), 1);
            gelem<int32_t>((uint32_t)(piVar5), 0) = iVar8;
            bVar1 = 1;
        }
    }
    piVar6 = (uint32_t)(this->field_0xc);
    this->field_0xc = piVar5;
    bVar1 = (piVar6 == (uint32_t)(0x0));
    this->field_0x8 = iVar4;
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar4 = (gelem<int32_t>((uint32_t)(piVar6), 0) + -(1));
            if (('\x01' != '\0'))
            {
                iVar8 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), iVar4), 1);
                gelem<int32_t>((uint32_t)(piVar6), 0) = iVar8;
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar10 = ((iVar4 == 0) << 1);
        if ((iVar4 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x14)), (uint32_t)(piVar6), (uint32_t)(uVar7), (uint32_t)(0), (uint32_t)(piVar6), (uint32_t)(piVar6)))) >> 32);
            piVar9 = (piVar6 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar9), 0);
                if (('\x01' != '\0'))
                {
                    iVar8 = (ST32((uint32_t)(piVar9) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar9), 0) = iVar8;
                    bVar10 = 2;
                }
            }
            while (!((uint8_t)(((bVar10 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)))) >> 32);
            }
        }
    }
    bVar1 = (piVar5 == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar4 = (gelem<int32_t>((uint32_t)(piVar5), 0) + -(1));
            if (('\x01' != '\0'))
            {
                iVar8 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), iVar4), 1);
                gelem<int32_t>((uint32_t)(piVar5), 0) = iVar8;
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar10 = ((iVar4 == 0) << 1);
        if ((iVar4 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) + 0x14)), (uint32_t)(piVar5)))) >> 32);
            piVar6 = (piVar5 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar6), 0);
                if (('\x01' != '\0'))
                {
                    iVar8 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar6), 0) = iVar8;
                    bVar10 = 2;
                }
            }
            while (!((uint8_t)(((bVar10 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) + 0x1c)), (uint32_t)(piVar5)))) >> 32);
            }
        }
    }
    this->field_0x14 = a4;
    this->field_0x10 = a3;
    native_frame_close(_frame, 8);
}
#endif

/* {ctor}(not_null_ptr<ItemInstance>, int, int, int) [0x02AAACCC] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WeighedTreasure::WeighedTreasure(gptr<not_null_ptr_ItemInstance> a1, int32_t a2, int32_t a3, int32_t a4)
{
    guest::stub(0x02AAACCCu, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, (uint32_t)a4);
}
#else
WeighedTreasure::WeighedTreasure(gptr<not_null_ptr_ItemInstance> a1, int32_t a2, int32_t a3, int32_t a4)
{
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t puVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t uVar5 = 0;
    uint32_t piVar6 = 0;
    uint32_t piVar7 = 0;
    uint8_t bVar8 = 0;
    this->field_0x0 = a4;
    puVar3 = (gaddr(this) + (2) * 4);
    this->vfptr = GSTR(L"", 0x102AAFCC);
    if (((puVar3 != (uint32_t)(0x0)) || (puVar3 = (uint32_t)(guest::guest_new((uint32_t)(8))), (puVar3 != (uint32_t)(0x0)))))
    {
        gelem<uint32_t>((uint32_t)(puVar3) + 1u, 0) = 0;
        gelem<uint32_t>((uint32_t)(puVar3), 0) = 0;
    }
    uVar5 = a1->field_0x0;
    piVar7 = (uint32_t)(a1->field_0x4);
    bVar1 = (piVar7 == (uint32_t)(0x0));
    while (!(bVar1))
    {
        if (('\x01' != '\0'))
        {
            iVar4 = (ST32((uint32_t)(piVar7) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar7), 0) + 1)), 1);
            gelem<int32_t>((uint32_t)(piVar7), 0) = iVar4;
            bVar1 = 1;
        }
    }
    piVar6 = (uint32_t)(this->field_0xc);
    this->field_0xc = piVar7;
    bVar1 = (piVar6 == (uint32_t)(0x0));
    this->field_0x8 = uVar5;
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar4 = (gelem<int32_t>((uint32_t)(piVar6), 0) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), iVar4), 1);
                gelem<int32_t>((uint32_t)(piVar6), 0) = iVar2;
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar8 = ((iVar4 == 0) << 1);
        if ((iVar4 == 0))
        {
            (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x14)), (uint32_t)(piVar6)) >> 32);
            piVar7 = (piVar6 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar7), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar7) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar7), 0) = iVar2;
                    bVar8 = 2;
                }
            }
            while (!((uint8_t)(((bVar8 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)) >> 32);
            }
        }
    }
    this->field_0x14 = a3;
    this->field_0x10 = a2;
}
#endif

/* {dtor}() [0x02AD1C6C] - LOWERED-DRAFT; not reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WeighedTreasure::~WeighedTreasure()
{
    guest::stub(0x02AD1C6Cu, gaddr(this), 0u);
}
#else
WeighedTreasure::~WeighedTreasure()
{
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    uint32_t piVar4 = 0;
    uint32_t piVar5 = 0;
    uint8_t bVar6 = 0;
    if (((uint32_t)((gaddr(this) + 0xc)) != (uint32_t)(0x0)))
    {
        piVar5 = LD32((uint32_t)((gaddr(this) + 0xc)));
        bVar1 = (piVar5 == (uint32_t)(0x0));
        if (!(bVar1))
        {
            ((void)0 /* sync dropped */);
            do
            {
                iVar3 = (gelem<int32_t>((uint32_t)(piVar5), 0) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), iVar3), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            bVar6 = ((iVar3 == 0) << 1);
            if ((iVar3 == 0))
            {
                (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) + 0x14)), (uint32_t)(piVar5)) >> 32);
                piVar4 = (piVar5 + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar3 = gelem<int32_t>((uint32_t)(piVar4), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar2 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar3 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar4), 0) = iVar2;
                        bVar6 = 2;
                    }
                }
                while (!((uint8_t)(((bVar6 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar3 + -(1)) == 0))
                {
                    (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar5) + 2u, 0) + 0x1c)), (uint32_t)(piVar5)) >> 32);
                }
            }
        }
    }
}
#endif

/* getItem() [0x02AAAE44] - LOWERED-DRAFT; not reconstructed (3 open edges, 2 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WeighedTreasure::getItem()
{
    guest::stub(0x02AAAE44u, gaddr(this));
}
#else
void WeighedTreasure::getItem(uint32_t a1)
{
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint32_t puVar3 = 0;
    uint32_t piVar4 = 0;
    if ((((a1 != (uint32_t)(0x0)) || (a1 = (uint32_t)(guest::guest_new((uint32_t)(8))), (a1 != (uint32_t)(0x0)))) && ((a1 != (uint32_t)(0x0)) || (a1 = (uint32_t)(guest::guest_new((uint32_t)(8))), (a1 != (uint32_t)(0x0))))))
    {
        puVar3 = (a1 + (1) * 4);
        gelem<uint32_t>((uint32_t)(a1), 0) = this->field_0x8;
        if (((puVar3 != (uint32_t)(0x0)) || (puVar3 = (uint32_t)(guest::guest_new((uint32_t)(4))), (puVar3 != (uint32_t)(0x0)))))
        {
            piVar4 = LD32((uint32_t)((gaddr(this) + 0xc)));
            bVar1 = (piVar4 == (uint32_t)(0x0));
            gelem<uint32_t>((uint32_t)(puVar3), 0) = piVar4;
            while (!(bVar1))
            {
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar4), 0) + 1)), 1);
                    gelem<int32_t>((uint32_t)(piVar4), 0) = iVar2;
                    bVar1 = 1;
                }
            }
        }
    }
    return;
}
#endif

/* addChestItems(Random*, arrayWithLength<WeighedTreasure*>, boost::shared_ptr<Container>, int, bool) [static] [0x02AAAF04] - LOWERED-DRAFT; not reconstructed (55 open edges, 252 raw IR ops, 79 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WeighedTreasure::addChestItems(gptr<Random> a1, uint32_t a2, gptr<sp_Container> a3, int32_t a4, bool a5)
{
    guest::stub(0x02AAAF04u, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, (uint32_t)a4, (uint32_t)a5);
}
#else
void WeighedTreasure::addChestItems(gptr<Random> a1, uint32_t a2, gptr<sp_Container> a3, int32_t a4, bool a5)
{
    uint32_t _frame = native_frame_open(64);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t uVar5 = 0;
    uint32_t piVar6 = 0;
    uint32_t extraout_r4 = 0;
    int32_t iVar7 = 0;
    uint32_t piVar8 = 0;
    uint32_t piVar9 = 0;
    int32_t iVar10 = 0;
    uint32_t pcVar11 = 0;
    uint8_t bVar12 = 0;
    uint64_t uVar13 = 0;
    uint64_t __pair = 0;
    if ((0 < a4))
    {
        do
        {
            while (1)
            {
                ST32((_frame + 52), gelem<uint32_t>((uint32_t)(a2) + 1u, 0));
                ST32((_frame + 48), gelem<uint32_t>((uint32_t)(a2), 0));
                ST32((_frame + 56), LD32((_frame + 48)));
                ST32((_frame + 60), LD32((_frame + 52)));
                iVar3 = (uint32_t)(((__pair = (uint64_t)(guest::WeighedRandom_getRandomItem((uint32_t)(a1), (uint32_t)((_frame + 56)))))) >> 32);
                iVar10 = LD32((uint32_t)((iVar3 + 0x10)));
                iVar4 = a1->nextInt((int32_t)(((LD32((uint32_t)((iVar3 + 0x14))) - iVar10) + 1)));
                iVar7 = LD32((uint32_t)((iVar3 + 8)));
                iVar10 = (iVar10 + iVar4);
                if ((iVar7 == 0))
                {
                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C098C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                    iVar7 = LD32((uint32_t)((iVar3 + 8)));
                }
                iVar4 = (uint32_t)(((__pair = (uint64_t)(((ItemInstance *)gmem(iVar7))->getMaxStackSize()))) >> 32);
                if ((iVar10 <= iVar4))
                {
                    break;
                }
                if ((a5 == 0))
                {
                    if ((0 < iVar10))
                    {
                        do
                        {
                            iVar4 = LD32((uint32_t)((iVar3 + 8)));
                            if ((iVar4 == 0))
                            {
                                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C098C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                                iVar4 = LD32((uint32_t)((iVar3 + 8)));
                            }
                            (__pair = (uint64_t)(((ItemInstance *)gmem(iVar4))->copy((_frame + 8))));
                            if ((LD32((_frame + 8)) == 0))
                            {
                                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C098C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                            }
                            ((ItemInstance *)gmem(LD32((_frame + 8))))->setCount((int32_t)(1));
                            iVar4 = a3->field_0x0;
                            if ((iVar4 == 0))
                            {
                                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                                iVar4 = a3->field_0x0;
                            }
                            piVar6 = (uint32_t)(a3->field_0x0);
                            iVar7 = gelem<int32_t>((uint32_t)(piVar6), 0);
                            if ((iVar4 == 0))
                            {
                                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                            }
                            iVar4 = LD32((uint32_t)(a3->field_0x0));
                            bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0));
                            ST32((_frame + 16), LD32((_frame + 8)));
                            ST32((_frame + 20), LD32((_frame + 12)));
                            while (!(bVar1))
                            {
                                if (('\x01' != '\0'))
                                {
                                    iVar2 = (ST32((uint32_t)(LD32((_frame + 12))) + (uint32_t)(0), (LD32(LD32((_frame + 12))) + 1)), 1);
                                    ST32(LD32((_frame + 12)), iVar2);
                                    bVar1 = 1;
                                }
                            }
                            pcVar11 = LD32((uint32_t)((iVar7 + 0x3c)));
                            uVar5 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar4 + 0x14))))) >> 32);
                            uVar5 = a1->nextInt((int32_t)(uVar5));
                            (uint32_t)((__pair = (uint64_t)(guest::indirect((uint32_t)(pcVar11), (uint32_t)(piVar6), (uint32_t)(uVar5), (uint32_t)((_frame + 16))))) >> 32);
                            piVar6 = LD32((_frame + 20));
                            bVar1 = (LD32((_frame + 20)) == (uint32_t)(0x0));
                            if (!(bVar1))
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
                                bVar12 = ((iVar4 == 0) << 1);
                                if ((iVar4 == 0))
                                {
                                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 20)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 20))), (uint32_t)(LD32((_frame + 20)))))) >> 32);
                                    piVar8 = (piVar6 + (1) * 4);
                                    ((void)0 /* sync dropped */);
                                    do
                                    {
                                        iVar4 = gelem<int32_t>((uint32_t)(piVar8), 0);
                                        if (('\x01' != '\0'))
                                        {
                                            iVar7 = (ST32((uint32_t)(piVar8) + (uint32_t)(0), (iVar4 + -(1))), 1);
                                            gelem<int32_t>((uint32_t)(piVar8), 0) = iVar7;
                                            bVar12 = 2;
                                        }
                                    }
                                    while (!((uint8_t)(((bVar12 >> 1) & 1))));
                                    ((void)0 /* instructionSynchronize dropped */);
                                    if (((iVar4 + -(1)) == 0))
                                    {
                                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)))) >> 32);
                                    }
                                }
                            }
                            piVar6 = LD32((_frame + 12));
                            bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0));
                            if (!(bVar1))
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
                                bVar12 = ((iVar4 == 0) << 1);
                                if ((iVar4 == 0))
                                {
                                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 12)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 12)))))) >> 32);
                                    piVar8 = (piVar6 + (1) * 4);
                                    ((void)0 /* sync dropped */);
                                    do
                                    {
                                        iVar4 = gelem<int32_t>((uint32_t)(piVar8), 0);
                                        if (('\x01' != '\0'))
                                        {
                                            iVar7 = (ST32((uint32_t)(piVar8) + (uint32_t)(0), (iVar4 + -(1))), 1);
                                            gelem<int32_t>((uint32_t)(piVar8), 0) = iVar7;
                                            bVar12 = 2;
                                        }
                                    }
                                    while (!((uint8_t)(((bVar12 >> 1) & 1))));
                                    ((void)0 /* instructionSynchronize dropped */);
                                    if (((iVar4 + -(1)) == 0))
                                    {
                                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)))) >> 32);
                                    }
                                }
                            }
                            iVar10 = (iVar10 + -(1));
                        }
                        while ((iVar10 != 0));
                    }
                }
                else
                {
                    if ((0 < iVar10))
                    {
                        do
                        {
                            iVar4 = LD32((uint32_t)((iVar3 + 8)));
                            if ((iVar4 == 0))
                            {
                                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C098C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                                iVar4 = LD32((uint32_t)((iVar3 + 8)));
                            }
                            (__pair = (uint64_t)(((ItemInstance *)gmem(iVar4))->copy((_frame + 8))));
                            if ((LD32((_frame + 8)) == 0))
                            {
                                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C098C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                            }
                            ((ItemInstance *)gmem(LD32((_frame + 8))))->setCount((int32_t)(1));
                            if ((a3->field_0x0 == 0))
                            {
                                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                            }
                            uVar13 = ((Container *)gmem(a3->field_0x0))->isFull();
                            piVar6 = (uint32_t)(uVar13);
                            if (((int32_t)(((uint64_t)(uVar13) >> 0x20)) != 0))
                            {
                                if ((LD32((_frame + 12)) == (uint32_t)(0x0)))
                                {
                                    native_frame_close(_frame, 64);
                                    return;
                                }
                                ((void)0 /* sync dropped */);
                                bVar1 = 0;
                                do
                                {
                                    iVar3 = (LD32(LD32((_frame + 12))) + -(1));
                                    if (('\x01' != '\0'))
                                    {
                                        iVar4 = (ST32((uint32_t)(LD32((_frame + 12))) + (uint32_t)(0), iVar3), 1);
                                        ST32(LD32((_frame + 12)), iVar4);
                                        bVar1 = 1;
                                    }
                                }
                                while (!(bVar1));
                                ((void)0 /* instructionSynchronize dropped */);
                                bVar12 = ((iVar3 == 0) << 1);
                                if ((iVar3 != 0))
                                {
                                    native_frame_close(_frame, 64);
                                    return;
                                }
                                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 12)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 12))), (uint32_t)(LD32((_frame + 12)))))) >> 32);
                                piVar6 = (LD32((_frame + 12)) + (1) * 4);
                                ((void)0 /* sync dropped */);
                                do
                                {
                                    iVar3 = gelem<int32_t>((uint32_t)(piVar6), 0);
                                    if (('\x01' != '\0'))
                                    {
                                        iVar4 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar3 + -(1))), 1);
                                        gelem<int32_t>((uint32_t)(piVar6), 0) = iVar4;
                                        bVar12 = 2;
                                    }
                                }
                                while (!((uint8_t)(((bVar12 >> 1) & 1))));
                                ((void)0 /* instructionSynchronize dropped */);
                                if (((iVar3 + -(1)) != 0))
                                {
                                    native_frame_close(_frame, 64);
                                    return;
                                }
                                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 12)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 12)))))) >> 32);
                                native_frame_close(_frame, 64);
                                return;
                            }
                            do
                            {
                                if ((a3->field_0x0 == 0))
                                {
                                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                                    piVar6 = 0u /* unmodelled extraout_r4 */;
                                }
                                uVar5 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)(a3->field_0x0)) + 0x14)), (uint32_t)((uint32_t)(a3->field_0x0)), (uint32_t)(piVar6)))) >> 32);
                                uVar5 = a1->nextInt((int32_t)(uVar5));
                                if ((a3->field_0x0 == 0))
                                {
                                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                                }
                                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)(a3->field_0x0)) + 0x24)), (uint32_t)((uint32_t)(a3->field_0x0)), (uint32_t)(_frame), (uint32_t)(uVar5)))) >> 32);
                                if ((LD32(_frame) == 0))
                                {
                                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C098C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                                }
                                uVar13 = ((ItemInstance *)gmem(LD32(_frame)))->isEmpty();
                                piVar8 = LD32((_frame + 4));
                                bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
                                piVar6 = (uint32_t)(uVar13);
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
                                    bVar12 = ((iVar4 == 0) << 1);
                                    piVar6 = LD32((_frame + 4));
                                    if ((iVar4 == 0))
                                    {
                                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                                        piVar6 = (piVar8 + (1) * 4);
                                        ((void)0 /* sync dropped */);
                                        do
                                        {
                                            iVar4 = gelem<int32_t>((uint32_t)(piVar6), 0);
                                            if (('\x01' != '\0'))
                                            {
                                                iVar7 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar4 + -(1))), 1);
                                                gelem<int32_t>((uint32_t)(piVar6), 0) = iVar7;
                                                bVar12 = 2;
                                            }
                                        }
                                        while (!((uint8_t)(((bVar12 >> 1) & 1))));
                                        ((void)0 /* instructionSynchronize dropped */);
                                        piVar6 = ((uint32_t)__pair);
                                        if (((iVar4 + -(1)) == 0))
                                        {
                                            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar8) + 2u, 0) + 0x1c)), (uint32_t)(piVar8)))) >> 32);
                                            piVar6 = ((uint32_t)__pair);
                                        }
                                    }
                                }
                            }
                            while (((int32_t)(((uint64_t)(uVar13) >> 0x20)) != 1));
                            if ((a3->field_0x0 == 0))
                            {
                                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                            }
                            piVar6 = (uint32_t)(a3->field_0x0);
                            iVar4 = gelem<int32_t>((uint32_t)(piVar6), 0);
                            bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0));
                            ST32(_frame, LD32((_frame + 8)));
                            ST32((_frame + 4), LD32((_frame + 12)));
                            while (!(bVar1))
                            {
                                if (('\x01' != '\0'))
                                {
                                    iVar7 = (ST32((uint32_t)(LD32((_frame + 12))) + (uint32_t)(0), (LD32(LD32((_frame + 12))) + 1)), 1);
                                    ST32(LD32((_frame + 12)), iVar7);
                                    bVar1 = 1;
                                }
                            }
                            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar4 + 0x3c)), (uint32_t)(piVar6), (uint32_t)(uVar5), (uint32_t)(_frame)))) >> 32);
                            piVar6 = LD32((_frame + 4));
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
                                bVar12 = ((iVar4 == 0) << 1);
                                if ((iVar4 == 0))
                                {
                                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                                    piVar8 = (piVar6 + (1) * 4);
                                    ((void)0 /* sync dropped */);
                                    do
                                    {
                                        iVar4 = gelem<int32_t>((uint32_t)(piVar8), 0);
                                        if (('\x01' != '\0'))
                                        {
                                            iVar7 = (ST32((uint32_t)(piVar8) + (uint32_t)(0), (iVar4 + -(1))), 1);
                                            gelem<int32_t>((uint32_t)(piVar8), 0) = iVar7;
                                            bVar12 = 2;
                                        }
                                    }
                                    while (!((uint8_t)(((bVar12 >> 1) & 1))));
                                    ((void)0 /* instructionSynchronize dropped */);
                                    if (((iVar4 + -(1)) == 0))
                                    {
                                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)))) >> 32);
                                    }
                                }
                            }
                            piVar6 = LD32((_frame + 12));
                            bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0));
                            if (!(bVar1))
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
                                bVar12 = ((iVar4 == 0) << 1);
                                if ((iVar4 == 0))
                                {
                                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 12)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 12)))))) >> 32);
                                    piVar8 = (piVar6 + (1) * 4);
                                    ((void)0 /* sync dropped */);
                                    do
                                    {
                                        iVar4 = gelem<int32_t>((uint32_t)(piVar8), 0);
                                        if (('\x01' != '\0'))
                                        {
                                            iVar7 = (ST32((uint32_t)(piVar8) + (uint32_t)(0), (iVar4 + -(1))), 1);
                                            gelem<int32_t>((uint32_t)(piVar8), 0) = iVar7;
                                            bVar12 = 2;
                                        }
                                    }
                                    while (!((uint8_t)(((bVar12 >> 1) & 1))));
                                    ((void)0 /* instructionSynchronize dropped */);
                                    if (((iVar4 + -(1)) == 0))
                                    {
                                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)))) >> 32);
                                    }
                                }
                            }
                            iVar10 = (iVar10 + -(1));
                        }
                        while ((iVar10 != 0));
                    }
                }
joined_r0x02aabab4:;
                a4 = (a4 + -(1));
                if ((a4 == 0))
                {
                    native_frame_close(_frame, 64);
                    return;
                }
            }
            iVar4 = LD32((uint32_t)((iVar3 + 8)));
            if ((iVar4 == 0))
            {
                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C098C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                iVar4 = LD32((uint32_t)((iVar3 + 8)));
            }
            (__pair = (uint64_t)(((ItemInstance *)gmem(iVar4))->copy((_frame + 32))));
            if ((LD32((_frame + 32)) == 0))
            {
                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C098C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
            }
            iVar3 = LD32((_frame + 32));
            if ((LD32((_frame + 32)) == 0))
            {
                guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C098C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
            }
            iVar4 = ((ItemInstance *)gmem(LD32((_frame + 32))))->getCount();
            ((ItemInstance *)gmem(iVar3))->setCount((int32_t)((iVar10 * iVar4)));
            iVar3 = a3->field_0x0;
            if ((a5 == 0))
            {
                if ((iVar3 == 0))
                {
                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                    iVar3 = a3->field_0x0;
                }
                piVar6 = (uint32_t)(a3->field_0x0);
                iVar4 = gelem<int32_t>((uint32_t)(piVar6), 0);
                if ((iVar3 == 0))
                {
                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                }
                iVar3 = LD32((uint32_t)(a3->field_0x0));
                bVar1 = (LD32((_frame + 36)) == (uint32_t)(0x0));
                ST32((_frame + 40), LD32((_frame + 32)));
                ST32((_frame + 44), LD32((_frame + 36)));
                while (!(bVar1))
                {
                    if (('\x01' != '\0'))
                    {
                        iVar7 = (ST32((uint32_t)(LD32((_frame + 36))) + (uint32_t)(0), (LD32(LD32((_frame + 36))) + 1)), 1);
                        ST32(LD32((_frame + 36)), iVar7);
                        bVar1 = 1;
                    }
                }
                pcVar11 = LD32((uint32_t)((iVar4 + 0x3c)));
                uVar5 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar3 + 0x14))))) >> 32);
                uVar5 = a1->nextInt((int32_t)(uVar5));
                (uint32_t)((__pair = (uint64_t)(guest::indirect((uint32_t)(pcVar11), (uint32_t)(piVar6), (uint32_t)(uVar5), (uint32_t)((_frame + 40))))) >> 32);
                piVar8 = LD32((_frame + 44));
                bVar1 = (LD32((_frame + 44)) == (uint32_t)(0x0));
                piVar6 = LD32((_frame + 36));
                if (!(bVar1))
                {
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar3 = (LD32(LD32((_frame + 44))) + -(1));
                        if (('\x01' != '\0'))
                        {
                            iVar4 = (ST32((uint32_t)(LD32((_frame + 44))) + (uint32_t)(0), iVar3), 1);
                            ST32(LD32((_frame + 44)), iVar4);
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    ((void)0 /* instructionSynchronize dropped */);
                    bVar12 = ((iVar3 == 0) << 1);
                    if ((iVar3 == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 44)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 44)))))) >> 32);
                        piVar9 = (piVar8 + (1) * 4);
                        ((void)0 /* sync dropped */);
                        do
                        {
                            iVar3 = gelem<int32_t>((uint32_t)(piVar9), 0);
                            if (('\x01' != '\0'))
                            {
                                iVar4 = (ST32((uint32_t)(piVar9) + (uint32_t)(0), (iVar3 + -(1))), 1);
                                gelem<int32_t>((uint32_t)(piVar9), 0) = iVar4;
                                bVar12 = 2;
                            }
                        }
                        while (!((uint8_t)(((bVar12 >> 1) & 1))));
                        ((void)0 /* instructionSynchronize dropped */);
                        piVar6 = LD32((_frame + 36));
                        if (((iVar3 + -(1)) == 0))
                        {
                            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar8) + 2u, 0) + 0x1c)), (uint32_t)(piVar8), (uint32_t)(gelem<int32_t>((uint32_t)(piVar8) + 2u, 0)), (uint32_t)(piVar9)))) >> 32);
                            piVar6 = LD32((_frame + 36));
                        }
                    }
                }
            }
            else
            {
                if ((iVar3 == 0))
                {
                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                }
                iVar3 = ((Container *)gmem(a3->field_0x0))->isFull();
                if ((iVar3 != 0))
                {
                    if ((LD32((_frame + 36)) == (uint32_t)(0x0)))
                    {
                        native_frame_close(_frame, 64);
                        return;
                    }
                    ((void)0 /* sync dropped */);
                    bVar1 = 0;
                    do
                    {
                        iVar3 = (LD32(LD32((_frame + 36))) + -(1));
                        if (('\x01' != '\0'))
                        {
                            iVar4 = (ST32((uint32_t)(LD32((_frame + 36))) + (uint32_t)(0), iVar3), 1);
                            ST32(LD32((_frame + 36)), iVar4);
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    ((void)0 /* instructionSynchronize dropped */);
                    bVar12 = ((iVar3 == 0) << 1);
                    if ((iVar3 != 0))
                    {
                        native_frame_close(_frame, 64);
                        return;
                    }
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 36)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 36))), (uint32_t)(LD32((_frame + 36)))))) >> 32);
                    piVar6 = (LD32((_frame + 36)) + (1) * 4);
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar3 = gelem<int32_t>((uint32_t)(piVar6), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar4 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar3 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar6), 0) = iVar4;
                            bVar12 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar12 >> 1) & 1))));
                    ((void)0 /* instructionSynchronize dropped */);
                    if (((iVar3 + -(1)) != 0))
                    {
                        native_frame_close(_frame, 64);
                        return;
                    }
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 36)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 36)))))) >> 32);
                    native_frame_close(_frame, 64);
                    return;
                }
                do
                {
                    if ((a3->field_0x0 == 0))
                    {
                        guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                    }
                    uVar5 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)(a3->field_0x0)) + 0x14)), (uint32_t)(a3->field_0x0)))) >> 32);
                    uVar5 = a1->nextInt((int32_t)(uVar5));
                    if ((a3->field_0x0 == 0))
                    {
                        guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                    }
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32((uint32_t)(a3->field_0x0)) + 0x24)), (uint32_t)((uint32_t)(a3->field_0x0)), (uint32_t)((_frame + 24)), (uint32_t)(uVar5)))) >> 32);
                    if ((LD32((_frame + 24)) == 0))
                    {
                        guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C098C)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                    }
                    uVar13 = ((ItemInstance *)gmem(LD32((_frame + 24))))->isEmpty();
                    piVar6 = LD32((_frame + 28));
                    bVar1 = (LD32((_frame + 28)) == (uint32_t)(0x0));
                    if (!(bVar1))
                    {
                        ((void)0 /* sync dropped */);
                        do
                        {
                            iVar3 = (LD32(LD32((_frame + 28))) + -(1));
                            if (('\x01' != '\0'))
                            {
                                iVar4 = (ST32((uint32_t)(LD32((_frame + 28))) + (uint32_t)(0), iVar3), 1);
                                ST32(LD32((_frame + 28)), iVar4);
                                bVar1 = 1;
                            }
                        }
                        while (!(bVar1));
                        ((void)0 /* instructionSynchronize dropped */);
                        bVar12 = ((iVar3 == 0) << 1);
                        if ((iVar3 == 0))
                        {
                            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 28)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 28))), (uint32_t)((int32_t)(uVar13)), (uint32_t)(LD32((_frame + 28)))))) >> 32);
                            piVar8 = (piVar6 + (1) * 4);
                            ((void)0 /* sync dropped */);
                            do
                            {
                                iVar3 = gelem<int32_t>((uint32_t)(piVar8), 0);
                                if (('\x01' != '\0'))
                                {
                                    iVar4 = (ST32((uint32_t)(piVar8) + (uint32_t)(0), (iVar3 + -(1))), 1);
                                    gelem<int32_t>((uint32_t)(piVar8), 0) = iVar4;
                                    bVar12 = 2;
                                }
                            }
                            while (!((uint8_t)(((bVar12 >> 1) & 1))));
                            ((void)0 /* instructionSynchronize dropped */);
                            if (((iVar3 + -(1)) == 0))
                            {
                                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)))) >> 32);
                            }
                        }
                    }
                }
                while (((int32_t)(((uint64_t)(uVar13) >> 0x20)) != 1));
                if ((a3->field_0x0 == 0))
                {
                    guest::boost_assertion_failed((uint32_t)(GSTR("px____0_102c0984_s_boost__detail__sp_member_access_", 0x102C0A40)), (uint32_t)(GSTR("_________Libraries_Boost_boost_1", 0x102C09F8)), (uint32_t)(0x28d), 0u, 0u);
                }
                piVar6 = (uint32_t)(a3->field_0x0);
                iVar3 = gelem<int32_t>((uint32_t)(piVar6), 0);
                bVar1 = (LD32((_frame + 36)) == (uint32_t)(0x0));
                ST32((_frame + 24), LD32((_frame + 32)));
                ST32((_frame + 28), LD32((_frame + 36)));
                while (!(bVar1))
                {
                    if (('\x01' != '\0'))
                    {
                        iVar4 = (ST32((uint32_t)(LD32((_frame + 36))) + (uint32_t)(0), (LD32(LD32((_frame + 36))) + 1)), 1);
                        ST32(LD32((_frame + 36)), iVar4);
                        bVar1 = 1;
                    }
                }
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar3 + 0x3c)), (uint32_t)(piVar6), (uint32_t)(uVar5), (uint32_t)((_frame + 24))))) >> 32);
                piVar8 = LD32((_frame + 28));
                piVar6 = LD32((_frame + 36));
                if ((LD32((_frame + 28)) != (uint32_t)(0x0)))
                {
                    ((void)0 /* sync dropped */);
                    bVar1 = 0;
                    do
                    {
                        iVar3 = (LD32(LD32((_frame + 28))) + -(1));
                        if (('\x01' != '\0'))
                        {
                            iVar4 = (ST32((uint32_t)(LD32((_frame + 28))) + (uint32_t)(0), iVar3), 1);
                            ST32(LD32((_frame + 28)), iVar4);
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    ((void)0 /* instructionSynchronize dropped */);
                    bVar12 = ((iVar3 == 0) << 1);
                    if ((iVar3 == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 28)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 28))), (uint32_t)(0), (uint32_t)(LD32((_frame + 28)))))) >> 32);
                        piVar6 = (piVar8 + (1) * 4);
                        ((void)0 /* sync dropped */);
                        do
                        {
                            iVar3 = gelem<int32_t>((uint32_t)(piVar6), 0);
                            if (('\x01' != '\0'))
                            {
                                iVar4 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar3 + -(1))), 1);
                                gelem<int32_t>((uint32_t)(piVar6), 0) = iVar4;
                                bVar12 = 2;
                            }
                        }
                        while (!((uint8_t)(((bVar12 >> 1) & 1))));
                        ((void)0 /* instructionSynchronize dropped */);
                        piVar6 = LD32((_frame + 36));
                        if (((iVar3 + -(1)) == 0))
                        {
                            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar8) + 2u, 0) + 0x1c)), (uint32_t)(piVar8)))) >> 32);
                            piVar6 = LD32((_frame + 36));
                        }
                    }
                }
            }
            ST32((_frame + 36), piVar6);
            if ((piVar6 == (uint32_t)(0x0)))
            {
                goto joined_r0x02aabab4;
            }
            bVar1 = 0;
            ((void)0 /* sync dropped */);
            do
            {
                iVar3 = (gelem<int32_t>((uint32_t)(piVar6), 0) + -(1));
                if (('\x01' != '\0'))
                {
                    iVar4 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), iVar3), 1);
                    gelem<int32_t>((uint32_t)(piVar6), 0) = iVar4;
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            bVar12 = ((iVar3 == 0) << 1);
            if ((iVar3 != 0))
            {
                goto joined_r0x02aabab4;
            }
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x14)), (uint32_t)(piVar6)))) >> 32);
            piVar8 = (piVar6 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar3 = gelem<int32_t>((uint32_t)(piVar8), 0);
                if (('\x01' != '\0'))
                {
                    iVar4 = (ST32((uint32_t)(piVar8) + (uint32_t)(0), (iVar3 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar8), 0) = iVar4;
                    bVar12 = 2;
                }
            }
            while (!((uint8_t)(((bVar12 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar3 + -(1)) != 0))
            {
                goto joined_r0x02aabab4;
            }
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6)))) >> 32);
            a4 = (a4 + -(1));
        }
        while ((a4 != 0));
    }
    native_frame_close(_frame, 64);
    return;
    native_frame_close(_frame, 64);
}
#endif

}  /* namespace recon */
