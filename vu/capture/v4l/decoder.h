/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Generic frame decoder                                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-FEB-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 977 $
 * $Date: 2008-03-03 10:39:17 +0100 (lun., 03 mars 2008) $
 */

#ifndef __TVU_DECODER_H__
#define __TVU_DECODER_H__

typedef struct decoder_s decoder_t;

typedef int decoder_process(decoder_t *decoder,
			    unsigned char *source_buf, int source_size,
			    unsigned char *target_buf, int target_size);
typedef void decoder_destroy(decoder_t *decoder);

struct decoder_s {
  decoder_process *process;
  decoder_destroy *destroy;
};

#endif /* __TVU_DECODER_H__ */
