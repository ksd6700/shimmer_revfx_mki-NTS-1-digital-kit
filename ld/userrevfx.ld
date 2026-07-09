ENTRY(_entry)

MEMORY
{
  SRAM   (rx) : org = 0x20019000, len = 12K
  SDRAM  (rw) : org = 0xC0420000, len = 2432K
}

INCLUDE rules.ld
