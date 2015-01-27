/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_report_config_window (void)
{
  GtkWidget *report_config_window;
  GdkPixbuf *report_config_window_icon_pixbuf;
  GtkWidget *vbox0;
  GtkWidget *notebook1;
  GtkWidget *vbox5;
  GtkWidget *frame8;
  GtkWidget *vbox21;
  GtkWidget *show_standard;
  GtkWidget *stylesheet_box;
  GtkWidget *stylesheet_entry;
  GtkWidget *stylesheet_browse;
  GtkWidget *alignment14;
  GtkWidget *hbox45;
  GtkWidget *image25;
  GtkWidget *label34;
  GtkWidget *label33;
  GtkWidget *show_operator;
  GtkWidget *show_in_title;
  GtkWidget *show_in_header;
  GtkWidget *show_in_verdict;
  GtkWidget *show_duration;
  GtkWidget *show_validation_parens;
  GtkWidget *show_log;
  GtkWidget *label2;
  GtkWidget *vbox6;
  GtkWidget *show_scenario;
  GtkWidget *show_case;
  GtkWidget *show_criticity;
  GtkWidget *frame7;
  GtkWidget *table6;
  GtkWidget *show_nonsignificant;
  GtkWidget *show_verdict_passed;
  GtkWidget *alignment15;
  GtkWidget *hbox66;
  GtkWidget *image2057;
  GtkWidget *label70;
  GtkWidget *show_verdict_failed;
  GtkWidget *alignment16;
  GtkWidget *hbox67;
  GtkWidget *image2058;
  GtkWidget *label71;
  GtkWidget *show_verdict_inconclusive;
  GtkWidget *alignment17;
  GtkWidget *hbox68;
  GtkWidget *image2059;
  GtkWidget *label72;
  GtkWidget *show_verdict_skip;
  GtkWidget *alignment18;
  GtkWidget *hbox69;
  GtkWidget *image2060;
  GtkWidget *label73;
  GtkWidget *label10;
  GtkWidget *label3;
  GtkWidget *vbox20;
  GtkWidget *show_dump;
  GtkWidget *show_dump_box;
  GtkWidget *show_validation_state;
  GtkWidget *dump_output_area;
  GtkWidget *table3;
  GtkWidget *show_dump_passed;
  GtkWidget *alignment20;
  GtkWidget *hbox71;
  GtkWidget *image2062;
  GtkWidget *label75;
  GtkWidget *show_dump_failed;
  GtkWidget *alignment21;
  GtkWidget *hbox72;
  GtkWidget *image2063;
  GtkWidget *label76;
  GtkWidget *show_dump_inconclusive;
  GtkWidget *alignment22;
  GtkWidget *hbox73;
  GtkWidget *image2064;
  GtkWidget *label77;
  GtkWidget *show_dump_skip;
  GtkWidget *alignment19;
  GtkWidget *hbox70;
  GtkWidget *image2061;
  GtkWidget *label74;
  GtkWidget *label12;
  GtkWidget *label4;
  GtkWidget *hbox63;
  GtkWidget *label69;
  GtkWidget *name;
  GtkWidget *delete;
  GtkWidget *image2056;
  GtkWidget *hbuttonbox1;
  GtkWidget *accept;
  GtkWidget *cancel;
  GtkAccelGroup *accel_group;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  accel_group = gtk_accel_group_new ();

  report_config_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (report_config_window), 5);
  gtk_window_set_title (GTK_WINDOW (report_config_window), _("TestFarm - Report Configuration"));
  gtk_window_set_position (GTK_WINDOW (report_config_window), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW (report_config_window), TRUE);
  report_config_window_icon_pixbuf = create_pixbuf ("testfarm.png");
  if (report_config_window_icon_pixbuf)
    {
      gtk_window_set_icon (GTK_WINDOW (report_config_window), report_config_window_icon_pixbuf);
      gdk_pixbuf_unref (report_config_window_icon_pixbuf);
    }

  vbox0 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox0);
  gtk_container_add (GTK_CONTAINER (report_config_window), vbox0);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (vbox0), notebook1, TRUE, TRUE, 0);

  vbox5 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox5);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox5);
  gtk_container_set_border_width (GTK_CONTAINER (vbox5), 5);

  frame8 = gtk_frame_new (NULL);
  gtk_widget_show (frame8);
  gtk_box_pack_start (GTK_BOX (vbox5), frame8, FALSE, FALSE, 0);

  vbox21 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox21);
  gtk_container_add (GTK_CONTAINER (frame8), vbox21);
  gtk_container_set_border_width (GTK_CONTAINER (vbox21), 5);

  show_standard = gtk_check_button_new_with_mnemonic (_("Use standard Test Report layout"));
  gtk_widget_show (show_standard);
  gtk_box_pack_start (GTK_BOX (vbox21), show_standard, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_standard), TRUE);

  stylesheet_box = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (stylesheet_box);
  gtk_box_pack_start (GTK_BOX (vbox21), stylesheet_box, TRUE, TRUE, 0);

  stylesheet_entry = gtk_entry_new ();
  gtk_widget_show (stylesheet_entry);
  gtk_box_pack_start (GTK_BOX (stylesheet_box), stylesheet_entry, TRUE, TRUE, 0);

  stylesheet_browse = gtk_button_new ();
  gtk_widget_show (stylesheet_browse);
  gtk_box_pack_start (GTK_BOX (stylesheet_box), stylesheet_browse, FALSE, FALSE, 0);

  alignment14 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment14);
  gtk_container_add (GTK_CONTAINER (stylesheet_browse), alignment14);

  hbox45 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox45);
  gtk_container_add (GTK_CONTAINER (alignment14), hbox45);

  image25 = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image25);
  gtk_box_pack_start (GTK_BOX (hbox45), image25, FALSE, FALSE, 0);

  label34 = gtk_label_new_with_mnemonic (_("Browse"));
  gtk_widget_show (label34);
  gtk_box_pack_start (GTK_BOX (hbox45), label34, FALSE, FALSE, 0);

  label33 = gtk_label_new (_("Stylesheet"));
  gtk_widget_show (label33);
  gtk_frame_set_label_widget (GTK_FRAME (frame8), label33);

  show_operator = gtk_check_button_new_with_mnemonic (_("Show Operator name"));
  gtk_widget_show (show_operator);
  gtk_box_pack_start (GTK_BOX (vbox5), show_operator, FALSE, FALSE, 0);

  show_in_title = gtk_check_button_new_with_mnemonic (_("Show IN__TITLE output messages"));
  gtk_widget_show (show_in_title);
  gtk_box_pack_start (GTK_BOX (vbox5), show_in_title, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_in_title), TRUE);

  show_in_header = gtk_check_button_new_with_mnemonic (_("Show IN__HEADER output messages"));
  gtk_widget_show (show_in_header);
  gtk_box_pack_start (GTK_BOX (vbox5), show_in_header, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_in_header), TRUE);

  show_in_verdict = gtk_check_button_new_with_mnemonic (_("Show IN__VERDICT output messages"));
  gtk_widget_show (show_in_verdict);
  gtk_box_pack_start (GTK_BOX (vbox5), show_in_verdict, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_in_verdict), TRUE);

  show_duration = gtk_check_button_new_with_mnemonic (_("Show durations"));
  gtk_widget_show (show_duration);
  gtk_box_pack_start (GTK_BOX (vbox5), show_duration, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_duration), TRUE);

  show_validation_parens = gtk_check_button_new_with_mnemonic (_("Names in parenthesis if test not validated"));
  gtk_widget_show (show_validation_parens);
  gtk_box_pack_start (GTK_BOX (vbox5), show_validation_parens, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_validation_parens), TRUE);

  show_log = gtk_check_button_new_with_mnemonic (_("Generate HTML Test Log links"));
  gtk_widget_show (show_log);
  gtk_box_pack_start (GTK_BOX (vbox5), show_log, FALSE, FALSE, 0);

  label2 = gtk_label_new (_("General Options"));
  gtk_widget_show (label2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label2);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_CENTER);

  vbox6 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox6);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox6);
  gtk_notebook_set_tab_label_packing (GTK_NOTEBOOK (notebook1), vbox6,
                                      FALSE, FALSE, GTK_PACK_START);
  gtk_container_set_border_width (GTK_CONTAINER (vbox6), 5);

  show_scenario = gtk_check_button_new_with_mnemonic (_("Show verdicts by Scenario (leaf sequence)"));
  gtk_widget_show (show_scenario);
  gtk_box_pack_start (GTK_BOX (vbox6), show_scenario, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_scenario), TRUE);

  show_case = gtk_check_button_new_with_mnemonic (_("Show verdicts by Test Case"));
  gtk_widget_show (show_case);
  gtk_box_pack_start (GTK_BOX (vbox6), show_case, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_case), TRUE);

  show_criticity = gtk_check_button_new_with_mnemonic (_("Show criticity level"));
  gtk_widget_show (show_criticity);
  gtk_box_pack_start (GTK_BOX (vbox6), show_criticity, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_criticity), TRUE);

  frame7 = gtk_frame_new (NULL);
  gtk_widget_show (frame7);
  gtk_box_pack_start (GTK_BOX (vbox6), frame7, FALSE, FALSE, 0);

  table6 = gtk_table_new (5, 1, FALSE);
  gtk_widget_show (table6);
  gtk_container_add (GTK_CONTAINER (frame7), table6);
  gtk_container_set_border_width (GTK_CONTAINER (table6), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table6), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table6), 5);

  show_nonsignificant = gtk_check_button_new_with_mnemonic (_("Non-significant (no criticity)"));
  gtk_widget_show (show_nonsignificant);
  gtk_table_attach (GTK_TABLE (table6), show_nonsignificant, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  show_verdict_passed = gtk_check_button_new ();
  gtk_widget_show (show_verdict_passed);
  gtk_table_attach (GTK_TABLE (table6), show_verdict_passed, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  alignment15 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment15);
  gtk_container_add (GTK_CONTAINER (show_verdict_passed), alignment15);

  hbox66 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox66);
  gtk_container_add (GTK_CONTAINER (alignment15), hbox66);

  image2057 = create_pixmap (report_config_window, "passed.png");
  gtk_widget_show (image2057);
  gtk_box_pack_start (GTK_BOX (hbox66), image2057, FALSE, FALSE, 0);

  label70 = gtk_label_new_with_mnemonic (_("PASSED"));
  gtk_widget_show (label70);
  gtk_box_pack_start (GTK_BOX (hbox66), label70, FALSE, FALSE, 0);

  show_verdict_failed = gtk_check_button_new ();
  gtk_widget_show (show_verdict_failed);
  gtk_table_attach (GTK_TABLE (table6), show_verdict_failed, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_verdict_failed), TRUE);

  alignment16 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment16);
  gtk_container_add (GTK_CONTAINER (show_verdict_failed), alignment16);

  hbox67 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox67);
  gtk_container_add (GTK_CONTAINER (alignment16), hbox67);

  image2058 = create_pixmap (report_config_window, "failed.png");
  gtk_widget_show (image2058);
  gtk_box_pack_start (GTK_BOX (hbox67), image2058, FALSE, FALSE, 0);

  label71 = gtk_label_new_with_mnemonic (_("FAILED"));
  gtk_widget_show (label71);
  gtk_box_pack_start (GTK_BOX (hbox67), label71, FALSE, FALSE, 0);

  show_verdict_inconclusive = gtk_check_button_new ();
  gtk_widget_show (show_verdict_inconclusive);
  gtk_table_attach (GTK_TABLE (table6), show_verdict_inconclusive, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_verdict_inconclusive), TRUE);

  alignment17 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment17);
  gtk_container_add (GTK_CONTAINER (show_verdict_inconclusive), alignment17);

  hbox68 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox68);
  gtk_container_add (GTK_CONTAINER (alignment17), hbox68);

  image2059 = create_pixmap (report_config_window, "inconclusive.png");
  gtk_widget_show (image2059);
  gtk_box_pack_start (GTK_BOX (hbox68), image2059, FALSE, FALSE, 0);

  label72 = gtk_label_new_with_mnemonic (_("INCONCLUSIVE"));
  gtk_widget_show (label72);
  gtk_box_pack_start (GTK_BOX (hbox68), label72, FALSE, FALSE, 0);

  show_verdict_skip = gtk_check_button_new ();
  gtk_widget_show (show_verdict_skip);
  gtk_table_attach (GTK_TABLE (table6), show_verdict_skip, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  alignment18 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment18);
  gtk_container_add (GTK_CONTAINER (show_verdict_skip), alignment18);

  hbox69 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox69);
  gtk_container_add (GTK_CONTAINER (alignment18), hbox69);

  image2060 = create_pixmap (report_config_window, "skip.png");
  gtk_widget_show (image2060);
  gtk_box_pack_start (GTK_BOX (hbox69), image2060, FALSE, FALSE, 0);

  label73 = gtk_label_new_with_mnemonic (_("SKIP"));
  gtk_widget_show (label73);
  gtk_box_pack_start (GTK_BOX (hbox69), label73, FALSE, FALSE, 0);

  label10 = gtk_label_new (_("Show scenarios and test cases when..."));
  gtk_widget_show (label10);
  gtk_frame_set_label_widget (GTK_FRAME (frame7), label10);

  label3 = gtk_label_new (_("Verdict List"));
  gtk_widget_show (label3);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label3);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_CENTER);

  vbox20 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox20);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox20);
  gtk_container_set_border_width (GTK_CONTAINER (vbox20), 5);

  show_dump = gtk_check_button_new_with_mnemonic (_("Enable detailed output dump"));
  gtk_widget_show (show_dump);
  gtk_box_pack_start (GTK_BOX (vbox20), show_dump, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_dump), TRUE);

  show_dump_box = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (show_dump_box);
  gtk_box_pack_start (GTK_BOX (vbox20), show_dump_box, TRUE, TRUE, 0);

  show_validation_state = gtk_check_button_new_with_mnemonic (_("Show validation state"));
  gtk_widget_show (show_validation_state);
  gtk_box_pack_start (GTK_BOX (show_dump_box), show_validation_state, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_validation_state), TRUE);

  dump_output_area = gtk_frame_new (NULL);
  gtk_widget_show (dump_output_area);
  gtk_box_pack_start (GTK_BOX (show_dump_box), dump_output_area, FALSE, FALSE, 0);

  table3 = gtk_table_new (4, 1, FALSE);
  gtk_widget_show (table3);
  gtk_container_add (GTK_CONTAINER (dump_output_area), table3);
  gtk_container_set_border_width (GTK_CONTAINER (table3), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table3), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table3), 5);

  show_dump_passed = gtk_check_button_new ();
  gtk_widget_show (show_dump_passed);
  gtk_table_attach (GTK_TABLE (table3), show_dump_passed, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_dump_passed), TRUE);

  alignment20 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment20);
  gtk_container_add (GTK_CONTAINER (show_dump_passed), alignment20);

  hbox71 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox71);
  gtk_container_add (GTK_CONTAINER (alignment20), hbox71);

  image2062 = create_pixmap (report_config_window, "passed.png");
  gtk_widget_show (image2062);
  gtk_box_pack_start (GTK_BOX (hbox71), image2062, FALSE, FALSE, 0);

  label75 = gtk_label_new_with_mnemonic (_("PASSED"));
  gtk_widget_show (label75);
  gtk_box_pack_start (GTK_BOX (hbox71), label75, FALSE, FALSE, 0);

  show_dump_failed = gtk_check_button_new ();
  gtk_widget_show (show_dump_failed);
  gtk_table_attach (GTK_TABLE (table3), show_dump_failed, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_dump_failed), TRUE);

  alignment21 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment21);
  gtk_container_add (GTK_CONTAINER (show_dump_failed), alignment21);

  hbox72 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox72);
  gtk_container_add (GTK_CONTAINER (alignment21), hbox72);

  image2063 = create_pixmap (report_config_window, "failed.png");
  gtk_widget_show (image2063);
  gtk_box_pack_start (GTK_BOX (hbox72), image2063, FALSE, FALSE, 0);

  label76 = gtk_label_new_with_mnemonic (_("FAILED"));
  gtk_widget_show (label76);
  gtk_box_pack_start (GTK_BOX (hbox72), label76, FALSE, FALSE, 0);

  show_dump_inconclusive = gtk_check_button_new ();
  gtk_widget_show (show_dump_inconclusive);
  gtk_table_attach (GTK_TABLE (table3), show_dump_inconclusive, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_dump_inconclusive), TRUE);

  alignment22 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment22);
  gtk_container_add (GTK_CONTAINER (show_dump_inconclusive), alignment22);

  hbox73 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox73);
  gtk_container_add (GTK_CONTAINER (alignment22), hbox73);

  image2064 = create_pixmap (report_config_window, "inconclusive.png");
  gtk_widget_show (image2064);
  gtk_box_pack_start (GTK_BOX (hbox73), image2064, FALSE, FALSE, 0);

  label77 = gtk_label_new_with_mnemonic (_("INCONCLUSIVE"));
  gtk_widget_show (label77);
  gtk_box_pack_start (GTK_BOX (hbox73), label77, FALSE, FALSE, 0);

  show_dump_skip = gtk_check_button_new ();
  gtk_widget_show (show_dump_skip);
  gtk_table_attach (GTK_TABLE (table3), show_dump_skip, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_dump_skip), TRUE);

  alignment19 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment19);
  gtk_container_add (GTK_CONTAINER (show_dump_skip), alignment19);

  hbox70 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox70);
  gtk_container_add (GTK_CONTAINER (alignment19), hbox70);

  image2061 = create_pixmap (report_config_window, "skip.png");
  gtk_widget_show (image2061);
  gtk_box_pack_start (GTK_BOX (hbox70), image2061, FALSE, FALSE, 0);

  label74 = gtk_label_new_with_mnemonic (_("SKIP"));
  gtk_widget_show (label74);
  gtk_box_pack_start (GTK_BOX (hbox70), label74, FALSE, FALSE, 0);

  label12 = gtk_label_new (_("Show full output dump on..."));
  gtk_widget_show (label12);
  gtk_frame_set_label_widget (GTK_FRAME (dump_output_area), label12);

  label4 = gtk_label_new (_("Output Dump"));
  gtk_widget_show (label4);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label4);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_CENTER);

  hbox63 = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox63);
  gtk_box_pack_start (GTK_BOX (vbox0), hbox63, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox63), 5);

  label69 = gtk_label_new (_("Config Name:"));
  gtk_widget_show (label69);
  gtk_box_pack_start (GTK_BOX (hbox63), label69, FALSE, FALSE, 0);

  name = gtk_combo_box_entry_new ();
  gtk_widget_show (name);
  gtk_box_pack_start (GTK_BOX (hbox63), name, TRUE, TRUE, 0);

  delete = gtk_button_new ();
  gtk_widget_show (delete);
  gtk_box_pack_start (GTK_BOX (hbox63), delete, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, delete, _("Delete this Report Configuration"), NULL);

  image2056 = gtk_image_new_from_stock ("gtk-delete", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image2056);
  gtk_container_add (GTK_CONTAINER (delete), image2056);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox0), hbuttonbox1, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox1), 5);
  gtk_box_set_spacing (GTK_BOX (hbuttonbox1), 20);

  accept = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (accept);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), accept);
  GTK_WIDGET_SET_FLAGS (accept, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (accept, "clicked", accel_group,
                              GDK_Return, 0,
                              GTK_ACCEL_VISIBLE);

  cancel = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancel);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), cancel);
  GTK_WIDGET_SET_FLAGS (cancel, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (cancel, "clicked", accel_group,
                              GDK_Escape, 0,
                              GTK_ACCEL_VISIBLE);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (report_config_window, report_config_window, "report_config_window");
  GLADE_HOOKUP_OBJECT (report_config_window, vbox0, "vbox0");
  GLADE_HOOKUP_OBJECT (report_config_window, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT (report_config_window, vbox5, "vbox5");
  GLADE_HOOKUP_OBJECT (report_config_window, frame8, "frame8");
  GLADE_HOOKUP_OBJECT (report_config_window, vbox21, "vbox21");
  GLADE_HOOKUP_OBJECT (report_config_window, show_standard, "show_standard");
  GLADE_HOOKUP_OBJECT (report_config_window, stylesheet_box, "stylesheet_box");
  GLADE_HOOKUP_OBJECT (report_config_window, stylesheet_entry, "stylesheet_entry");
  GLADE_HOOKUP_OBJECT (report_config_window, stylesheet_browse, "stylesheet_browse");
  GLADE_HOOKUP_OBJECT (report_config_window, alignment14, "alignment14");
  GLADE_HOOKUP_OBJECT (report_config_window, hbox45, "hbox45");
  GLADE_HOOKUP_OBJECT (report_config_window, image25, "image25");
  GLADE_HOOKUP_OBJECT (report_config_window, label34, "label34");
  GLADE_HOOKUP_OBJECT (report_config_window, label33, "label33");
  GLADE_HOOKUP_OBJECT (report_config_window, show_operator, "show_operator");
  GLADE_HOOKUP_OBJECT (report_config_window, show_in_title, "show_in_title");
  GLADE_HOOKUP_OBJECT (report_config_window, show_in_header, "show_in_header");
  GLADE_HOOKUP_OBJECT (report_config_window, show_in_verdict, "show_in_verdict");
  GLADE_HOOKUP_OBJECT (report_config_window, show_duration, "show_duration");
  GLADE_HOOKUP_OBJECT (report_config_window, show_validation_parens, "show_validation_parens");
  GLADE_HOOKUP_OBJECT (report_config_window, show_log, "show_log");
  GLADE_HOOKUP_OBJECT (report_config_window, label2, "label2");
  GLADE_HOOKUP_OBJECT (report_config_window, vbox6, "vbox6");
  GLADE_HOOKUP_OBJECT (report_config_window, show_scenario, "show_scenario");
  GLADE_HOOKUP_OBJECT (report_config_window, show_case, "show_case");
  GLADE_HOOKUP_OBJECT (report_config_window, show_criticity, "show_criticity");
  GLADE_HOOKUP_OBJECT (report_config_window, frame7, "frame7");
  GLADE_HOOKUP_OBJECT (report_config_window, table6, "table6");
  GLADE_HOOKUP_OBJECT (report_config_window, show_nonsignificant, "show_nonsignificant");
  GLADE_HOOKUP_OBJECT (report_config_window, show_verdict_passed, "show_verdict_passed");
  GLADE_HOOKUP_OBJECT (report_config_window, alignment15, "alignment15");
  GLADE_HOOKUP_OBJECT (report_config_window, hbox66, "hbox66");
  GLADE_HOOKUP_OBJECT (report_config_window, image2057, "image2057");
  GLADE_HOOKUP_OBJECT (report_config_window, label70, "label70");
  GLADE_HOOKUP_OBJECT (report_config_window, show_verdict_failed, "show_verdict_failed");
  GLADE_HOOKUP_OBJECT (report_config_window, alignment16, "alignment16");
  GLADE_HOOKUP_OBJECT (report_config_window, hbox67, "hbox67");
  GLADE_HOOKUP_OBJECT (report_config_window, image2058, "image2058");
  GLADE_HOOKUP_OBJECT (report_config_window, label71, "label71");
  GLADE_HOOKUP_OBJECT (report_config_window, show_verdict_inconclusive, "show_verdict_inconclusive");
  GLADE_HOOKUP_OBJECT (report_config_window, alignment17, "alignment17");
  GLADE_HOOKUP_OBJECT (report_config_window, hbox68, "hbox68");
  GLADE_HOOKUP_OBJECT (report_config_window, image2059, "image2059");
  GLADE_HOOKUP_OBJECT (report_config_window, label72, "label72");
  GLADE_HOOKUP_OBJECT (report_config_window, show_verdict_skip, "show_verdict_skip");
  GLADE_HOOKUP_OBJECT (report_config_window, alignment18, "alignment18");
  GLADE_HOOKUP_OBJECT (report_config_window, hbox69, "hbox69");
  GLADE_HOOKUP_OBJECT (report_config_window, image2060, "image2060");
  GLADE_HOOKUP_OBJECT (report_config_window, label73, "label73");
  GLADE_HOOKUP_OBJECT (report_config_window, label10, "label10");
  GLADE_HOOKUP_OBJECT (report_config_window, label3, "label3");
  GLADE_HOOKUP_OBJECT (report_config_window, vbox20, "vbox20");
  GLADE_HOOKUP_OBJECT (report_config_window, show_dump, "show_dump");
  GLADE_HOOKUP_OBJECT (report_config_window, show_dump_box, "show_dump_box");
  GLADE_HOOKUP_OBJECT (report_config_window, show_validation_state, "show_validation_state");
  GLADE_HOOKUP_OBJECT (report_config_window, dump_output_area, "dump_output_area");
  GLADE_HOOKUP_OBJECT (report_config_window, table3, "table3");
  GLADE_HOOKUP_OBJECT (report_config_window, show_dump_passed, "show_dump_passed");
  GLADE_HOOKUP_OBJECT (report_config_window, alignment20, "alignment20");
  GLADE_HOOKUP_OBJECT (report_config_window, hbox71, "hbox71");
  GLADE_HOOKUP_OBJECT (report_config_window, image2062, "image2062");
  GLADE_HOOKUP_OBJECT (report_config_window, label75, "label75");
  GLADE_HOOKUP_OBJECT (report_config_window, show_dump_failed, "show_dump_failed");
  GLADE_HOOKUP_OBJECT (report_config_window, alignment21, "alignment21");
  GLADE_HOOKUP_OBJECT (report_config_window, hbox72, "hbox72");
  GLADE_HOOKUP_OBJECT (report_config_window, image2063, "image2063");
  GLADE_HOOKUP_OBJECT (report_config_window, label76, "label76");
  GLADE_HOOKUP_OBJECT (report_config_window, show_dump_inconclusive, "show_dump_inconclusive");
  GLADE_HOOKUP_OBJECT (report_config_window, alignment22, "alignment22");
  GLADE_HOOKUP_OBJECT (report_config_window, hbox73, "hbox73");
  GLADE_HOOKUP_OBJECT (report_config_window, image2064, "image2064");
  GLADE_HOOKUP_OBJECT (report_config_window, label77, "label77");
  GLADE_HOOKUP_OBJECT (report_config_window, show_dump_skip, "show_dump_skip");
  GLADE_HOOKUP_OBJECT (report_config_window, alignment19, "alignment19");
  GLADE_HOOKUP_OBJECT (report_config_window, hbox70, "hbox70");
  GLADE_HOOKUP_OBJECT (report_config_window, image2061, "image2061");
  GLADE_HOOKUP_OBJECT (report_config_window, label74, "label74");
  GLADE_HOOKUP_OBJECT (report_config_window, label12, "label12");
  GLADE_HOOKUP_OBJECT (report_config_window, label4, "label4");
  GLADE_HOOKUP_OBJECT (report_config_window, hbox63, "hbox63");
  GLADE_HOOKUP_OBJECT (report_config_window, label69, "label69");
  GLADE_HOOKUP_OBJECT (report_config_window, name, "name");
  GLADE_HOOKUP_OBJECT (report_config_window, delete, "delete");
  GLADE_HOOKUP_OBJECT (report_config_window, image2056, "image2056");
  GLADE_HOOKUP_OBJECT (report_config_window, hbuttonbox1, "hbuttonbox1");
  GLADE_HOOKUP_OBJECT (report_config_window, accept, "accept");
  GLADE_HOOKUP_OBJECT (report_config_window, cancel, "cancel");
  GLADE_HOOKUP_OBJECT_NO_REF (report_config_window, tooltips, "tooltips");

  gtk_window_add_accel_group (GTK_WINDOW (report_config_window), accel_group);

  return report_config_window;
}

GtkWidget*
create_error_window (void)
{
  GtkWidget *error_window;
  GdkPixbuf *error_window_icon_pixbuf;
  GtkWidget *vbox18;
  GtkWidget *hbox37;
  GtkWidget *image13;
  GtkWidget *label;
  GtkWidget *hseparator3;
  GtkWidget *ok;
  GtkAccelGroup *accel_group;

  accel_group = gtk_accel_group_new ();

  error_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (error_window), _("TestFarm - Error"));
  gtk_window_set_position (GTK_WINDOW (error_window), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW (error_window), TRUE);
  error_window_icon_pixbuf = create_pixbuf ("testfarm.png");
  if (error_window_icon_pixbuf)
    {
      gtk_window_set_icon (GTK_WINDOW (error_window), error_window_icon_pixbuf);
      gdk_pixbuf_unref (error_window_icon_pixbuf);
    }

  vbox18 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox18);
  gtk_container_add (GTK_CONTAINER (error_window), vbox18);

  hbox37 = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox37);
  gtk_box_pack_start (GTK_BOX (vbox18), hbox37, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox37), 5);

  image13 = gtk_image_new_from_stock ("gtk-dialog-error", GTK_ICON_SIZE_DIALOG);
  gtk_widget_show (image13);
  gtk_box_pack_start (GTK_BOX (hbox37), image13, TRUE, TRUE, 0);

  label = gtk_label_new (_(" "));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox37), label, FALSE, FALSE, 0);

  hseparator3 = gtk_hseparator_new ();
  gtk_widget_show (hseparator3);
  gtk_box_pack_start (GTK_BOX (vbox18), hseparator3, FALSE, FALSE, 0);

  ok = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (ok);
  gtk_box_pack_start (GTK_BOX (vbox18), ok, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (ok), 5);
  gtk_widget_add_accelerator (ok, "clicked", accel_group,
                              GDK_Return, 0,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (ok, "clicked", accel_group,
                              GDK_Escape, 0,
                              GTK_ACCEL_VISIBLE);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (error_window, error_window, "error_window");
  GLADE_HOOKUP_OBJECT (error_window, vbox18, "vbox18");
  GLADE_HOOKUP_OBJECT (error_window, hbox37, "hbox37");
  GLADE_HOOKUP_OBJECT (error_window, image13, "image13");
  GLADE_HOOKUP_OBJECT (error_window, label, "label");
  GLADE_HOOKUP_OBJECT (error_window, hseparator3, "hseparator3");
  GLADE_HOOKUP_OBJECT (error_window, ok, "ok");

  gtk_window_add_accel_group (GTK_WINDOW (error_window), accel_group);

  return error_window;
}

