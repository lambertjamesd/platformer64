
#ifndef _ALIGN_H
#define _ALIGN_H

#define ALIGN_8_BYTES(input)   (((input) + 0x7) & ~0x7)
#define ALIGN_64_BYTES(input)   (((input) + 0x3f) & ~0x3f)

#endif