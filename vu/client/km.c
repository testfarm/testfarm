/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Keyboard/Mouse commands                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 24-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1036 $
 * $Date: 2008-12-07 14:18:30 +0100 (dim., 07 déc. 2008) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "useful.h"
#include "shell.h"
#include "log.h"

#include "error.h"
#include "keysyms.h"
#include "scroll.h"
#include "frame.h"
#include "km.h"


static frame_t *km_root = NULL;


/*******************************************************/
/* kp - Keypad/keyboard                                */
/*******************************************************/

static int kp_ton = 50;
static int kp_toff = 50;


static int kp_action(shell_t *shell, char *key, int pressed, char *tag)
{
  keysyms_t *keysym = keysyms_retrieve(key);

  if ( keysym == NULL ) {
    error(tag, "kp: Unknown or disabled key '%s'", key);
    return -1;
  }

  capture_action_key(km_root->capture, pressed, keysym->v);

  return 0;
}


static int kp_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *hdr = log_hdr_(tag);
  int i;

  /* No arguments: display matrix status */
  if ( check_argc(shell, cmd_argv, tag, 2, 0) )
    return -1;

  /* 'press' command */
  if ( strcmp(argv[1], "press") == 0 ) {
    if ( check_argc(shell, cmd_argv, tag, 3, 0) )
      return -1;

    for (i = 2; i < argc; i++)
      kp_action(shell, argv[i], 1, tag);
  }

  /* 'release' command */
  else if ( strcmp(argv[1], "release") == 0 ) {
    if ( check_argc(shell, cmd_argv, tag, 3, 0) )
      return -1;

    for (i = 2; i < argc; i++)
      kp_action(shell, argv[i], 0, tag);
  }

  /* 'delay' command */
  else if ( strcmp(argv[1], "delay") == 0 ) {
    if ( check_argc(shell, cmd_argv, tag, 0, 4) )
      return -1;

    for (i = 2; i < argc; i++) {
      if ( strncmp(argv[i], "on=", 3) == 0 ) {
	kp_ton = strtol(argv[i]+3, (char **) NULL, 0);
      }
      else if ( strncmp(argv[i], "off=", 4) == 0 ) {
	kp_toff = strtol(argv[i]+4, (char **) NULL, 0);
      }
      else {
        error(tag, "kp delay: Delay qualifier should be 'on=<value>' or 'off=<value>'");
	shell_std_help(shell, argv[0]);
	return -1;
      }
    }

    printf("%sTon=%dms Toff=%dms\n", hdr, kp_ton, kp_toff);
  }

  /* 'dial' command */
  else if ( strcmp(argv[1], "dial") == 0 ) {
    if ( check_argc(shell, cmd_argv, tag, 3, 0) )
      return -1;

    for (i = 2; i < argc; i++) {
      /* Press key */
      if ( kp_action(shell, argv[i], 1, tag) == 0 ) {
        if ( kp_ton > 0 )
          sleep_ms(kp_ton);

        /* Release key */
        kp_action(shell, argv[i], 0, tag);
        if ( kp_toff > 0 )
          sleep_ms(kp_toff);
      }
    }
  }

  /* 'type' command */
  else if ( strcmp(argv[1], "type") == 0 ) {
    char *str;
    int len;
    int i;

    if ( check_argc(shell, cmd_argv, tag, 3, 3) )
      return -1;

    str = argv[2];
    len = strlen(str);

    for (i = 0; i < len; i++) {
      capture_action_key(km_root->capture, 1, str[i]);
      if ( kp_ton > 0 )
        sleep_ms(kp_ton);
      capture_action_key(km_root->capture, 0, str[i]);
      if ( kp_toff > 0 )
        sleep_ms(kp_toff);
    }
  }

  /* 'enable' command */
  else if ( strcmp(argv[1], "enable") == 0 ) {
    int i;

    if ( check_argc(shell, cmd_argv, tag, 3, 0) )
      return -1;

    for (i = 2; i < argc; i++) {
      keysyms_class_t *class = keysyms_class(argv[i]);
      if ( class == NULL )
        error(tag, "kp enable: Unknown key symbol class '%s'", argv[i]);
      else
        class->enable = 1;
    }
  }

  /* 'disable' command */
  else if ( strcmp(argv[1], "disable") == 0 ) {
    int i;

    if ( check_argc(shell, cmd_argv, tag, 3, 0) )
      return -1;

    for (i = 2; i < argc; i++) {
      keysyms_class_t *class = keysyms_class(argv[i]);
      if ( class == NULL )
        error(tag, "kp disable: Unknown key symbol class '%s'", argv[i]);
      else
        class->enable = 0;
    }
  }

  /* 'show' command */
  else if ( strcmp(argv[1], "show") == 0 ) {
    /* If no class name is given, show list of classes */
    if ( argc < 3 ) {
      keysyms_show(hdr, NULL, 0);
    }
    else {
      int i;

      for (i = 2; i < argc; i++)
        keysyms_show(hdr, argv[i], 1);
    }
  }

  /* Unknown command */
  else {
    error(tag, "kp: Unknown sub-command");
    shell_std_help(shell, argv[0]);
    return -1;
  }

  return 0;
}