GtkWidget*
create_warning_window (void)
{
  GtkWidget *warning_window;
  GtkWidget *vbox32;
  GtkWidget *hbox64;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *hseparator9;
  GtkWidget *hbox65;
  GtkWidget *no;
  GtkWidget *yes;
  GtkAccelGroup *accel_group;

  accel_group = gtk_accel_group_new ();

  warning_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (warning_window), _("TestFarm - Warning"));
  gtk_window_set_position (GTK_WINDOW (warning_window), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW (warning_window), TRUE);

  vbox32 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox32);
  gtk_container_add (GTK_CONTAINER (warning_window), vbox32);

  hbox64 = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox64);
  gtk_box_pack_start (GTK_BOX (vbox32), hbox64, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox64), 5);

  image = gtk_image_new_from_stock ("gtk-dialog-warning", GTK_ICON_SIZE_DIALOG);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (hbox64), image, FALSE, FALSE, 0);

  label = gtk_label_new (_(" "));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox64), label, FALSE, FALSE, 0);

  hseparator9 = gtk_hseparator_new ();
  gtk_widget_show (hseparator9);
  gtk_box_pack_start (GTK_BOX (vbox32), hseparator9, FALSE, FALSE, 0);

  hbox65 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox65);
  gtk_box_pack_start (GTK_BOX (vbox32), hbox65, FALSE, FALSE, 0);

  no = gtk_button_new_from_stock ("gtk-no");
  gtk_widget_show (no);
  gtk_box_pack_start (GTK_BOX (hbox65), no, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (no), 5);
  gtk_widget_add_accelerator (no, "clicked", accel_group,
                              GDK_Escape, 0,
                              GTK_ACCEL_VISIBLE);

  yes = gtk_button_new_from_stock ("gtk-yes");
  gtk_widget_show (yes);
  gtk_box_pack_end (GTK_BOX (hbox65), yes, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (yes), 5);
  gtk_widget_add_accelerator (yes, "clicked", accel_group,
                              GDK_Return, 0,
                              GTK_ACCEL_VISIBLE);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (warning_window, warning_window, "warning_window");
  GLADE_HOOKUP_OBJECT (warning_window, vbox32, "vbox32");
  GLADE_HOOKUP_OBJECT (warning_window, hbox64, "hbox64");
  GLADE_HOOKUP_OBJECT (warning_window, image, "image");
  GLADE_HOOKUP_OBJECT (warning_window, label, "label");
  GLADE_HOOKUP_OBJECT (warning_window, hseparator9, "hseparator9");
  GLADE_HOOKUP_OBJECT (warning_window, hbox65, "hbox65");
  GLADE_HOOKUP_OBJECT (warning_window, no, "no");
  GLADE_HOOKUP_OBJECT (warning_window, yes, "yes");

  gtk_window_add_accel_group (GTK_WINDOW (warning_window), accel_group);

  return warning_window;
}
