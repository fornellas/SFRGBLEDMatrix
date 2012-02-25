extern "C" {
  #include <inttypes.h>
  #include <stdlib.h>
  #include <math.h>
}

#include <Arduino.h>
#include <SFRGBLEDMatrix.h>
#include <SPI.h>
#include <fonts.h>

// Display properties
#define DISPLAY_PIXELS 64
#define DISPLAY_BUFFER_SIZE (DISPLAY_PIXELS*BITS_PER_COLOR*3>>3)
#define DISP_LEN byte(8)

// Font

// 2.5 Gamma correction
static unsigned char gamma25[] PROGMEM = {0, 0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 7, 9, 10, 13, 15};

//
// Functions
//

SFRGBLEDMatrix::SFRGBLEDMatrix(const uint8_t pinSS, const uint8_t numDispHoriz, const uint8_t numDispVert) {
  this->dispCount=numDispHoriz*numDispVert;
  this->width=DISP_LEN*numDispHoriz;
  this->height=DISP_LEN*numDispVert;
  this->recAdjStart=int((( ((height>>3)-1) * (width>>3) )<<3));
  this->recAdjIncr=((width>>3)+1)<<3;
  this->useGamma=false;
  this->pinSS=pinSS;
  frameBuffSize=DISPLAY_BUFFER_SIZE*dispCount;
  frameBuff=(byte *)calloc((size_t)(frameBuffSize), sizeof(byte)); // FIXME validate if NULL
  this->pixels=this->width*this->height;
  setupSPI();
  setupPINs();
}

SFRGBLEDMatrix::~SFRGBLEDMatrix() {
  free(frameBuff);
};

void SFRGBLEDMatrix::setupSPI() {
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.setBitOrder(MSBFIRST);
}

void SFRGBLEDMatrix::setupPINs() {
  pinMode(MOSI, OUTPUT);
  pinMode(pinSS, OUTPUT);
}

void SFRGBLEDMatrix::show() {
  digitalWrite(pinSS, LOW);
  for(uint16_t p=0;p<frameBuffSize;p++){
    SPI.transfer(*(frameBuff + p));
    delayMicroseconds(64);
  }
  digitalWrite(pinSS, HIGH);
  delayMicroseconds(297);
};

#define X_MAX (byte)(pgm_read_word_near(coffset+c-CHAR_MIN+1)-(byte)pgm_read_word_near(coffset+c-CHAR_MIN))

void SFRGBLEDMatrix::print(const Color color, const int xOffset, const int yOffset, const uint8_t size, const char c){
  prog_uint16_t *coffset;
  prog_uchar **line;
  
  switch(size){
    case 4:
       coffset=coffset_4p;
       line=line_4p;
       break;
    case 5:
       coffset=coffset_5p;
       line=line_5p;
       break;
    default:
       return;
  };

  if(c<CHAR_MIN||c>CHAR_MAX)
    return;
  for(int y=0;y<size;y++){
    if(y+yOffset>=height)
     continue;
    uint8_t x_max=X_MAX;
    for(int x=0;x<x_max;x++){
      unsigned int bitOffset;
      byte charData;
      byte pixel;
      if(x+yOffset>=width)
        continue;
      bitOffset=(unsigned int)pgm_read_word_near(coffset+c-CHAR_MIN)+x;
      charData=(byte)pgm_read_word_near((byte *)pgm_read_word_near(&line[y])+bitOffset/8);
      pixel=(charData>>(7-bitOffset%8)) & B00000001;
      if(pixel)
        paintPixel(color, x+xOffset, y+yOffset);
    }
  }
}

void SFRGBLEDMatrix::print(const Color color, int x, int y, const uint8_t size, const char *s){
  prog_uint16_t *coffset;
  switch(size){
    case 4:
       coffset=coffset_4p;
       break;
    case 5:
       coffset=coffset_5p;
       break;
    default:
       return;
  }  
  for(uint16_t p=0;s[p]!='\0';p++){
    char c;
    c=s[p];
    print(color, x, y, size, s[p]);
    x+=X_MAX+1;
  }
}

void SFRGBLEDMatrix::paintPixel(Color color, int x, int y) {
  uint16_t startPixel;
  uint16_t startByte;
  // Out of boundaries
  if(x>=width||y>=height||y<0||y<0)
    return;
  // Gamma
  if(useGamma){
    color=RGB_GAMMA(GET_RED(color), GET_GREEN(color), GET_BLUE(color));
  }
  // Adjust coordinates, can be disabled for single row of matrices
  x+=recAdjStart - int(y>>3)*recAdjIncr;
  // print pixel
  startPixel=(dispCount-1-(x>>3))*DISPLAY_PIXELS + ((7-y)<<3) + (x&7);
  startByte=startPixel*12/8;
  // odd pixels
  if(startPixel&0x01) {
    // XXXX RRRR
    frameBuff[startByte]=(frameBuff[startByte]&0xF0)|(color>>8);
    // GGGG BBBB
    frameBuff[startByte+1]=color&0xFF;
  // even pixels
  }else{
    // RRRR GGGG
    frameBuff[startByte]=color>>4;
    // BBBB XXXX
    frameBuff[startByte+1]=(frameBuff[startByte+1]&0x0F)|(color&0x0F)<<4;
  }
}

void SFRGBLEDMatrix::fill(Color color){
  for(int x=0;x<width;x++)
    for(int y=0;y<height;y++)
      paintPixel(color, x, y);
}

void SFRGBLEDMatrix::clear(){
  fill(BLACK);
}

void SFRGBLEDMatrix::gamma(boolean state){
  useGamma=state;
}
