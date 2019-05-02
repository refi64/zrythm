/*
 * Copyright (C) 2019 Alexandros Theodotou <alex at zrythm dot org>
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

#ifndef __UNDO_COPY_TRACKS_ACTION_H__
#define __UNDO_COPY_TRACKS_ACTION_H__

#include "actions/undoable_action.h"

typedef struct TracklistSelections
  TracklistSelections;

typedef struct CopyTracksAction
{
  UndoableAction        parent_instance;

  /**
   * A clone of the timeline selections at the time.
   */
  TracklistSelections * tls;

  /** Position to copy to. */
  int                   pos;
} CopyTracksAction;

UndoableAction *
copy_tracks_action_new (
  TracklistSelections * tls,
  int                   pos);

int
copy_tracks_action_do (
  CopyTracksAction * self);

int
copy_tracks_action_undo (
  CopyTracksAction * self);

char *
copy_tracks_action_stringize (
  CopyTracksAction * self);

void
copy_tracks_action_free (
  CopyTracksAction * self);

#endif
