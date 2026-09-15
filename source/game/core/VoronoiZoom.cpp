#include "game/V/VoronoiZoom.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(long long, boost::shared_ptr<Layer>) [0x02AA4E38] - boundary stub */
VoronoiZoom::VoronoiZoom(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4)
{
    guest::stub(0x02AA4E38u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, (uint32_t)a4);
}

/* {dtor}() [0x02AD1324] - LOWERED-DRAFT; not reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
VoronoiZoom::~VoronoiZoom()
{
    guest::stub(0x02AD1324u, gaddr(this), 0u);
}
#else
VoronoiZoom::~VoronoiZoom()
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

/* getArea(int, int, int, int) [0x02AA4FA4] - boundary stub; body not reconstructed */
uint64_t VoronoiZoom::getArea(int32_t a1, int32_t a2, int32_t a3, int32_t a4)
{
    return (uint64_t)guest::stub(0x02AA4FA4u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, (uint32_t)a4);
}

}  /* namespace recon */
