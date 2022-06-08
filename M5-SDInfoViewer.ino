#include <M5Stack.h>
#define LGFX_AUTODETECT
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>
#include <SdFat.h>
#include "SD_DataTable.h"

#define DEBUGMODE

#ifdef  DEBUGMODE
#define DEBUG(x)           _DEBUG(,x)
#define _DEBUG(dummy,x)    dummy##x
#else
#define DEBUG(x)
#endif

static LGFX lcd;
static SdFs sd;

static bool isCardReady = false;
static cid_t cid;
static csd_t csd;
static SdStatus_t sdstat;
static int DisplayPage = 0;

void siftString( char *str ) {
  char *p = str;
  while ( *p != 0 ) {
    if ( *p < ' ' || *p > 0x7e ) {
      *p = '?';
    }
    p ++;
  }
}

void uint64ToString( uint64_t value, char *dest, bool comma = false ) {
  char buff[32];
  buff[31] = 0;
  char *p = buff + 31;
  int count = 0;
  do {
    int d = (int)( value % 10 );
    value /= 10;
    if ( comma ) {
      if ( count > 0 && count % 3 == 0 ) {
        p--;
        *p = ',';
      }
    }
    count ++;
    p--;
    *p = (char)( '0' + d );
  } while ( value != 0 );
  strcpy( dest, p );
}

const char *getValueById( uint8_t id, DATAPAIR *list, size_t list_size ) {
  for ( int i = 0 ; i < list_size ; i ++ ) {
    if ( list[i].id == id ) {
      return list[i].value;
    }
  }
  return NULL;
}

float sdCardMaxDataSpeed( csd_t *csd ) {
  uint8_t unit = csd->v1.tran_speed & 0x07;
  uint8_t time_value = ( csd->v1.tran_speed >> 3 ) & 0x0f;
  uint32_t speed;
  switch ( time_value ) {
    case 0x1: speed = 10; break;
    case 0x2: speed = 12; break;
    case 0x3: speed = 13; break;
    case 0x4: speed = 15; break;
    case 0x5: speed = 20; break;
    case 0x6: speed = 25; break;
    case 0x7: speed = 30; break;
    case 0x8: speed = 35; break;
    case 0x9: speed = 40; break;
    case 0xa: speed = 45; break;
    case 0xb: speed = 50; break;
    case 0xc: speed = 55; break;
    case 0xd: speed = 60; break;
    case 0xe: speed = 70; break;
    case 0xf: speed = 80; break;
    default: speed = 0;
  }
  switch ( unit ) {
    case 0: speed *= 100/10;    break;
    case 1: speed *= 1000/10;   break;
    case 2: speed *= 10000/10;  break;
    case 3: speed *= 100000/10; break;
    default: speed = 0;
  }
  return (float)speed / 1000;
}

#define ROWS  5
#define COLS  2
#define PAGES 2

typedef void(*getValueTextFunc)(char*);

typedef struct {
  const char * const label;
  const getValueTextFunc getValueText;
} DISPLAYITEM;

typedef struct {
  const DISPLAYITEM items[ROWS][COLS];
} DISPLAYPAGE;

