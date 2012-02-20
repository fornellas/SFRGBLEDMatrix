extern "C" {
  #include <inttypes.h>
  #include <stdlib.h>
  #include <math.h>
}

#include <Arduino.h>
#include <SFRGBLEDMatrix.h>
#include <SPI.h>

// Display properties
#define DISPLAY_PIXELS 64
#define DISPLAY_BUFFER_SIZE (DISPLAY_PIXELS*BITS_PER_COLOR*3>>3)
#define DISP_LEN byte(8)

// Font
#define CHAR_MIN 32
#define CHAR_MAX 126
#define X_MAX_4P (byte)(pgm_read_word_near(coffset_4p+c-CHAR_MIN+1)-(byte)pgm_read_word_near(coffset_4p+c-CHAR_MIN))
prog_uint16_t coffset_4p[] PROGMEM={
  0, 1, 2, 5, 10, 13, 17, 21, 22, 24, 26, 31, 34, 36, 38, 39, 43, 46, 49, 52, 55, 58, 61, 64, 67, 70, 73, 74, 76, 78, 80, 82, 85, 88, 91, 94, 97, 100, 103, 106, 109, 112, 115, 118, 122, 125, 130, 134, 138, 141, 145, 148, 151, 154, 158, 162, 167, 170, 173, 177, 179, 183, 185, 188, 191, 193, 196, 199, 202, 205, 208, 210, 212, 215, 216, 219, 222, 224, 229, 232, 235, 238, 241, 243, 246, 249, 252, 255, 260, 263, 266, 269, 272, 273, 276, 280};
prog_uchar line0_4p[] PROGMEM={
  0x6a, 0x9c, 0xb5, 0xaa, 0x0, 0x29, 0x6c, 0x7b, 0xff, 0x90, 0x33, 0x59, 0xef, 0xdd, 0xe6, 0x64, 0x65, 0xb3, 0x67, 0xe6, 0x63, 0x6f, 0xf1, 0xa1, 0x8, 0x8, 0x79, 0x12, 0x0, 0x19, 0x1, 0x0, 0x1, 0x7b, 0xe0};
prog_uchar line1_4p[] PROGMEM={
  0x6f, 0xf1, 0x26, 0x5c, 0x80, 0x57, 0x17, 0x64, 0x3e, 0xc7, 0x8d, 0xbe, 0x5d, 0x25, 0x46, 0xa6, 0xf6, 0x6c, 0xd8, 0xa6, 0x6a, 0xa9, 0x48, 0xd0, 0xbc, 0xdb, 0xbc, 0x36, 0xd6, 0x56, 0xef, 0xdb, 0x1b, 0xce, 0xb5};
prog_uchar line2_4p[] PROGMEM={
  0x7, 0xda, 0x4a, 0x5d, 0xdc, 0x95, 0x43, 0xcf, 0x57, 0x98, 0x44, 0xf6, 0x59, 0xaf, 0x57, 0xa5, 0x6e, 0x75, 0xe2, 0xa5, 0x77, 0x52, 0x44, 0x80, 0x5b, 0x2e, 0xdb, 0xba, 0xad, 0xbb, 0xc9, 0x5b, 0x54, 0x52, 0xaa};
prog_uchar line3_4p[] PROGMEM={
  0x42, 0xb4, 0xb1, 0xaa, 0xa3, 0xb, 0xfc, 0x77, 0x9c, 0xe7, 0x93, 0xb9, 0xef, 0x1d, 0xea, 0x7c, 0x65, 0xa3, 0xdc, 0x98, 0xa3, 0x57, 0xe3, 0x8e, 0x3c, 0xdb, 0xbb, 0x57, 0xad, 0x51, 0xd9, 0x34, 0xab, 0xbb, 0xe0};
prog_uchar *line_4p[] PROGMEM={
  line0_4p, line1_4p, line2_4p, line3_4p};

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

void SFRGBLEDMatrix::print(const Color color, const int xOffset, const int yOffset, const uint8_t size, const char c){
  switch(size){
    case 4:
      if(c<CHAR_MIN||c>CHAR_MAX)
        return;
      for(int y=0;y<4;y++){
        if(y+yOffset>=height)
          continue;
        uint8_t x_max=X_MAX_4P;
        for(int x=0;x<x_max;x++){
          unsigned int bitOffset;
          byte charData;
          byte pixel;
          if(x+yOffset>=width)
            continue;
          bitOffset=(unsigned int)pgm_read_word_near(coffset_4p+c-CHAR_MIN)+x;
          charData=(byte)pgm_read_word_near((byte *)pgm_read_word_near(&line_4p[y])+bitOffset/8);
          pixel=(charData>>(7-bitOffset%8)) & B00000001;
          if(pixel)
            paintPixel(color, x+xOffset, y+yOffset);
        }
      }
      break;
  }
}

void SFRGBLEDMatrix::print(const Color color, int x, int y, const uint8_t size, const char *s){
  for(uint16_t p=0;s[p]!='\0';p++){
    char c;
    c=s[p];
    print(color, x, y, size, s[p]);
    x+=X_MAX_4P+1;
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