/*******************************************************/
/* mouse                                               */
/*******************************************************/

static int mouse_ton = 10;
static int mouse_toff = 100;
static int mouse_x = 0;
static int mouse_y = 0;
static int mouse_buttons = 0;


static int mouse_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *hdr = log_hdr_(tag);
  int i;

  /* No arguments: display matrix status */
  if ( check_argc(shell, cmd_argv, tag, 2, 0) )
    return -1;

  /* 'press' command */
  if ( strcmp(argv[1], "press") == 0 ) {
    int button = 1;

    if ( check_argc(shell, cmd_argv, tag, 0, 3) )
      return -1;

    /* Get button id */
    if ( argc > 2 ) {
      button = strtol(argv[2], (char **) NULL, 0);

      if ( button < 1 ) {
        error(tag, "mouse press: Illegal button number (should be 1,2,...)");
        return -1;
      }
    }

    /* Update mouse button mask */
    mouse_buttons |= (1 << (button-1));

    /* Perform mouse button action */
    capture_action_pointer(km_root->capture, mouse_buttons, mouse_x, mouse_y);

    /* Display mouse button mask */
    printf("%s%02X\n", hdr, mouse_buttons);
  }

  /* 'release' command */
  else if ( strcmp(argv[1], "release") == 0 ) {
    if ( check_argc(shell, cmd_argv, tag, 0, 3) )
      return -1;

    /* If no parameters, release all mouse buttons */
    if ( argc == 2 ) {
      /* Clear mouse button mask */
      mouse_buttons = 0;
    }

    else {
      int button = strtol(argv[2], (char **) NULL, 0);

      if ( button < 1 ) {
        error(tag, "mouse release: Illegal button number (should be 1,2,...)");
        return -1;
      }

      /* Update mouse button mask */
      mouse_buttons &= ~(1 << (button-1));
    }

    /* Perform mouse button action */
    capture_action_pointer(km_root->capture, mouse_buttons, mouse_x, mouse_y);

    /* Display mouse button mask */
    printf("%s%02X\n", hdr, mouse_buttons);
  }

  /* 'delay' command */
  else if ( strcmp(argv[1], "delay") == 0 ) {
    if ( check_argc(shell, cmd_argv, tag, 0, 5) )
      return -1;

    for (i = 2; i < argc; i++) {
      if ( strncmp(argv[i], "on=", 3) == 0 ) {
	mouse_ton = strtol(argv[i]+3, (char **) NULL, 0);
      }
      else if ( strncmp(argv[i], "off=", 4) == 0 ) {
	mouse_toff = strtol(argv[i]+4, (char **) NULL, 0);
      }
      else {
        error(tag, "mouse delay: Delay qualifier should be 'on=<ms>' or 'off=<ms>'");
	shell_std_help(shell, argv[0]);
	return -1;
      }
    }

    printf("%sTon=%dms Toff=%dms\n", hdr, mouse_ton, mouse_toff);
  }

  /* 'click' command */
  else if ( strcmp(argv[1], "click") == 0 ) {
    int button = 1;
    int count = 1;

    /* Check argument list */
    if ( check_argc(shell, cmd_argv, tag, 0, 4) )
      return -1;

    /* Get click count and button id */
    for (i = 2; i < argc; i++) {
      if ( argv[i][0] == '*' ) {
        count = strtol(argv[i]+1, (char **) NULL, 0);
        if ( count < 1 ) {
          error(tag, "mouse click: Illegal click count (should be >=1)");
          return -1;
        }
      }
      else {
        button = strtol(argv[i], (char **) NULL, 0);
        if ( button < 1 ) {
          error(tag, "mouse click: Illegal button number (should be 1,2,...)");
          return -1;
        }
      }
    }

    /* Perform button action */
    if ( button >= 0 ) {
      int mask = (1 << (button-1));

      for (i = 0; i < count; i++) {
	capture_action_pointer(km_root->capture, mouse_buttons | mask, mouse_x, mouse_y);
        sleep_ms(mouse_ton);
	capture_action_pointer(km_root->capture, mouse_buttons, mouse_x, mouse_y);
        sleep_ms(mouse_toff);
      }
    }

    /* Display mouse button mask */
    printf("%s%02X\n", hdr, mouse_buttons);
  }

  /* 'move' command */
  else if ( strcmp(argv[1], "move") == 0 ) {
    int setx = 0;
    int sety = 0;
    int nx = 0;
    int ny = 0;

    if ( check_argc(shell, cmd_argv, tag, 0, 5) )
      return -1;

    /* Get position value(s) */
    for (i = 2; i < argc; i++) {
      char *str = argv[i];

      if ( strncmp(str, "x=", 2) == 0 ) {
	nx = atoi(str+2);
	setx = 1;
      }
      else if ( strncmp(str, "dx=", 3) == 0 ) {
	nx = mouse_x + atoi(str+3);
	setx = 1;
      }
      else if ( strncmp(str, "y=", 2) == 0 ) {
	ny = atoi(str+2);
	sety = 1;
      }
      else if ( strncmp(str, "dy=", 3) == 0 ) {
	ny = mouse_y + atoi(str+3);
	sety = 1;
      }
      else if ( (str[0] == '+') || (str[0] == '-') ) {
	frame_geometry_t g = FRAME_GEOMETRY_NULL;
	frame_geometry_t g0;

	if ( frame_geometry_parse_position(str, &g) ) {
	  error(tag, "mouse move: illegal position format");
	  shell_std_help(shell, argv[0]);
	  return -1;
	}

	capture_get_window(km_root->capture, &g0);
	frame_geometry_clip(&g, &g0);

	setx = sety = 1;
	nx = g0.x + g.x;
	ny = g0.y + g.y;
      }
      else {
	shell_std_help(shell, argv[0]);
	return -1;
      }
    }

    /* Check & Set X position */
    if ( setx ) {
      int width = km_root->hdr.fb->rgb.width;

      if  ( nx >= width )
	nx = (width-1);
      if ( nx < 0 )
	nx = 0;

      mouse_x = nx;
    }

    /* Check & Set Y position */
    if ( sety ) {
      int height = km_root->hdr.fb->rgb.height;

      if ( ny >= height )
	ny = (height-1);
      if ( ny < 0 )
	ny = 0;

      mouse_y = ny;
    }

    /* Update mouse position */
    capture_action_pointer(km_root->capture, mouse_buttons, mouse_x, mouse_y);

    /* Display mouse position */
    printf("%s x=%d y=%d\n", hdr, mouse_x, mouse_y);
  }

  /* 'scroll' command */
  else if ( strcmp(argv[1], "scroll") == 0 ) {
    int count = 1;
    int dir = -1;

    /* Check argument list */
    if ( check_argc(shell, cmd_argv, tag, 0, 4) )
      return -1;

    /* Get click count and button id */
    for (i = 2; i < argc; i++) {
      char *str = argv[i];

      if ( str[0] == '*' ) {
        count = strtol(argv[i]+1, (char **) NULL, 0);
        if ( count < 1 ) {
          error(tag, "mouse scroll: Illegal scroll count (should be >=1)");
          return -1;
        }
      }
      else if ( strcmp(str, "up") == 0 ) {
	dir = SCROLL_UP;
      }
      else if ( strcmp(str, "down") == 0 ) {
	dir = SCROLL_DOWN;
      }
      else if ( strcmp(str, "left") == 0 ) {
	dir = SCROLL_LEFT;
      }
      else if ( strcmp(str, "right") == 0 ) {
	dir = SCROLL_RIGHT;
      }
      else {
	error(tag, "mouse scroll: Illegal argument '%s'", str);
	shell_std_help(shell, argv[0]);
	return -1;
      }
    }

    if ( dir != -1 ) {
      for (i = 1; i <= count; i++) {
	/* Display scroll action */
	printf("%s SCROLL %s\n", log_hdr_(tag), scroll_str(dir));

	/* Perform scroll action */
	capture_action_scroll(km_root->capture, dir);

	/* Wait between 2 scroll actions */
	if ( i < count )
	  sleep_ms(mouse_toff);
      }
    }
  }

  /* Unknown command */
  else {
    error(tag, "mouse: Unknown sub-command");
    shell_std_help(shell, argv[0]);
    return -1;
  }

  return 0;
}


