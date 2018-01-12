.export _UnRLE

.segment "ZEROPAGE"
RLE_LOW:	.res 1
RLE_HIGH:	.res 1
RLE_TAG:	.res 1
RLE_BYTE:	.res 1

.segment "CODE"

.include "rle.s"

