#pragma once

#include <generic/bootinfo.h>

namespace PCI
{
    struct HBA_PORT {
        uint32_t clb;
        uint32_t clbu;
        uint32_t fb;
        uint32_t fbu;
        uint32_t is;
        uint32_t ie;
        uint32_t cmd;
        uint32_t rsv0;
        uint32_t tfd;
        uint32_t sig;
        uint32_t ssts;
        uint32_t sctl;
        uint32_t serr;
        uint32_t sact;
        uint32_t ci;
        uint32_t sntf;
        uint32_t fbs;
        uint32_t rsv1[11];
        uint32_t vendor[4];
    };

    struct HBA_MEM {
        uint32_t cap;
        uint32_t ghc;
        uint32_t is;
        uint32_t pi;
        uint32_t vs;
        uint32_t ccc_ctl;
        uint32_t ccc_pts;
        uint32_t em_loc;
        uint32_t em_ctl;
        uint32_t cap2;
        uint32_t bohc;
        uint8_t  rsv[0xA0 - 0x2C];
        uint8_t  vendor[0x100 - 0xA0];
        HBA_PORT ports[32];
    };

    struct HBA_CMD_HEADER
    {
        // DW0
        uint8_t  cfl:5;		// Command FIS length in DWORDS, 2 ~ 16
        uint8_t  a:1;		// ATAPI
        uint8_t  w:1;		// Write, 1: H2D, 0: D2H
        uint8_t  p:1;		// Prefetchable

        uint8_t  r:1;		// Reset
        uint8_t  b:1;		// BIST
        uint8_t  c:1;		// Clear busy upon R_OK
        uint8_t  rsv0:1;		// Reserved
        uint8_t  pmp:4;		// Port multiplier port

        uint16_t prdtl;		// Physical region descriptor table length in entries

        // DW1
        volatile uint32_t prdbc;		// Physical region descriptor byte count transferred

        // DW2, 3
        uint32_t ctba;		// Command table descriptor base address
        uint32_t ctbau;		// Command table descriptor base address upper 32 bits

        // DW4 - 7
        uint32_t rsv1[4];	// Reserved
    };

    struct HBA_PRDT_ENTRY
    {
        uint32_t dba;		// Data base address
        uint32_t dbau;		// Data base address upper 32 bits
        uint32_t rsv0;		// Reserved

        // DW3
        uint32_t dbc:22;		// Byte count, 4M max
        uint32_t rsv1:9;		// Reserved
        uint32_t i:1;		// Interrupt on completion
    };

    struct HBA_CMD_TBL
    {
        // 0x00
        uint8_t  cfis[64];	// Command FIS

        // 0x40
        uint8_t  acmd[16];	// ATAPI command, 12 or 16 bytes

        // 0x50
        uint8_t  rsv[48];	// Reserved

        // 0x80
        HBA_PRDT_ENTRY	prdt_entry[1];	// Physical region descriptor table entries, 0 ~ 65535
    };

    class AHCI
    {
    public:
        AHCI();
    private:
        void ProbePorts();
    private:
        HBA_MEM* abar;
    };

}