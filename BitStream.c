/**
 * @file BitStream.c
 *
 * @brief Implements the routines for bitstream manipulation
 * 
 * @author Makarand Kulkarni
 * 
 * @internal BitStreamCreate
 *           BitStreamDelete
 *           BitStreamRealloc
 *           BitStreamShow
 *           BitStreamPutByte
 *           BitStreamGetByte
 *           BitStreamPutBits
 *           BitStreamGetBits
 *	     BitStreamFill
 *	     BitStreamFillAscii
 *
 * Copyright (c) 2017, Makarand Kulkarni under GPLv3 License
 */

#include "BitStream.h"

/**
 * @fn unsigned char xtoi(char c)
 *
 * @brief converts ascii hex character to integer value
 *
 * Modelled on lines of atoi :) 
 * @param [in] c\n
 * 	valid hex character, [A-F],[a-f],[0-9], illegal hex will throw assert!
 * 	unfortunately there are no Exceptions!
 *
 * @returns \n
 * 	Integer value of corresponding ascii hex character
 */
static inline unsigned char xtoi(char c) {
      unsigned char ll;
      if (c >= '0' && c <= '9')
             ll = c - '0';
      else if (c >= 'a' && c <= 'f')
            ll = c - 'a' + 10;
      else if (c >= 'A' && c <- 'F')
            ll = c - 'A' + 10;
      else 
            assert(0);
       return ll;
}

/**
 * @fn int32_t strtox(const char* in, uint8_t *out, uint16_t size)
 *
 * @brief Convert HEX ascii string into byte array of integers
 * THe string has to be terminated by '\0' character, else the result is not
 * guaranteed
 * Routine does not take care of overflow cases in output buffer if not
 * allocated properly
 *
 * @param [in] in\n
 * 	pointer to the string containing HEX ascii characters
 * @param [out] out\n
 * 	pre-allocated byte buffer to hold the converted values
 * @param [in] size\n
 * 	size of the output buffer
 *
 * @returns number of bytes converted (1 converted byte = 2 HEX ascii chars)
 */
static inline int32_t strtox(const char* in, uint8_t *out, uint16_t size) {
    short len = strlen(in);
    short i = 0, j   = 0;

    if (len % 2) {
       i = 1;
       out[j++] = xtoi(in[0]) ;
    }
    while (i < len) {
        char ll = xtoi(in[i]);
        char uu = xtoi(in[i+1]);
        out[j++] = ll << 4 | uu;

        if (j >= size) 
            return (-1);
        i += 2;
    }
    return j;
}
 
/**
 * @ingroup Bitstream
 * @fn BitStream* BitStreamCreate(uint16_t nbits)
 * @brief Creates a object of type BitStream and allocates space to hold nbits
 *
 * @param [in] nbits\n
 * 	number of bits to hold in bit stream. If nbits is zero, just a container
 * 	object is created and new buffer can be added with BitStreamBuffer()
 * @returns pointer to newly created bit stream object, NULL on failure
 */
BitStream* BitStreamCreate(uint16_t nbits) {
   
   BitStream *bs = (BitStream *)malloc(sizeof(BitStream));
   if (bs != NULL) { 

      if (nbits) {
        bs->array = (uint8_t *)calloc((nbits + BITS_PER_BYTE - 1)/BITS_PER_BYTE,
			sizeof(uint8_t));
        if (NULL == bs->array) {
          free(bs);
          bs = NULL;
        } else {
	  memset(bs->array, (nbits + BITS_PER_BYTE - 1)/BITS_PER_BYTE, '\0');
        }
      } else {
	 bs->array = NULL;
      }
   }
   bs->nbits = nbits;

   return bs;
}

/**
 * @ingroup Bitstream
 *
 * @fn BitStreamRealloc(BitStream* bs, uint8_t buffer, uint16_t nbits) 
 *
 * @brief Reinitialize the BitStream buffer to a new one
 * 	Routine will create a new one with number of bits if not provided
 * 	else use the buffer provided as param
 * 	In case there's no new buffer provided, it calls realloc() to adjust
 * 	the size of the allocated buffer
 * @param [in] bs\n
 * 	BitStream object to operate on
 * @param [in] *buffer\n
 * 	Buffer pointer to use for reallocation, if NULL, either a new one or 
 * 	reallocated one will be used
 * @param [in] nbits\n
 * 	size in bits of the new buffer
 * @returns none
 */
