#include "game/W/WitherTargetHighestDamage.hpp"
#include "game/G/Goal.hpp"
#include "game/T/TargetGoal.hpp"
#include "_thirdparty/boost__shared_ptr_LivingEntity_.hpp"
#include "_thirdparty/std___Tree_std___Tmap_traits_Q2_5boost33shared_ptr__tm__15_12ServerPlayeriQ2_3std57less__tm__45_Q2_5boost33shared_ptr__t_2.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(WitherBoss*) [0x02AB4078] - reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WitherTargetHighestDamage::WitherTargetHighestDamage(gptr<WitherBoss> a1)
{
    guest::stub(0x02AB4078u, gaddr(this), (uint32_t)a1);
}
#else
WitherTargetHighestDamage::WitherTargetHighestDamage(gptr<WitherBoss> a1) : TargetGoal((gptr<PathfinderMob>)(a1), 0, 0)
{
    this->Goal::setRequiredControlFlags(1);
}
#endif

/* {dtor}() [0x02AD1E80] - LOWERED-DRAFT; not reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
WitherTargetHighestDamage::~WitherTargetHighestDamage()
{
    guest::stub(0x02AD1E80u, gaddr(this), 0u);
}
#else
WitherTargetHighestDamage::~WitherTargetHighestDamage()
{
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t piVar5 = 0;
    if (((uint32_t)((gaddr(this) + 0x20)) != (uint32_t)(0x0)))
    {
        iVar3 = this->field_0x20;
        bVar1 = (iVar3 == 0);
        if (!(bVar1))
        {
            piVar5 = (uint32_t)((iVar3 + 4));
            ((void)0 /* sync dropped */);
            do
            {
                iVar4 = gelem<int32_t>((uint32_t)(piVar5), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar4 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar4 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32((uint32_t)((iVar3 + 8))) + 0x1c)), (uint32_t)(iVar3)) >> 32);
            }
        }
    }
}
#endif

