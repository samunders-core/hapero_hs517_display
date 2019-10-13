#ifndef __LCD_DECODER_H__
#define __LCD_DECODER_H__

#include <Arduino.h>
#include "print_utils.h"

//awk -nF_ '/ascii/ {val="0x"substr($2,1,2);FS=" "} /r25,/{printf("%s    case 0x%02X: return '"'%c'"';\n",dupl[$4],$4,val+0);dupl[$4]="//";FS="_"}' lcd_encoding.txt | sed -re "s/'\o302\o220'/' '/" | tail -n +2
char decode_DE_byte(byte input, char* dot = NULL) {
  switch(input) {
    case 0x12: return ' ';
    case 0xDA: return '2';
    case 0xF2: return '3';
    case 0x66: return '4';
    case 0xB6: return '5';
    case 0xBE: return '6';
    case 0xE0: return '7';
    case 0xFE: return '8';
    case 0xF6: return '9';
    case 0xFC: return '0';
    case 0x9E: return 'E';
    case 0xCE: return 'P';
    case 0x02: return '-';
    case 0x00: return '-';
    case 0x1C: return 'L';
    case 0x6E: return 'H';
//    case 0xFC: return 'O';
    case 0x3B: return 'a';  // or 'o' ?
    case 0x2A: return 'n';
    case 0x8E: return 'F';
    case 0x9C: return 'C';
    case 0xEE: return 'A';
//    case 0xEE: return 'R';
    case 0x0B: return 'r';
    case 0x7A: return 'd';
    case 0xF4: return 'J';
    case 0x21: return 'i';
    case 0x3F: return 'b';
//    case 0xB6: return 'S';
    case 0x7C: return 'V';
//    case 0x7C: return 'U';
//    case 0xBF: return 'G';  // prevents 6.
    case 0x39: return 'u';
    case 0x1E: return 't';
    case 0x2E: return 'h';
    case 0x3A: return 'o';  // or 'a' ?
    case 0x77: return 'y';
    case 0xED: return 'm';
//    case 0xE1: return 'Q';  // prevents 7.
    case 0x63: return '<';
    case 0x0F: return '>';
    case 0x1D: return 'q';
    case 0x11: return '_';
    case 0x81: return '#';
    case 0xC7: return ':';
//    case 0x3B: return '.';
//    case 0x9C: return '[';
    case 0xF0: return ']';
    case 0x0C: return 'X';
    case 0x6C: return 'M';
    case 0x60: return '1';	// 31h_one_49h_I_59h_Y
    default:
      if (dot) {
        *dot = '.';
        return decode_DE_byte((input % 2) ? input - 1 : (input + 1));
      }
      return input;
  }
}

void print_8_DE_bytes(Stream& stream, byte* bytes) {
  stream.print("DE ");
  for (int i = 0; i < 4; ++i) {
    char buffer[4] = "^  ";
    *buffer = decode_DE_byte(bytes[1 + i], buffer + 1);
    stream.write(buffer);
  }
  for (int i = 5; i < 8; ++i) {
    print_zero_hex(stream, *(bytes + i));
    stream.print(' ');
  }
}

void print_4_DE_bytes(char* buffer, byte* bytes) {
  for (int i = 0; i < 4; ++i, buffer += 2) {
    *buffer = decode_DE_byte(bytes[1 + i], buffer + 1);
  }
}

#endif
