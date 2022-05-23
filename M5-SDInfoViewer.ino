#include <M5Stack.h>
#define LGFX_AUTODETECT
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>
#include <SdFat.h>
#include "SD_MID.h"

//#define DEBUGMODE

#ifdef  DEBUGMODE
#define DEBUG(x)           _DEBUG(,x)
#define _DEBUG(dummy,x)    dummy##x
#else
#define DEBUG(x)
#endif

static LGFX lcd;
static SdFs sd;

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

#define LABEL_FONT &fonts::FreeSans9pt7b
#define LABEL_HEIGHT 21
#define VALUE_FONT &fonts::FreeSans12pt7b
#define VALUE_HEIGHT 26
#define VALUE_INDENT 16
#define SCREEN_WIDTH (lcd.width())
#define SCREEN_HEIGHT (lcd.height())

void drawLabel( int col, int y, const char *label, bool half ) {
  int width;
  if ( half ) {
    width = SCREEN_WIDTH / 2;
  } else {
    width = SCREEN_WIDTH;
  }
  int x;
  if ( col == 0 ) {
    x = 0;
    if ( half ) {
      width -= 4;
    }
  } else {
    x = SCREEN_WIDTH / 2;
  }
  lcd.fillRect( x, y, width, LABEL_HEIGHT-2, NAVY );
  lcd.setCursor( x+1, y, LABEL_FONT );
  lcd.print( label );
}

bool retrieveCardInformations( cid_t &cid, csd_t &csd ) {
  if ( !sd.cardBegin( SdSpiConfig( TFCARD_CS_PIN, SHARED_SPI, SD_SCK_MHZ( 16 ) ) ) ) {
    DEBUG( Serial.print( "sd.cardBegin failed : " ) );
    DEBUG( Serial.println( sd.sdErrorCode() ) );
    return false;
  }
  DEBUG( Serial.println( "SD Card initialized" ) );

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
  DEBUG( Serial.print( "CSD structure: " ) );
  DEBUG( Serial.println( csd.v1.csd_ver ) );
  return true;
}

void showCardInformations( cid_t &cid, csd_t &csd ) {
  lcd.fillScreen( BLACK );
  lcd.setTextColor( WHITE );

  int y = 0;
  char buff[32];

  drawLabel( 0, y, "Manufacturer ID", false );
  y += LABEL_HEIGHT;
  char const *name = "unknown";
  for ( int i = 0 ; i < sizeof( mid_list ) / sizeof( mid_list[0] ) ; i ++ ) {
    if ( cid.mid == mid_list[i].id ) {
      name = mid_list[i].name;
      break;
    }
  }
  lcd.setCursor( VALUE_INDENT, y, VALUE_FONT );
  lcd.printf( "%02X (%s)", cid.mid, name );
  y += VALUE_HEIGHT;

  drawLabel( 0, y, "OEM/Application ID", false );
  y += LABEL_HEIGHT;
  memcpy( buff, cid.oid, 2 );
  buff[2] = 0;
  siftString( buff );
  uint16_t oid = ( (uint16_t)cid.oid[0] << 8 ) | (uint16_t)cid.oid[1];
  lcd.setCursor( VALUE_INDENT, y, VALUE_FONT );
  lcd.printf( "%04X (%s)", oid, buff );
  y += VALUE_HEIGHT;

  drawLabel( 0, y, "Product name", true );
  memcpy( buff, cid.pnm, 5 );
  buff[5] = 0;
  siftString( buff );
  lcd.setCursor( VALUE_INDENT, y+LABEL_HEIGHT, VALUE_FONT );
  lcd.printf( "%s", buff );
  
  drawLabel( 1, y, "Revision", true );
  lcd.setCursor( SCREEN_WIDTH/2+VALUE_INDENT, y+LABEL_HEIGHT, VALUE_FONT );
  lcd.printf( "%d.%d", cid.prv_n, cid.prv_m );
  y += LABEL_HEIGHT + VALUE_HEIGHT;

  drawLabel( 0, y, "Serial number", true );
  lcd.setCursor( VALUE_INDENT, y+LABEL_HEIGHT, VALUE_FONT );
  lcd.printf( "%u", cid.psn );

  drawLabel( 1, y, "Manufacturing date", true );
  int mdt_year = 2000 + 16*cid.mdt_year_high + cid.mdt_year_low;
  lcd.setCursor( SCREEN_WIDTH/2+VALUE_INDENT, y+LABEL_HEIGHT, VALUE_FONT );
  lcd.printf( "%d/%d", mdt_year, cid.mdt_month );
  y += LABEL_HEIGHT + VALUE_HEIGHT;

  drawLabel( 0, y, "Capacity", true );
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
  uint64ToString( (uint64_t)capacity_HF, buff, true );
  lcd.setCursor( VALUE_INDENT, y+LABEL_HEIGHT, VALUE_FONT );
  lcd.printf( "%s%s", buff, capacity_unit );

  drawLabel( 1, y, "Max bus clock", true );
  float tran_speed = sdCardMaxDataSpeed( &csd );
  if ( tran_speed < 10 ) {
    sprintf( buff, "%.1f", tran_speed );
  } else {
    sprintf( buff, "%d", (int)tran_speed );
  }
  lcd.setCursor( SCREEN_WIDTH/2+VALUE_INDENT, y+LABEL_HEIGHT, VALUE_FONT );
  lcd.printf( "%sMHz", buff );
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

  cid_t cid;
  csd_t csd;
  if ( retrieveCardInformations( cid, csd ) ) {
    showCardInformations( cid, csd );
  } else {
    lcd.setFont( LABEL_FONT );
    lcd.setTextDatum( textdatum_t::middle_center );
    lcd.drawString( "SD Card Information Viewer", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 25 );
    lcd.drawString( "Insert SD Card and reset", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 25 );
  }
}

void loop() {
  M5.update();
}
