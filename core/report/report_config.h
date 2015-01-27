/****************************************************************************/
/* TestFarm                                                                 */
/* Test Report Configuration                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 18-DEC-2001                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __TESTFARM_REPORT_CONFIG_H__
#define __TESTFARM_REPORT_CONFIG_H__

#define REPORT_CONFIG_SUBDIR "report"


/* Report config flags */
#define REPORT_CONFIG_FULL                 0xFFFFFFFF
#define REPORT_CONFIG_DEFAULT              0x00FFFF0F

#define REPORT_CONFIG_DURATION             0x00000001
#define REPORT_CONFIG_DUMP                 0x00000002
#define REPORT_CONFIG_CRITICITY            0x00000004
#define REPORT_CONFIG_VALIDATED            0x00000008

#define REPORT_CONFIG_OPERATOR             0x00000010
#define REPORT_CONFIG_NONSIGNIFICANT       0x00000020
#define REPORT_CONFIG_LOG                  0x00000040

#define REPORT_CONFIG_IN_TITLE             0x00000100
#define REPORT_CONFIG_IN_HEADER            0x00000200
#define REPORT_CONFIG_IN_VERDICT           0x00000400
#define REPORT_CONFIG_STANDARD             0x00000800

#define REPORT_CONFIG_CASE                 0x00001000
#define REPORT_CONFIG_SCENARIO             0x00002000
#define REPORT_CONFIG_VALIDATION_STATE     0x00004000

#define REPORT_CONFIG_VERDICT_PASSED       0x00010000
#define REPORT_CONFIG_VERDICT_FAILED       0x00020000
#define REPORT_CONFIG_VERDICT_INCONCLUSIVE 0x00040000
#define REPORT_CONFIG_VERDICT_SKIP         0x00080000
#define REPORT_CONFIG_VERDICT_ANY          0x000F0000
#define REPORT_CONFIG_VERDICT_FLAG(__result__)  (1<<((__result__)+16))

#define REPORT_CONFIG_DUMP_PASSED          0x00100000
#define REPORT_CONFIG_DUMP_FAILED          0x00200000
#define REPORT_CONFIG_DUMP_INCONCLUSIVE    0x00400000
#define REPORT_CONFIG_DUMP_SKIP            0x00800000
#define REPORT_CONFIG_DUMP_ANY             0x00F00000
#define REPORT_CONFIG_DUMP_FLAG(__result__)  (1<<((__result__)+20))

/* Report config indexes */
typedef struct {
  char *name;
  char *fname;
  char *stylesheet;
  unsigned long conf;
} report_config_t;

typedef struct {
  char *id;
  unsigned long mask;
} report_config_item_t;

extern report_config_item_t report_config_table[];

extern char *report_config_load(report_config_t *rc, char *name);
extern int report_config_save(report_config_t *rc, char *name);
extern void report_config_clear(report_config_t *rc);

extern report_config_t *report_config_alloc(void);
extern void report_config_destroy(report_config_t *rc);

extern void report_config_set_stylesheet(report_config_t *rc, char *stylesheet);
extern char *report_config_get_stylesheet(report_config_t *rc);

#endif /* __TESTFARM_REPORT_CONFIG_H__ */
