/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* Key symbols management                                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-NOV-2001                                                    */
/****************************************************************************/

/*
 * $Revision: 487 $
 * $Date: 2007-04-25 15:20:30 +0200 (mer., 25 avril 2007) $
 */

#ifndef __TVU_KEYSYMS_H__
#define __TVU_KEYSYMS_H__

typedef struct keysyms_class_s keysyms_class_t;

typedef struct {
  char *sym;
  unsigned long v;
  keysyms_class_t *class;
} keysyms_t;

struct keysyms_class_s {
  char *name;
  keysyms_t *tab;
  int size;
  int enable;
};

extern void keysyms_init(void);
extern void keysyms_destroy(void);

extern void keysyms_show(char *hdr, char *class_name, int show_content);
extern keysyms_class_t *keysyms_class(char *class_name);
extern keysyms_t *keysyms_retrieve(char *key);
extern keysyms_t *keysyms_retrieve_by_code(unsigned long v);

#endif /* __TVU_KEYSYMS_H__ */