/* getHighestDamageTarget() [0x02AB870C] - LOWERED-DRAFT; not reconstructed (12 open edges, 90 raw IR ops, 30 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t WitherTargetHighestDamage::getHighestDamageTarget()
{
    return (uint64_t)guest::stub(0x02AB870Cu, gaddr(this));
}
#else
uint64_t WitherTargetHighestDamage::getHighestDamageTarget(uint32_t a1)
{
    uint32_t _frame = native_frame_open(32);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint8_t bVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t puVar5 = 0;
    uint32_t piVar6 = 0;
    uint32_t piVar7 = 0;
    uint32_t uVar8 = 0;
    int32_t iVar9 = 0;
    int32_t iVar10 = 0;
    int32_t iVar11 = 0;
    uint32_t piVar12 = 0;
    uint8_t bVar13 = 0;
    uint64_t __pair = 0;
    iVar11 = 0;
    ST32((_frame + 4), (uint32_t)(0x0));
    ST32(_frame, 0);
    iVar4 = guest::WitherBoss_getPlayerParty((uint32_t)(this->field_0x8));
    ST32((_frame + 24) + (0) * 4, LD32(LD32((uint32_t)((iVar4 + 4)))));
    if (((uint32_t)(LD32((_frame + 24) + (0) * 4)) != LD32((uint32_t)((iVar4 + 4)))))
    {
        do
        {
            iVar10 = LD32((uint32_t)((LD32((_frame + 24) + (0) * 4) + 0x14)));
            uVar8 = LD32((uint32_t)((LD32((_frame + 24) + (0) * 4) + 0xc)));
            piVar12 = LD32((uint32_t)((LD32((_frame + 24) + (0) * 4) + 0x10)));
            bVar1 = (piVar12 == (uint32_t)(0x0));
            while (!(bVar1))
            {
                if (('\x01' != '\0'))
                {
                    iVar9 = (ST32((uint32_t)(piVar12) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar12), 0) + 1)), 1);
                    gelem<int32_t>((uint32_t)(piVar12), 0) = iVar9;
                    bVar1 = 1;
                }
            }
            bVar1 = 0;
            bVar3 = 0;
            if ((iVar11 < iVar10))
            {
                iVar9 = this->field_0x4;
                bVar1 = (piVar12 == (uint32_t)(0x0));
                while (!(bVar1))
                {
                    if (('\x01' != '\0'))
                    {
                        iVar2 = (ST32((uint32_t)(piVar12) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar12), 0) + 1)), 1);
                        gelem<int32_t>((uint32_t)(piVar12), 0) = iVar2;
                        bVar1 = 1;
                    }
                }
                bVar1 = 1;
                ST32((_frame + 8), uVar8);
                ST32((_frame + 12), piVar12);
                iVar9 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar9 + 0x6c)), (uint32_t)(gaddr(this)), (uint32_t)((_frame + 8)), (uint32_t)(0)))) >> 32);
                if ((iVar9 != 0))
                {
                    bVar3 = 1;
                }
            }
            piVar6 = LD32((_frame + 12));
            if ((bVar1 && (bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0)), !(bVar1))))
            {
                ((void)0 /* sync dropped */);
                do
                {
                    iVar9 = (LD32(LD32((_frame + 12))) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar2 = (ST32((uint32_t)(LD32((_frame + 12))) + (uint32_t)(0), iVar9), 1);
                        ST32(LD32((_frame + 12)), iVar2);
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                ((void)0 /* instructionSynchronize dropped */);
                bVar13 = ((iVar9 == 0) << 1);
                if ((iVar9 == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 12)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 12)))))) >> 32);
                    piVar7 = (piVar6 + (1) * 4);
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar9 = gelem<int32_t>((uint32_t)(piVar7), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar2 = (ST32((uint32_t)(piVar7) + (uint32_t)(0), (iVar9 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar7), 0) = iVar2;
                            bVar13 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar13 >> 1) & 1))));
                    ((void)0 /* instructionSynchronize dropped */);
                    if (((iVar9 + -(1)) == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar6) + 2u, 0) + 0x1c)), (uint32_t)(piVar6), (uint32_t)(piVar7)))) >> 32);
                    }
                }
            }
            piVar6 = LD32((_frame + 4));
            if (bVar3)
            {
                bVar1 = (piVar12 == (uint32_t)(0x0));
                while (!(bVar1))
                {
                    if (('\x01' != '\0'))
                    {
                        iVar11 = (ST32((uint32_t)(piVar12) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar12), 0) + 1)), 1);
                        gelem<int32_t>((uint32_t)(piVar12), 0) = iVar11;
                        bVar1 = 1;
                    }
                }
                ST32((_frame + 16), LD32(_frame));
                ST32((_frame + 20), LD32((_frame + 4)));
                bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
                iVar11 = iVar10;
                ST32(_frame, uVar8);
                piVar6 = piVar12;
                if (!(bVar1))
                {
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar10 = (LD32(LD32((_frame + 4))) + -(1));
                        if (('\x01' != '\0'))
                        {
                            iVar9 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar10), 1);
                            ST32(LD32((_frame + 4)), iVar9);
                            bVar1 = 1;
                        }
                    }
                    while (!(bVar1));
                    ((void)0 /* instructionSynchronize dropped */);
                    bVar13 = ((iVar10 == 0) << 1);
                    if ((iVar10 == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                        piVar7 = (LD32((_frame + 4)) + (1) * 4);
                        ((void)0 /* sync dropped */);
                        do
                        {
                            iVar10 = gelem<int32_t>((uint32_t)(piVar7), 0);
                            if (('\x01' != '\0'))
                            {
                                iVar9 = (ST32((uint32_t)(piVar7) + (uint32_t)(0), (iVar10 + -(1))), 1);
                                gelem<int32_t>((uint32_t)(piVar7), 0) = iVar9;
                                bVar13 = 2;
                            }
                        }
                        while (!((uint8_t)(((bVar13 >> 1) & 1))));
                        ((void)0 /* instructionSynchronize dropped */);
                        if (((iVar10 + -(1)) == 0))
                        {
                            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                        }
                    }
                }
            }
            ST32((_frame + 4), piVar6);
            bVar1 = (piVar12 == (uint32_t)(0x0));
            if (!(bVar1))
            {
                ((void)0 /* sync dropped */);
                do
                {
                    iVar10 = (gelem<int32_t>((uint32_t)(piVar12), 0) + -(1));
                    if (('\x01' != '\0'))
                    {
                        iVar9 = (ST32((uint32_t)(piVar12) + (uint32_t)(0), iVar10), 1);
                        gelem<int32_t>((uint32_t)(piVar12), 0) = iVar9;
                        bVar1 = 1;
                    }
                }
                while (!(bVar1));
                ((void)0 /* instructionSynchronize dropped */);
                bVar13 = ((iVar10 == 0) << 1);
                if ((iVar10 == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar12) + 2u, 0) + 0x14)), (uint32_t)(piVar12)))) >> 32);
                    piVar6 = (piVar12 + (1) * 4);
                    ((void)0 /* sync dropped */);
                    do
                    {
                        iVar10 = gelem<int32_t>((uint32_t)(piVar6), 0);
                        if (('\x01' != '\0'))
                        {
                            iVar9 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar10 + -(1))), 1);
                            gelem<int32_t>((uint32_t)(piVar6), 0) = iVar9;
                            bVar13 = 2;
                        }
                    }
                    while (!((uint8_t)(((bVar13 >> 1) & 1))));
                    ((void)0 /* instructionSynchronize dropped */);
                    if (((iVar10 + -(1)) == 0))
                    {
                        (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar12) + 2u, 0) + 0x1c)), (uint32_t)(piVar12), (uint32_t)(((uint32_t)__pair)), (uint32_t)(piVar6), (uint32_t)(piVar6)))) >> 32);
                    }
                }
            }
            ((std__Tree_std__Tmap_traits_Q2_5boost33shared_ptr__tm__15_12ServerPlayeriQ2_3std57less__tm__45_Q2_5boost33shared_ptr__tm__15_12ServerPlayerQ2_3std86allocator__tm__69_Q2_3std59pair__tm__47_CQ2_5boost33shared_ptr__tm__15_12ServerPlayeriXCbL_1_0__const_iterator *)gmem((_frame + 24)))->_Inc();
        }
        while ((LD32((_frame + 24) + (0) * 4) != LD32((uint32_t)((iVar4 + 4)))));
    }
    if (((a1 != (uint32_t)(0x0)) || (a1 = (uint32_t)(guest::guest_new((uint32_t)(8))), (a1 != (uint32_t)(0x0)))))
    {
        puVar5 = (a1 + (1) * 4);
        gelem<uint32_t>((uint32_t)(a1), 0) = LD32(_frame);
        if (((puVar5 != (uint32_t)(0x0)) || (puVar5 = (uint32_t)(guest::guest_new((uint32_t)(4))), (puVar5 != (uint32_t)(0x0)))))
        {
            gelem<uint32_t>((uint32_t)(puVar5), 0) = LD32((_frame + 4));
            if ((LD32((_frame + 4)) == (uint32_t)(0x0)))
            {
                { uint64_t t1 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
                native_frame_close(_frame, 32);
                return t1; }
            }
            bVar1 = 0;
            do
            {
                if (('\x01' != '\0'))
                {
                    iVar11 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
                    ST32(LD32((_frame + 4)), iVar11);
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
        }
    }
    bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar11 = (LD32(LD32((_frame + 4))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar4 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), iVar11), 1);
                ST32(LD32((_frame + 4)), iVar4);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar13 = ((iVar11 == 0) << 1);
        if ((iVar11 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
            piVar12 = (LD32((_frame + 4)) + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar11 = gelem<int32_t>((uint32_t)(piVar12), 0);
                if (('\x01' != '\0'))
                {
                    iVar4 = (ST32((uint32_t)(piVar12) + (uint32_t)(0), (iVar11 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar12), 0) = iVar4;
                    bVar13 = 2;
                }
            }
            while (!((uint8_t)(((bVar13 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar11 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
            }
        }
    }
    { uint64_t t2 = ((uint64_t)(uint32_t)(gaddr(this)) << 32);
    native_frame_close(_frame, 32);
    return t2; }
    native_frame_close(_frame, 32);
}
#endif

/* canUse() [0x02AB8B1C] - LOWERED-DRAFT; not reconstructed (8 open edges, 61 raw IR ops, 17 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t WitherTargetHighestDamage::canUse()
{
    return (uint32_t)(guest::stub(0x02AB8B1Cu, gaddr(this)) >> 32);
}
#else
uint32_t WitherTargetHighestDamage::canUse()
{
    uint32_t _frame = native_frame_open(16);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint8_t bVar3 = 0;
    uint32_t piVar4 = 0;
    uint32_t piVar5 = 0;
    int32_t iVar6 = 0;
    uint32_t piVar7 = 0;
    uint8_t bVar8 = 0;
    uint64_t __pair = 0;
    (__pair = (uint64_t)(this->getHighestDamageTarget(_frame)));
    bVar1 = 0;
    bVar3 = 0;
    if ((LD32(_frame) != 0))
    {
        iVar6 = this->field_0x4;
        ST32((_frame + 8), LD32(_frame));
        bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
        ST32((_frame + 12), LD32((_frame + 4)));
        while (!(bVar1))
        {
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 4))) + (uint32_t)(0), (LD32(LD32((_frame + 4))) + 1)), 1);
                ST32(LD32((_frame + 4)), iVar2);
                bVar1 = 1;
            }
        }
        bVar1 = 1;
        iVar6 = (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((iVar6 + 0x6c)), (uint32_t)(gaddr(this)), (uint32_t)((_frame + 8)), (uint32_t)(0)))) >> 32);
        if ((iVar6 != 0))
        {
            bVar3 = 1;
        }
    }
    piVar7 = LD32((_frame + 12));
    if ((bVar1 && (bVar1 = (LD32((_frame + 12)) == (uint32_t)(0x0)), !(bVar1))))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar6 = (LD32(LD32((_frame + 12))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 12))) + (uint32_t)(0), iVar6), 1);
                ST32(LD32((_frame + 12)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar8 = ((iVar6 == 0) << 1);
        if ((iVar6 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 12)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 12)))))) >> 32);
            piVar4 = (piVar7 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar6 = gelem<int32_t>((uint32_t)(piVar4), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar6 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar4), 0) = iVar2;
                    bVar8 = 2;
                }
            }
            while (!((uint8_t)(((bVar8 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar6 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar7) + 2u, 0) + 0x1c)), (uint32_t)(piVar7), (uint32_t)(((uint32_t)__pair)), (uint32_t)(piVar4), (uint32_t)(piVar4)))) >> 32);
            }
        }
    }
    piVar7 = LD32((_frame + 4));
    if (!(bVar3))
    {
        bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            ((void)0 /* sync dropped */);
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
            ((void)0 /* instructionSynchronize dropped */);
            bVar8 = ((iVar6 == 0) << 1);
            if ((iVar6 == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                piVar7 = (LD32((_frame + 4)) + (1) * 4);
                ((void)0 /* sync dropped */);
                do
                {
                    iVar6 = gelem<int32_t>((uint32_t)(piVar7), 0);
                    if (('\x01' != '\0'))
                    {
                        iVar2 = (ST32((uint32_t)(piVar7) + (uint32_t)(0), (iVar6 + -(1))), 1);
                        gelem<int32_t>((uint32_t)(piVar7), 0) = iVar2;
                        bVar8 = 2;
                    }
                }
                while (!((uint8_t)(((bVar8 >> 1) & 1))));
                ((void)0 /* instructionSynchronize dropped */);
                if (((iVar6 + -(1)) == 0))
                {
                    (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((LD32(LD32((_frame + 4)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 4)))))) >> 32);
                }
            }
        }
        { int32_t t1 = 0;
        native_frame_close(_frame, 16);
        return t1; }
    }
    piVar4 = LD32((uint32_t)((gaddr(this) + 0x20)));
    this->field_0x1c = LD32(_frame);
    if ((LD32((_frame + 4)) != piVar4))
    {
        bVar1 = (LD32((_frame + 4)) == (uint32_t)(0x0));
        if (!(bVar1))
        {
            piVar4 = (LD32((_frame + 4)) + (1) * 4);
            do
            {
                if (('\x01' != '\0'))
                {
                    iVar6 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (gelem<int32_t>((uint32_t)(piVar4), 0) + 1)), 1);
                    gelem<int32_t>((uint32_t)(piVar4), 0) = iVar6;
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            piVar4 = LD32((uint32_t)((gaddr(this) + 0x20)));
        }
        bVar1 = (piVar4 == (uint32_t)(0x0));
        if (!(bVar1))
        {
            piVar5 = (piVar4 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar6 = gelem<int32_t>((uint32_t)(piVar5), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar5) + (uint32_t)(0), (iVar6 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar5), 0) = iVar2;
                    bVar1 = 1;
                }
            }
            while (!(bVar1));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar6 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar4) + 2u, 0) + 0x1c)), (uint32_t)(piVar4)))) >> 32);
            }
        }
        ST32((uint32_t)((gaddr(this) + 0x20)), piVar7);
        piVar7 = LD32((_frame + 4));
    }
    bVar1 = (piVar7 == (uint32_t)(0x0));
    if (!(bVar1))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar6 = (gelem<int32_t>((uint32_t)(piVar7), 0) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(piVar7) + (uint32_t)(0), iVar6), 1);
                gelem<int32_t>((uint32_t)(piVar7), 0) = iVar2;
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar8 = ((iVar6 == 0) << 1);
        if ((iVar6 == 0))
        {
            (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar7) + 2u, 0) + 0x14)), (uint32_t)(piVar7)))) >> 32);
            piVar4 = (piVar7 + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar6 = gelem<int32_t>((uint32_t)(piVar4), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar4) + (uint32_t)(0), (iVar6 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar4), 0) = iVar2;
                    bVar8 = 2;
                }
            }
            while (!((uint8_t)(((bVar8 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar6 + -(1)) == 0))
            {
                (uint32_t)((__pair = (uint64_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar7) + 2u, 0) + 0x1c)), (uint32_t)(piVar7)))) >> 32);
            }
        }
    }
    { int32_t t2 = 1;
    native_frame_close(_frame, 16);
    return t2; }
    native_frame_close(_frame, 16);
}
#endif