/*******************************************************/
/* Command interpreter setup                           */
/*******************************************************/

static shell_std_help_t km_help_tab[] = {
  { "kp",      "kp press <keysym>\n"
               "  Press key <keysym>\n"
               "kp release <keysym>\n"
               "  Release key <keysym>\n"
               "kp dial <keysym1> ...\n"
               "  Dial a sequence of keys\n"
               "kp type \"<text>\"\n"
               "  Type <text> on keyboard\n"
               "kp delay [on=<ms>] [off=<ms>]\n"
               "  Set and show on- and off-delays for dial/type commands\n"
               "kp enable <class> ...\n"
               "  Enable key symbol class(es)\n"
               "kp disable <class> ...\n"
               "  Enable key symbol class(es)\n"
               "kp show\n"
               "  Show the list of key symbols classes (between paren's if disabled)\n"
               "kp show <class> ...\n"
               "  Show content of key symbols definition class(es)\n" },
  { "mouse",   "mouse press [<button>]\n"
               "  Press mouse button (default:1)\n"
               "mouse release [<button>]\n"
               "  Release mouse button (default:all)\n"
               "mouse click [*<n>] [<button>]\n"
               "  Click <n> times on mouse button (default:1)\n"
               "mouse delay [on=<ms>] [off=<ms>]\n"
               "  Set and show on-delay and off-delay for click/dblclick commands\n"
               "mouse move\n"
               "  Show mouse pointer position\n"
               "mouse move [x|dx=<x>] [y|dy=<y>]\n"
               "  Move mouse pointer at position <x> and/or <y> (dx/dy implies relative move).\n"
               "  Position is referenced to physical display device frame.\n"
               "mouse move +<x>+<y>\n"
               "  Move mouse pointer at position <x> and <y>.\n"
               "  Position is referenced to active frame geometry.\n"
               "mouse scroll [*<n>] up|down|left|right\n"
               "  Roll mouse wheel up/down/left/right <n> times (default:1)\n" },
  { NULL,     NULL }
};


static shell_command_t km_cmd_tab[] = {
  { "kp",      kp_command,      "KEYPAD " },
  { "mouse",   mouse_command,   "MOUSE  " },
  { NULL,      shell_std_unknown, NULL }
};


int km_init(shell_t *shell, frame_t *root)
{
  /* Setup keyboard/mouse commands */
  shell_set_cmd(shell, km_cmd_tab);
  shell_std_set_help(km_help_tab);

  /* Set root frame pointer */
  km_root = root;

  return 0;
}


void km_done(shell_t *shell)
{
  /* Clear root frame pointer */
  km_root = NULL;

  /* Remove keyboard/mouse commands */
  if ( shell != NULL ) {
    shell_std_unset_help(km_help_tab);
    shell_unset_cmd(shell, km_cmd_tab);
  }
}
