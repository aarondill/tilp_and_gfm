/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

/*  tilp - Ti Linking Program
 *  Copyright (C) 1999-2004  Romain Lievin
 *
 *  This program is free software you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif				/*  */

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <string.h>

#include "clock.h"
#include "tilp_core.h"

static TicalcClock tmp_clk;
static GladeXML *xml;
static gboolean modified;

static void update_fields(const TicalcClock * clk)
{
	GtkWidget *data;

	data = glade_xml_get_widget(xml, "spinbutton1");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data), clk->day);
	
	data = glade_xml_get_widget(xml, "optionmenu1");
	gtk_option_menu_set_history(GTK_OPTION_MENU(data), clk->month - 1);

	data = glade_xml_get_widget(xml, "spinbutton3");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data), clk->year);

	data = glade_xml_get_widget(xml, "spinbutton4");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data), clk->hours);
	
	data = glade_xml_get_widget(xml, "spinbutton5");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data), clk->minutes);
	
	data = glade_xml_get_widget(xml, "spinbutton6");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data), clk->seconds);
	
	data = glade_xml_get_widget(xml, "radiobutton1");
	if (clk->time_format == 12)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data),
					     TRUE);

	data = glade_xml_get_widget(xml, "radiobutton2");
	if (clk->time_format == 24)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data),
					     TRUE);

	data = glade_xml_get_widget(xml, "optionmenu2");
	gtk_option_menu_set_history(GTK_OPTION_MENU(data), 
				    clk->date_format - 1);
}

gint display_clock_dbox()
{
	GtkWidget *dbox;
	gint result;

	if (!(ti_calc.supported_operations() & OPS_CLOCK)) {
		tilp_error(ERR_VOID_FUNCTION);
		return -1;
	}
	if (tilp_calc_isready())
		return -1;
	if (tilp_error(ti_calc.get_clock(&tmp_clk, 0)))
		return -1;
	printl(0, "%02i/%02i/%02i %02i:%02i:%02i, %s, %s\n", tmp_clk.day,
		tmp_clk.month, tmp_clk.year, tmp_clk.hours,
		tmp_clk.minutes, tmp_clk.seconds,
		(tmp_clk.time_format == 12) ? "12" : "24",
		ticalc_format_to_date(tmp_clk.date_format));
	xml =
	    glade_xml_new(tilp_paths_build_glade("clock-2.glade"),
			  "clock_dbox", PACKAGE);
	if (!xml)
		g_error(_("action.c: GUI loading failed !\n"));
	glade_xml_signal_autoconnect(xml);
	dbox = glade_xml_get_widget(xml, "clock_dbox");
	update_fields(&tmp_clk);
	modified = FALSE;
	while (gtk_events_pending())
		gtk_main_iteration();
	result = gtk_dialog_run(GTK_DIALOG(dbox));
	switch (result) {
	case GTK_RESPONSE_OK:
		if (modified == FALSE)
			break;
		printl(0, "%02i/%02i/%02i %02i:%02i:%02i, %s, %s\n",
			tmp_clk.day, tmp_clk.month, tmp_clk.year,
			tmp_clk.hours, tmp_clk.minutes, tmp_clk.seconds,
			(tmp_clk.time_format == 12) ? "12" : "24",
			ticalc_format_to_date(tmp_clk.date_format));
		if (tilp_calc_isready())
			break;
		if (tilp_error(ti_calc.set_clock(&tmp_clk, 0)))
			break;
		modified = FALSE;
		break;
	case GTK_RESPONSE_HELP:
		break;
	default:
		break;
	}
	gtk_widget_destroy(dbox);
	return 0;
}

GLADE_CB void
clock_radiobutton3_toggled(GtkToggleButton * togglebutton,
			   gpointer user_data)
{
} 

GLADE_CB void
clock_radiobutton4_toggled(GtkToggleButton * togglebutton,
			   gpointer user_data)
{
} 

GLADE_CB void
clock_radiobutton1_toggled(GtkToggleButton * togglebutton,
			   gpointer user_data)
{
	tmp_clk.time_format = 12;
	modified = TRUE;
} 

