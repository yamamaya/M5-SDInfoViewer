#ifndef __SD_MID_H__
#define __SD_MID_H__

typedef struct {
  const uint8_t id;
  const char * const name;
} DATAPAIR;

static DATAPAIR mid_list[] = {
  { 0x01, "Panasonic" },
  { 0x02, "Toshiba" },
  { 0x03, "SanDisk" },
  { 0x04, "SanDisk" },
  { 0x06, "Ritek" },
  { 0x09, "ATP" },
  { 0x13, "Kingmax" },
  { 0x19, "Dynacard" },
  { 0x1a, "PQI" },
  { 0x1b, "Samsung" },
  { 0x1c, "Transcend" },
  { 0x1d, "ADATA" },
  { 0x27, "Phison" },
  { 0x28, "Barun" },
  { 0x31, "Silicon Power" },
  { 0x41, "Kingston" },
  { 0x51, "STEC" },
  { 0x5d, "SwissBit" },
  { 0x61, "Netkust" },
  { 0x63, "Cactus" },
  { 0x74, "Jiaelec" },
  { 0x76, "Patriot" },
  { 0x82, "Jiang Tay" },
  { 0x83, "Netcom" },
  { 0x84, "Strontium" }
};

#endif
