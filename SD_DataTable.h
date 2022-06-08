#ifndef __SD_DATATABLE_H__
#define __SD_DATATABLE_H__

typedef struct {
  const uint8_t id;
  const char * const value;
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

static DATAPAIR SpeedClass_list[] = {
  { 0x00, "Class 0" },
  { 0x01, "Class 2" },
  { 0x02, "Class 4" },
  { 0x03, "Class 6" },
  { 0x04, "Class 10" }
};

static DATAPAIR AUSize_list[] = {
  { 0x00, "N/A" },
  { 0x01, "16KB" },
  { 0x02, "32KB" },
  { 0x03, "64KB" },
  { 0x04, "128KB" },
  { 0x05, "256KB" },
  { 0x06, "512KB" },
  { 0x07, "1MB" },
  { 0x08, "2MB" },
  { 0x09, "4MB" },
  { 0x0a, "8MB" },
  { 0x0b, "12MB" },
  { 0x0c, "16MB" },
  { 0x0d, "24MB" },
  { 0x0e, "32MB" },
  { 0x0f, "64MB" }
};

static DATAPAIR UHSSpeedClass_list[] = {
  { 0x00, "<10MB/s" },
  { 0x01, "10MB/s" },
  { 0x03, "30MB/s" },
};

static DATAPAIR VideoSpeedClass_list[] = {
  { 0, "Class 0" },
  { 6, "Class 6" },
  { 10, "Class 10" },
  { 30, "Class 30" },
  { 60, "Class 60" },
  { 90, "Class 90" }
};

static DATAPAIR AppPerfClass_list[] = {
  { 0x00, "N/A" },
  { 0x01, "A1" },
  { 0x02, "A2" }
};

#endif