// Display page definitions
const DISPLAYPAGE pages[PAGES] = {
  { // Page 1
    {
      { // Row 1
        {
          "Manufacturer ID",
          []( char *dest ) {
            const char *mid_name = getValueById( cid.mid, mid_list, sizeof( mid_list ) / sizeof( mid_list[0] ) );
            if ( mid_name == NULL ) {
              mid_name = "unknown";
            }
            sprintf( dest, "%02X (%s)", cid.mid, mid_name );
          }
        },{
          NULL, NULL
        }
      },
      { // Row 2
        {
          "OEM/Application ID",
          []( char *dest ) {
            char buff[3];
            memcpy( buff, cid.oid, 2 );
            buff[2] = 0;
            siftString( buff );
            uint16_t oid = ( (uint16_t)cid.oid[0] << 8 ) | (uint16_t)cid.oid[1];
            sprintf( dest, "%04X (%s)", oid, buff );
          }
        }, {
          NULL, NULL
        }
      },
      { // Row 3
        {
          "Product name",
          []( char *dest ) {
            memcpy( dest, cid.pnm, 5 );
            dest[5] = 0;
            siftString( dest );
          }
        }, {
          "Revision",
          []( char *dest ) {
            sprintf( dest, "%d.%d", cid.prv_n, cid.prv_m );
          }
        }
      },
      { // Row 4
        {
          "Serial number",
          []( char *dest ) {
            sprintf( dest, "%u", cid.psn );
          }
        }, {
          "Manufacturing date",
          []( char *dest ) {
            int mdt_year = 2000 + 16*cid.mdt_year_high + cid.mdt_year_low;
            sprintf( dest, "%d/%d", mdt_year, cid.mdt_month );
          }
        }
      },
      { // Row 5
        {
          "Capacity",
          []( char *dest ) {
            uint64_t capacity = (uint64_t)sdCardCapacity( &csd ) * 512;
            uint32_t capacity_HF;
            char const *capacity_unit;
            if ( capacity <= 1048574951424 ) {
              capacity_HF = (uint32_t)( capacity / 1048576 );
              capacity_unit = "MB";
            } else {
              capacity_HF = (uint32_t)( capacity / 1073741824 );
              capacity_unit = "GB";
            }
            char buff[32];
            uint64ToString( (uint64_t)capacity_HF, buff, true );
            sprintf( dest, "%s%s", buff, capacity_unit );
          }
        }, {
          "Max bus clock",
          []( char *dest ) {
            float tran_speed = sdCardMaxDataSpeed( &csd );
            if ( tran_speed < 10 ) {
              sprintf( dest, "%.1fMHz", tran_speed );
            } else {
              sprintf( dest, "%dMHz", (int)tran_speed );
            }
          }
        }
      }
    }
  },
  { // Page 2
    {
      { // Row 1
        {
          "Speed class",
          []( char *dest ) {
            const char *str = getValueById( sdstat.speedClass, SpeedClass_list, sizeof( SpeedClass_list ) / sizeof( SpeedClass_list[0] ) );
            if ( str == NULL ) {
              str = "unknown";
            }
            strcpy( dest, str );
          }
        }, {
          "UHS Speed grade",
          []( char *dest ) {
            uint8_t UHSSpeedGrade = sdstat.uhsSpeedAuSize >> 4;
            const char *str = getValueById( UHSSpeedGrade, UHSSpeedClass_list, sizeof( UHSSpeedClass_list ) / sizeof( UHSSpeedClass_list[0] ) );
            if ( str == NULL ) {
              str = "N/A";
            }
            strcpy( dest, str );
          }
        }
      },
      { // Row 2
        {
          "Video speed",
          []( char *dest ) {
            const char *str = getValueById( sdstat.videoSpeed, VideoSpeedClass_list, sizeof( VideoSpeedClass_list ) / sizeof( VideoSpeedClass_list[0] ) );
            if ( str == NULL ) {
              str = "N/A";
            }
            strcpy( dest, str );
          }
        }, {
          "App performance",
          []( char *dest ) {
            uint8_t AppPrefClass = ((uint8_t *)&sdstat)[42] & 0x0f;
            const char *str = getValueById( AppPrefClass, AppPerfClass_list, sizeof( AppPerfClass_list ) / sizeof( AppPerfClass_list[0] ) );
            if ( str != NULL ) {
              strcpy( dest, str );
            } else {
              sprintf( dest, "(%d)", AppPrefClass );
            }
          }
        }
      },
      { // Row 3
        {
          "AU Size",
          []( char *dest ) {
            uint8_t AUSize = sdstat.auSize >> 4;
            strcpy( dest, getValueById( AUSize, AUSize_list, sizeof( AUSize_list ) / sizeof( AUSize_list[0] ) ) );
          }
        }, {
          "UHS AU Size",
          []( char *dest ) {
            uint8_t UHS_AUSize = sdstat.uhsSpeedAuSize & 0x0f;
            strcpy( dest, getValueById( UHS_AUSize, AUSize_list, sizeof( AUSize_list ) / sizeof( AUSize_list[0] ) ) );
          }
        }
      }
    }
  }
};