void BitStreamRealloc(BitStream* bs, uint8_t *buffer, uint16_t nbits) {
   if (bs) { 
      if (bs->array) {
         if (buffer) {
	    free (bs->array);
	    bs->array = buffer;
	 } else {
            if (nbits) {
               bs->array = (uint8_t *)realloc(bs->array, 
			       (nbits + BITS_PER_BYTE - 1)/ BITS_PER_BYTE);
	    } else {
	       bs->array = NULL;
	    }
	 }
      } else {
	 bs->array = buffer;
      }
      bs->nbits = nbits;
   }
}


/**
 * @ingroup BitStream
 * @fn void BitStreamDelete(BitStream* bs)
 *
 * @brief Deletes the bit stream object, frees the memory holding bits
 *
 * @param [in] bs\n
 * 	pointer to bit stream object
 * @returns none
 */
void BitStreamDelete(BitStream* bs) {
   if (bs != NULL) {
      if (bs->array != NULL) {
         free(bs->array);
      }
      free(bs);
   }
}

/**
 * @ingroup BitStream
 * @fn void BitSreamShow(BitStream* bs, const char* fmt)
 * @brief Show contents of bit stream as hex array
 *
 * @param [in]	bs\n
 *  	pointer to bitstream to show
 * @returns void
 */
void BitStreamShow(BitStream* bs) {
   uint16_t i = 0;

   if (bs != NULL && bs->array != NULL) {
      printf("%03d\t", i);
      for (i = 0; i < (bs->nbits + BITS_PER_BYTE - 1)/BITS_PER_BYTE; i++) {
         if ((i != 0) && (i % 4 == 0))
		 printf("  ");
         if ((i != 0) && (i % 8 == 0))
		 printf("\n%03d\t", i);
         printf("%02x ", bs->array[i]);
      }
   } else {
      printf("NULL!\n");
   }
}

/**
 * @ingroup BitStream
 * @fn uint16_t BitStreamPutByte(BitStream* bs, uint8_t byte, uint16_t offset,\n 
 * 	uint16_t nbits) 
 *
 * @brief inserts maximum 1 byte of data in bit stream at offset (in bits) nbits
 *
 * If the number of bits inserted are less than BITS_PER_BYTE (8) then the bits
 * are shifted left so that they are aligned in network order, i.e, inserting 3 
 * bits (101b) will cause the bits to be inserted in the specified byte as follows
 * 	- 101xxxxxb
 * where 'x' indicates the existing bits in stream
 *
 * @param [in] bs\n
 * 	Bit stream in which the bits are to be inserted
 * @param [in] byte\n
 * 	Bits to be inserted in bit stream (maximum 8)
 * @param [in] offset\n
 * 	offset in bits at which the above byte is to be inserted
 * @param [in] nbits\n
 * 	number of bits to insert, by default is should be 8, else the shift as
 * 	explained above applies
 * @returns Number of bits inserted. Insertion fails while inserting bits 
 * 	beyond the size of bit stream
 */
uint16_t BitStreamPutByte(BitStream* bs, uint8_t byte, uint16_t offset, 
	uint16_t nbits) {

   uint16_t curBits;
   uint8_t mask;
   uint16_t bitsCopied = 0;

   DECL_BYTE_OFFSET(i);
   DECL_BITS_OFFSET(j);

   if (nbits < 8) byte = byte << (BITS_PER_BYTE - nbits);

   curBits = MIN((BITS_PER_BYTE - j), nbits);

   mask = (0xFF >> j) & (0xFF << (BITS_PER_BYTE - (j + curBits)));

   bs->array[i] = (bs->array[i] & ~mask) | ((byte >> j) & mask);

   bitsCopied += curBits;

   byte = byte << curBits;

   if (curBits < nbits) {
      curBits = nbits - curBits;

      mask = 0xFF << (BITS_PER_BYTE - curBits);

      bs->array[i + 1] = (bs->array[i + 1] & ~mask) | (byte & mask);

      bitsCopied += curBits;
   }
   return bitsCopied;
}