/* canContinueToUse() [0x02AB8E1C] - LOWERED-DRAFT; not reconstructed (6 open edges, 41 raw IR ops, 13 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint8_t WitherTargetHighestDamage::canContinueToUse()
{
    return (uint8_t)(guest::stub(0x02AB8E1Cu, gaddr(this)) >> 32);
}
#else
uint8_t WitherTargetHighestDamage::canContinueToUse()
{
    uint32_t _frame = native_frame_open(24);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    uint8_t bVar3 = 0;
    uint8_t uVar4 = 0;
    int32_t iVar5 = 0;
    uint32_t piVar6 = 0;
    uint8_t bVar7 = 0;
    bVar1 = 0;
    bVar3 = 0;
    uVar4 = 0;
    iVar5 = guest::TargetGoal_canContinueToUse(gaddr(this));
    if ((iVar5 != 0))
    {
        this->getHighestDamageTarget((_frame + 12));
        bVar3 = 1;
        ST8(_frame + (0), (uint8_t)(gaddr(this)));
        ST32((_frame + 4), 0);
        guest::boost_detail_shared_count_ct_2((uint32_t)((_frame + 8)), (uint32_t)(gaddr(&this->field_0x20)), (uint32_t)(_frame));
        if ((LD32((_frame + 8)) != (uint32_t)(0x0)))
        {
            ST32((_frame + 4), this->field_0x1c);
        }
        bVar1 = 1;
        if ((LD32((_frame + 12)) == LD32((_frame + 4))))
        {
            bVar1 = 1;
            uVar4 = 1;
        }
    }
    if ((bVar1 && (bVar1 = (LD32((_frame + 8)) == (uint32_t)(0x0)), !(bVar1))))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar5 = (LD32(LD32((_frame + 8))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 8))) + (uint32_t)(0), iVar5), 1);
                ST32(LD32((_frame + 8)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar7 = ((iVar5 == 0) << 1);
        if ((iVar5 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 8)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 8)))) >> 32);
            piVar6 = (LD32((_frame + 8)) + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar5 = gelem<int32_t>((uint32_t)(piVar6), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar5 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar6), 0) = iVar2;
                    bVar7 = 2;
                }
            }
            while (!((uint8_t)(((bVar7 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar5 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 8)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 8)))) >> 32);
            }
        }
    }
    if ((bVar3 && (bVar1 = (LD32((_frame + 16)) == (uint32_t)(0x0)), !(bVar1))))
    {
        ((void)0 /* sync dropped */);
        do
        {
            iVar5 = (LD32(LD32((_frame + 16))) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 16))) + (uint32_t)(0), iVar5), 1);
                ST32(LD32((_frame + 16)), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        ((void)0 /* instructionSynchronize dropped */);
        bVar7 = ((iVar5 == 0) << 1);
        if ((iVar5 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 16)) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 16)))) >> 32);
            piVar6 = (LD32((_frame + 16)) + (1) * 4);
            ((void)0 /* sync dropped */);
            do
            {
                iVar5 = gelem<int32_t>((uint32_t)(piVar6), 0);
                if (('\x01' != '\0'))
                {
                    iVar2 = (ST32((uint32_t)(piVar6) + (uint32_t)(0), (iVar5 + -(1))), 1);
                    gelem<int32_t>((uint32_t)(piVar6), 0) = iVar2;
                    bVar7 = 2;
                }
            }
            while (!((uint8_t)(((bVar7 >> 1) & 1))));
            ((void)0 /* instructionSynchronize dropped */);
            if (((iVar5 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 16)) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 16)))) >> 32);
            }
        }
    }
    { uint8_t t1 = uVar4;
    native_frame_close(_frame, 24);
    return t1; }
    native_frame_close(_frame, 24);
}
#endif