GLADE_CB void
clock_radiobutton2_toggled(GtkToggleButton * togglebutton,
			   gpointer user_data)
{
	tmp_clk.time_format = 24;
	modified = TRUE;
} 

/*
GLADE_CB void clock_entry2_changed(GtkEditable * editable,
				     gpointer user_data)
{
	gchar *ed = gtk_editable_get_chars(editable, 0, -1);
	tmp_clk.date_format = ticalc_date_to_format(ed);
	modified = TRUE;
}
*/

GLADE_CB void comm_optionmenu2_changed     (GtkOptionMenu   *optionmenu,
                                            gpointer         user_data)
{
	gint nitem = gtk_option_menu_get_history(optionmenu);
	
	tmp_clk.date_format = nitem + 1;
}

/*
#ifdef __WIN32__
#define strcasecmp _stricmp
#endif			
GLADE_CB void clock_entry1_changed(GtkEditable * editable,
				   gpointer user_data)
{
	gchar *ed = gtk_editable_get_chars(editable, 0, -1);
	if (!strcasecmp(ed, _("Jan")))
		tmp_clk.month = 1;

	else if (!strcasecmp(ed, _("Feb")))
		tmp_clk.month = 2;

	else if (!strcasecmp(ed, _("Mar")))
		tmp_clk.month = 3;

	else if (!strcasecmp(ed, _("Apr")))
		tmp_clk.month = 4;

	else if (!strcasecmp(ed, _("May")))
		tmp_clk.month = 5;

	else if (!strcasecmp(ed, _("Jun")))
		tmp_clk.month = 6;

	else if (!strcasecmp(ed, _("Jul")))
		tmp_clk.month = 7;

	else if (!strcasecmp(ed, _("Aug")))
		tmp_clk.month = 8;

	else if (!strcasecmp(ed, _("Sep")))
		tmp_clk.month = 9;

	else if (!strcasecmp(ed, _("Oct")))
		tmp_clk.month = 10;

	else if (!strcasecmp(ed, _("Nov")))
		tmp_clk.month = 11;

	else if (!strcasecmp(ed, _("Dec")))
		tmp_clk.month = 12;
	modified = TRUE;
}
*/

GLADE_CB void comm_optionmenu1_changed     (GtkOptionMenu   *optionmenu,
                                            gpointer         user_data)
{
	gint nitem = gtk_option_menu_get_history(optionmenu);
	
	tmp_clk.month = nitem + 1;
}

GLADE_CB void clock_spinbutton3_changed(GtkEditable * editable,
					gpointer user_data)
{
	tmp_clk.year =
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(user_data));
	modified = TRUE;
} 

GLADE_CB void clock_spinbutton4_changed(GtkEditable * editable,
					  gpointer user_data)
{
	tmp_clk.hours =
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(user_data));
	modified = TRUE;
} 

GLADE_CB void clock_spinbutton5_changed(GtkEditable * editable,
					  gpointer user_data)
{
	tmp_clk.minutes =
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(user_data));
	modified = TRUE;
} 

GLADE_CB void clock_spinbutton6_changed(GtkEditable * editable,
					  gpointer user_data)
{
	tmp_clk.seconds =
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(user_data));
	modified = TRUE;
} 

GLADE_CB void clock_spinbutton1_changed(GtkEditable * editable,
					  gpointer user_data)
{
	tmp_clk.day =
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(user_data));
	modified = TRUE;
} 

GLADE_CB void clock_sync_button_clicked(GtkButton * button,
					  gpointer user_data)
{
	time_t tt;
	struct tm *lt;
	TicalcClock tc;

	time(&tt);
	lt = localtime(&tt);
	memcpy(&tc, &tmp_clk, sizeof(TicalcClock));
	tc.day = lt->tm_mday;
	tc.month = lt->tm_mon + 1;
	tc.year = lt->tm_year + 1900;
	tc.hours = lt->tm_hour;
	tc.minutes = lt->tm_min;
	tc.seconds = lt->tm_sec;

	if (tilp_calc_isready())
		return;
	if (tilp_error(ti_calc.set_clock(&tc, 0)))
		return;

	update_fields(&tc);
	modified = FALSE;
}