/**
 * @ingroup BitStream
 * @fn uint16_t BitStreamGetByte(BitStream* bs, uint8_t *byte, uint16_t offset,\n 
 * 	uint16_t nbits) 
 *
 * @brief fetches maximum 1 byte of data in bit stream at offset (in bits) nbits
 *
 * If the number of bits requested are less than BITS_PER_BYTE (8) then the bits
 * are shifted right so that they are aligned in host order, i.e, requesting 3 
 * bits at offset 0, will fetch the bits as 00000xxx where xxx are the fetched 
 * bits
 *
 * @param [in] bs\n
 * 	Bit stream in which the bits are to be inserted
 * @param [out] *byte\n
 * 	Bits fetched from the stream (maxium 8)
 * @param [in] offset\n
 * 	offset in bits from which the above byte is to be fetched
 * @param [in] nbits\n
 * 	number of bits to insert, by default is should be 8, else the shift as
 * 	explained above applies
 * @returns Number of bits fetched. Retrieval fails while fetching bits 
 * 	beyond the size of bit stream
 */
uint16_t BitStreamGetByte(BitStream *bs, uint8_t *byte, uint16_t offset, 
		uint16_t nbits) {

   uint16_t curBits;
   uint8_t mask;
   uint16_t bitsCopied = 0;

   DECL_BYTE_OFFSET(i);
   DECL_BITS_OFFSET(j);

   if (offset + nbits > bs->nbits)
	   return (0);

   curBits = MIN((BITS_PER_BYTE - j), nbits);

   mask = (0xFF >> j) & (0xFF << (BITS_PER_BYTE - (j + curBits)));

   *byte = (bs->array[i] & mask) << j;

   bitsCopied += curBits;

   if (curBits < nbits) {
      curBits = nbits - curBits;

      mask = 0xFF << (BITS_PER_BYTE - curBits);

      *byte |= (bs->array[i + 1] & mask) >> (BITS_PER_BYTE - j);

      bitsCopied += curBits;
   }

   if (nbits < 8) *byte = *byte >> (BITS_PER_BYTE - nbits);

   return bitsCopied;
}

/**
 * @ingroup BitStream
 * @fn uint16_t BitStreamFill(BitStream* bs, uint8_t* inp, uint16_t nbits) 
 *
 * @brief fills the bytes from input buffer into bit stream
 *
 * @param [in,out] bs\n
 * 	bit stream to fill data in
 * @param [in] *inp\n
 * 	pointer to the data to be copied
 * @param [in] nbits\n
 * 	size of input data in bits
 * @returns number of bits copied into bit stream
 */
uint16_t BitStreamFill(BitStream* bs, uint8_t* inp, uint16_t nbits) {
   uint16_t bitsCopied = 0;
   
   while (bitsCopied < bs->nbits) {
      bitsCopied += BitStreamPutByte(bs, *inp++, bitsCopied, BITS_PER_BYTE);
   }

   return bitsCopied;
}

/**
 * @ingroup BitStream
 * @fn uint16_t BitStreamFillAscii(BitStream* bs, uint8_t* inp)
 *
 * @brief fills the bytes from input HEX ascii buffer into bit stream
 *
 * @param [in,out] bs\n
 * 	bit stream to fill data in
 * @param [in] *inp\n
 * 	pointer to the data to be copied
 * @returns number of bits copied into bit stream
 */
uint16_t BitStreamFillAscii(BitStream* bs, const char* inp) {
   
   uint16_t size = (strlen(inp) + 1) >> 1;

   if (bs) {
      BitStreamRealloc(bs, NULL, size * BITS_PER_BYTE);

      strtox(inp, bs->array, size);
   }
   return size * BITS_PER_BYTE;
}

/**
 * @ingroup BitStream
 * @fn BitStream* BitStreamHex2Base64(BitStream *bs) 
 *
 * @brief converts input bitstream of HEX ascii characters into Base64 bitstream
 *
 * @param [in] *bs\n
 * 	pointer to HEX ascii bit stream for conversion
 * @returns pointer to Base64 bit stream converted from input, NULL in case of
 * 	any error
 *
 * BitStream* BitStreamHex2Base64(BitStream *bs) {
 *
 * BitStream* out = NULL;
 *
 * return out;
 * }
 */