/* start() [0x02AB9020] - LOWERED-DRAFT; not reconstructed (4 open edges, 23 raw IR ops, 16 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void WitherTargetHighestDamage::start()
{
    guest::stub(0x02AB9020u, gaddr(this));
}
#else
void WitherTargetHighestDamage::start()
{
    uint32_t _frame = native_frame_open(16);
    uint8_t bVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    uint32_t piVar4 = 0;
    uint8_t bVar5 = 0;
    iVar3 = LD32((uint32_t)((this->field_0x8 + 0x348)));
    ST32((_frame + 4), 0);
    guest::boost_detail_shared_count_ct_2((uint32_t)((_frame + 8)), (uint32_t)(gaddr(&this->field_0x20)), (uint32_t)(_frame));
    if ((LD32((_frame + 8) + (0) * 4) != (uint32_t)(0x0)))
    {
        ST32((_frame + 4), this->field_0x1c);
    }
    (uint32_t)(guest::indirect(LD32((iVar3 + 0xc5c)), (uint32_t)(this->field_0x8), (uint32_t)((_frame + 4))) >> 32);
    bVar1 = (LD32((_frame + 8) + (0) * 4) == (uint32_t)(0x0));
    if (!(bVar1))
    {
        do
        {
            iVar3 = (LD32(LD32((_frame + 8) + (0) * 4)) + -(1));
            if (('\x01' != '\0'))
            {
                iVar2 = (ST32((uint32_t)(LD32((_frame + 8) + (0) * 4)) + (uint32_t)(0), iVar3), 1);
                ST32(LD32((_frame + 8) + (0) * 4), iVar2);
                bVar1 = 1;
            }
        }
        while (!(bVar1));
        bVar5 = ((iVar3 == 0) << 1);
        if ((iVar3 == 0))
        {
            (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 8) + (0) * 4) + (2) * 4) + 0x14)), (uint32_t)(LD32((_frame + 8) + (0) * 4))) >> 32);
            piVar4 = (LD32((_frame + 8) + (0) * 4) + (1) * 4);
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
            if (((iVar3 + -(1)) == 0))
            {
                (uint32_t)(guest::indirect(LD32((LD32(LD32((_frame + 8) + (0) * 4) + (2) * 4) + 0x1c)), (uint32_t)(LD32((_frame + 8) + (0) * 4))) >> 32);
            }
        }
    }
    native_frame_close(_frame, 16);
    return;
    native_frame_close(_frame, 16);
}
#endif

/* canAttack(boost::shared_ptr<LivingEntity>, bool) [0x02AB9130] - boundary stub; body not reconstructed */
uint32_t WitherTargetHighestDamage::canAttack(gptr<sp_LivingEntity> a1, bool a2)
{
    return (uint32_t)(guest::stub(0x02AB9130u, gaddr(this), (uint32_t)a1, (uint32_t)a2) >> 32);
}

}  /* namespace recon */
