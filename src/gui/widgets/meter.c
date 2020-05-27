/*
 * Copyright (C) 2018-2020 Alexandros Theodotou <alex at zrythm dot org>
 *
 * This file is part of Zrythm
 *
 * Zrythm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Zrythm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "audio/channel.h"
#include "gui/widgets/meter.h"
#include "gui/widgets/fader.h"
#include "utils/math.h"

G_DEFINE_TYPE (
  MeterWidget, meter_widget, GTK_TYPE_DRAWING_AREA)

static int
meter_draw_cb (
  GtkWidget * widget,
  cairo_t * cr, void* data)
{
  MeterWidget * self = (MeterWidget *) widget;

  GtkStyleContext *context =
  gtk_widget_get_style_context (widget);

  int width =
    gtk_widget_get_allocated_width (widget);
  int height =
    gtk_widget_get_allocated_height (widget);

  gtk_render_background (
    context, cr, 0, 0, width, height);

  /* get values */
  float peak = -198.0;
  float meter_val = -198.0;
  meter_get_value (
    self->meter, AUDIO_VALUE_FADER, &meter_val,
    &peak);

  float value_px = (float) height * meter_val;
  if (value_px < 0)
    value_px = 0;

  /* draw filled in bar */
  float width_without_padding =
    (float) (width - self->padding * 2);

  double intensity = (double) meter_val;
  const double intensity_inv = 1.0 - intensity;
  double r =
    intensity_inv * self->end_color.red +
    intensity * self->start_color.red;
  double g =
    intensity_inv * self->end_color.green +
    intensity * self->start_color.green;
  double b =
    intensity_inv * self->end_color.blue  +
    intensity * self->start_color.blue;

  cairo_set_source_rgba (cr, r,g,b, 1.0);

  /* use gradient */
  cairo_pattern_t * pat =
    cairo_pattern_create_linear (
      0.0, 0.0, 0.0, height);
  cairo_pattern_add_color_stop_rgba (
    pat, 0, r, g, b, 1);
  cairo_pattern_add_color_stop_rgba (
    pat, 0.5, r, g, b, 1);
  cairo_pattern_add_color_stop_rgba (
    pat, 0.75, 0, 1, 0, 1);
  cairo_pattern_add_color_stop_rgba (
    pat, 1, 0, 0.2, 1, 1);
  cairo_set_source (
    cr, pat);

  float x = self->padding;
  cairo_rectangle (
    cr, x,
    (float) height - value_px,
    x + width_without_padding,
    value_px);
  cairo_fill(cr);

  /* draw border line */
  cairo_set_source_rgba (
    cr, 0.1, 0.1, 0.1, 1.0);
  cairo_set_line_width (cr, 1.7);
  cairo_rectangle (
    cr, x, 0,
    x + width_without_padding, height);
  cairo_stroke(cr);

  /* draw meter line */
  cairo_set_source_rgba (cr, 0.4, 0.1, 0.05, 1);
  cairo_set_line_width (cr, 1.0);
  cairo_move_to (
    cr, x, (float) height - value_px);
  cairo_line_to (
    cr, x * 2 + width_without_padding,
    (float) height - value_px);
  cairo_stroke (cr);

  /* draw peak */
  peak *= height;
  cairo_set_source_rgba (
    cr, 0.6, 0.1, 0.05, 1);
  cairo_set_line_width (cr, 2.0);
  cairo_move_to (
    cr, x, (float) height - peak);
  cairo_line_to (
    cr, x * 2 + width_without_padding,
    (float) height - peak);
  cairo_stroke (cr);

  return FALSE;
}


static void
on_crossing (GtkWidget * widget, GdkEvent *event)
{
  MeterWidget * self = Z_METER_WIDGET (widget);
  GdkEventType type =
    gdk_event_get_event_type (event);
  if (type == GDK_ENTER_NOTIFY)
    {
      self->hover = 1;
    }
  else if (type == GDK_LEAVE_NOTIFY)
    {
        self->hover = 0;
    }
  gtk_widget_queue_draw(widget);
}

static gboolean
tick_cb (
  GtkWidget * widget,
  GdkFrameClock * frame_clock,
  MeterWidget * self)
{
  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

/**
 * Creates a new Meter widget and binds it to the
 * given value.
 *
 * @param port Port this meter is for.
 */
void
meter_widget_setup (
  MeterWidget *      self,
  Port *             port,
  int                width)
{
  self->meter = meter_new_for_port (port);
  self->padding = 2;

  /* set size */
  gtk_widget_set_size_request (
    GTK_WIDGET (self), width, -1);

  gtk_widget_add_tick_callback (
    GTK_WIDGET (self), (GtkTickCallback) tick_cb,
    self, NULL);
}

static void
finalize (
  MeterWidget * self)
{
  if (self->meter)
    meter_free (self->meter);

  G_OBJECT_CLASS (
    meter_widget_parent_class)->
      finalize (G_OBJECT (self));
}

static void
meter_widget_init (MeterWidget * self)
{
  /*gdk_rgba_parse (&self->start_color, "#00FF66");*/
  /*gdk_rgba_parse (&self->end_color, "#00FFCC");*/
  gdk_rgba_parse (&self->start_color, "#F9CA1B");
  gdk_rgba_parse (&self->end_color, "#1DDD6A");

  /* connect signals */
  g_signal_connect (
    G_OBJECT (self), "draw",
    G_CALLBACK (meter_draw_cb), self);
  g_signal_connect (
    G_OBJECT (self), "enter-notify-event",
    G_CALLBACK (on_crossing),  self);
  g_signal_connect (
    G_OBJECT(self), "leave-notify-event",
    G_CALLBACK (on_crossing),  self);
}

static void
meter_widget_class_init (MeterWidgetClass * _klass)
{
  GtkWidgetClass * klass = GTK_WIDGET_CLASS (_klass);
  gtk_widget_class_set_css_name (
    klass, "meter");

  GObjectClass * oklass = G_OBJECT_CLASS (_klass);
  oklass->finalize = (GObjectFinalizeFunc) finalize;
}
