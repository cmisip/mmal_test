#include <stdint.h>
#include <stdio.h>

class Buffer{


public:
   uint32_t cmd=0;              /**< Defines what the buffer header contains. This is a FourCC
                                   with 0 as a special value meaning stream data */

   uint8_t  *data=nullptr;            /**< Pointer to the start of the payload buffer (should not be
                                   changed by component) */
   uint32_t alloc_size=0;       /**< Allocated size in bytes of payload buffer */
   uint32_t length=0;           /**< Number of bytes currently used in the payload buffer (starting
                                   from offset) */
   uint32_t offset=0;           /**< Offset in bytes to the start of valid data in the payload buffer */

   uint32_t flags=0;            /**< Flags describing properties of a buffer header (see
                                   \ref bufferheaderflags "Buffer header flags") */

   int64_t  pts=0;              /**< Presentation timestamp in microseconds. \ref MMAL_TIME_UNKNOWN
                                   is used when the pts is unknown. */
   int64_t  dts=0;              /**< Decode timestamp in microseconds (dts = pts, except in the case
                                   of video streams with B frames). \ref MMAL_TIME_UNKNOWN
                                   is used when the dts is unknown. */
                                   
Buffer(uint32_t size);
~Buffer();                                   
};
