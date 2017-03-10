#include "BitStream.h"

/**
 * the cryptopals crypto challenges
 *
 * Set 1 / Challenge 2
 *
 * Fixed XOR
 * Write a function that takes two equal-length buffers and produces their XOR combination.
 *
 * If your function works properly, then when you feed it the string:
 *
 *   1c0111001f010100061a024b53535009181c
 *
 * ... after hex decoding, and when XOR'd against:
 *
 *   686974207468652062756c6c277320657965
 * ... should produce:
 *
 *   746865206b696420646f6e277420706c6179
 */   
int main() {
   BitStream  *bx, *by, *bz;

   bx = BitStreamCreateAscii("1c0111001f010100061a024b53535009181c");

   by = BitStreamCreateAscii("686974207468652062756c6c277320657965");

   if (bx && by) {
         bz = BitStreamExclusiveOr(bx, by);

         BitStreamShow(bz);
   } /* if (bx && by) */

   BitStreamDelete(bx);
   BitStreamDelete(by);
   BitStreamDelete(bz);

   return (0);
}