#define LABEL_FONT &fonts::FreeSans9pt7b
#define LABEL_HEIGHT 21
#define VALUE_FONT &fonts::FreeSans12pt7b
#define VALUE_HEIGHT 26
#define VALUE_INDENT 16
#define SCREEN_WIDTH (lcd.width())
#define SCREEN_HEIGHT (lcd.height())

bool initializeSdCard() {
  if ( !sd.cardBegin( SdSpiConfig( TFCARD_CS_PIN, SHARED_SPI, SD_SCK_MHZ( 16 ) ) ) ) {
    DEBUG( Serial.print( "sd.cardBegin failed : " ) );
    DEBUG( Serial.println( sd.sdErrorCode() ) );
    return false;
  }
  DEBUG( Serial.println( "SD Card initialized" ) );
  return true;
}

bool retrieveCardInformations() {
  if ( !sd.card()->readCID( &cid ) ) {
    DEBUG( Serial.println( "readCID failed" ) );
    return false;
  }
  DEBUG( Serial.println( "readCID succeeded" ) );

  if ( !sd.card()->readCSD( &csd ) ) {
    DEBUG( Serial.println( "readCSD failed" ) );
    return false;
  }
  DEBUG( Serial.println( "readCSD succeeded" ) );
  if ( !sd.card()->readStatus( (uint8_t*)&sdstat ) ) {
    DEBUG( Serial.println( "readStatus failed" ) );
    return false;
  }
  DEBUG( Serial.println( "readStatus succeeded" ) );
  return true;
}

void drawLabel( int col, int y, const char *label, bool full_width ) {
  int width;
  if ( full_width ) {
    width = SCREEN_WIDTH;
  } else {
    width = SCREEN_WIDTH / 2;
  }
  int x;
  if ( col == 0 ) {
    x = 0;
    if ( !full_width ) {
      width -= 4;
    }
  } else {
    x = SCREEN_WIDTH / 2;
  }
  lcd.fillRect( x, y, width, LABEL_HEIGHT-2, NAVY );
  lcd.setCursor( x+1, y, LABEL_FONT );
  lcd.print( label );
}

void setCursor( int col, int y ) {
  lcd.setCursor( ( SCREEN_WIDTH/2 * col ) + VALUE_INDENT, y + LABEL_HEIGHT, VALUE_FONT );
}

void showPage( int page_number ) {
  const DISPLAYPAGE *page = &pages[page_number];

  lcd.fillScreen( BLACK );
  lcd.setTextColor( WHITE );

  int y = 0;
  for ( int row = 0 ; row < ROWS ; row ++ ) {
    for ( int col = 0 ; col < COLS ; col ++ ) {
      if ( page->items[row][col].label == NULL ) {
        continue;
      }
      bool full_width = false;
      if ( col == 0 && page->items[row][1].label == NULL ) {
        full_width = true;
      }
      drawLabel( col, y, page->items[row][col].label, full_width );
      setCursor( col, y );
      char buff[32];
      page->items[row][col].getValueText( buff );
      lcd.print( buff );
      if ( full_width ) {
        break;
      }
    }
    y += LABEL_HEIGHT + VALUE_HEIGHT;
  }
}

void setup(){
  M5.begin( false, false, true, false );
  M5.Power.begin();
  delay( 500 );
  lcd.init();
  lcd.setTextWrap( false );

  lcd.fillScreen( BLACK );
  lcd.setTextColor( WHITE );
  lcd.setCursor( 0, 0 );

  if ( initializeSdCard() ) {
    isCardReady = true;
    retrieveCardInformations();
  } else {
    lcd.setFont( LABEL_FONT );
    lcd.setTextDatum( textdatum_t::middle_center );
    lcd.drawString( "SD Card Information Viewer", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 25 );
    lcd.drawString( "Insert SD Card and reset", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 25 );
  }
}

static bool first_loop = true;
static int current_page = 0;

void loop() {
  M5.update();
  if ( !isCardReady ) {
    return;
  }
  int page = current_page;
  if ( M5.BtnA.wasPressed() ) {
    page = 0;
  } else if ( M5.BtnB.wasPressed() ) {
    page = 1;
  }
  if ( page != current_page || first_loop ) {
    current_page = page;
    showPage( page );
  }
  first_loop = false;
}
