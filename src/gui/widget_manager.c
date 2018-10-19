/*
 * gui/widget_manager.c - Manages GUI widgets
 *
 * Copyright (C) 2018 Alexandros Theodotou
 *
 * This file is part of Zrythm
 *
 * Zrythm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Zrythm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include "zrythm_app.h"
#include "gui/widget_manager.h"

void
init_widget_manager ()
{
  Widget_Manager * widget_manager = calloc (1, sizeof (Widget_Manager));
  /*widget_manager->widgets = g_hash_table_new (g_str_hash,*/
                         /*g_str_equal);*/

  zrythm_app->widget_manager = widget_manager;

  widget_manager->entries[0].target = "PLUGIN_DESCR";
  widget_manager->entries[0].flags = GTK_TARGET_SAME_APP;
  widget_manager->entries[0].info = 0;
  widget_manager->num_entries = 1;
}

