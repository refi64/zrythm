/*
 * Copyright (C) 2020 Alexandros Theodotou <alex at zrythm dot org>
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

#include "zrythm-test-config.h"

#include "actions/range_action.h"
#include "actions/undoable_action.h"
#include "actions/undo_manager.h"
#include "audio/audio_region.h"
#include "audio/automation_region.h"
#include "audio/chord_region.h"
#include "audio/control_port.h"
#include "audio/master_track.h"
#include "audio/midi_note.h"
#include "audio/region.h"
#include "project.h"
#include "utils/flags.h"
#include "zrythm.h"

#include "tests/helpers/project.h"

#include <glib.h>

#define PLAYHEAD_BEFORE 7
#define LOOP_START_BEFORE 1
#define LOOP_END_BEFORE 6

#define RANGE_START_BAR 3
#define RANGE_END_BAR 5
#define RANGE_SIZE_IN_BARS \
  (RANGE_END_BAR - RANGE_START_BAR)

/* audio region starts before the range start and
 * ends in the middle of the range */
#define AUDIO_REGION_START_BAR \
  (RANGE_START_BAR - 1)
#define AUDIO_REGION_END_BAR \
  (RANGE_START_BAR + 1)

/* midi region starts after the range end */
#define MIDI_REGION_START_BAR \
  (RANGE_START_BAR + 2)
#define MIDI_REGION_END_BAR \
  (MIDI_REGION_START_BAR + 2)

static int midi_track_pos = -1;
static int audio_track_pos = -1;

static void
test_prepare_common (void)
{
  test_helper_zrythm_init ();

  /* create MIDI track with region */
  UndoableAction * ua =
    create_tracks_action_new (
      TRACK_TYPE_MIDI, NULL, NULL,
      TRACKLIST->num_tracks, NULL, 1);
  undo_manager_perform (UNDO_MANAGER, ua);
  midi_track_pos = TRACKLIST->num_tracks - 1;
  Track * midi_track =
    TRACKLIST->tracks[midi_track_pos];

  Position start, end;
  position_set_to_bar (
    &start, MIDI_REGION_START_BAR);
  position_set_to_bar (
    &end, MIDI_REGION_END_BAR);
  ZRegion * midi_region =
    midi_region_new (
      &start, &end, midi_track_pos, 0, 0);
  track_add_region (
    midi_track, midi_region, NULL, 0, F_GEN_NAME,
    F_NO_PUBLISH_EVENTS);
  arranger_object_select (
    (ArrangerObject *) midi_region, F_SELECT,
    F_NO_APPEND);
  ua =
    arranger_selections_action_new_create (
      (ArrangerSelections *) TL_SELECTIONS);
  undo_manager_perform (UNDO_MANAGER, ua);

  /* add another region to test problems with
   * indices on undo */
  position_add_bars (&start, 10);
  position_add_bars (&end, 10);
  midi_region =
    midi_region_new (
      &start, &end, midi_track_pos, 0, 0);
  track_add_region (
    midi_track, midi_region, NULL, 0, F_GEN_NAME,
    F_NO_PUBLISH_EVENTS);
  arranger_object_select (
    (ArrangerObject *) midi_region, F_SELECT,
    F_NO_APPEND);
  ua =
    arranger_selections_action_new_create (
      (ArrangerSelections *) TL_SELECTIONS);
  undo_manager_perform (UNDO_MANAGER, ua);

  /* create audio track with region */
  char * filepath =
    g_build_filename (
      TESTS_SRCDIR, "test.wav", NULL);
  SupportedFile * file =
    supported_file_new_from_path (filepath);
  g_free (filepath);
  position_set_to_bar (
    &start, AUDIO_REGION_START_BAR);
  position_set_to_bar (
    &end, AUDIO_REGION_END_BAR);
  UndoableAction * action =
    create_tracks_action_new (
      TRACK_TYPE_AUDIO, NULL, file,
      TRACKLIST->num_tracks, &start, 1);
  undo_manager_perform (UNDO_MANAGER, action);
  audio_track_pos = TRACKLIST->num_tracks - 1;
  Track * audio_track =
    TRACKLIST->tracks[audio_track_pos];
  ZRegion * audio_region =
    audio_track->lanes[0]->regions[0];

  /* resize audio region */
  arranger_object_select (
    (ArrangerObject *) audio_region, F_SELECT,
    F_NO_APPEND);
  double audio_region_size_ticks =
    arranger_object_get_length_in_ticks (
      (ArrangerObject *) audio_region);
  double missing_ticks  =
    (end.total_ticks - start.total_ticks) -
    audio_region_size_ticks;
  arranger_object_resize (
    (ArrangerObject *) audio_region, false,
    ARRANGER_OBJECT_RESIZE_LOOP, missing_ticks,
    false);
  ua =
    arranger_selections_action_new_resize (
      (ArrangerSelections *) TL_SELECTIONS,
      ARRANGER_SELECTIONS_ACTION_RESIZE_R_LOOP,
      missing_ticks);
  undo_manager_perform (UNDO_MANAGER, ua);
  g_assert_cmppos (
    &end, &audio_region->base.end_pos);

  /* set transport positions */
  position_set_to_bar (
    &TRANSPORT->playhead_pos, PLAYHEAD_BEFORE);
  position_set_to_bar (
    &TRANSPORT->loop_start_pos, LOOP_START_BEFORE);
  position_set_to_bar (
    &TRANSPORT->loop_end_pos, LOOP_END_BEFORE);
}

static void
check_before_insert ()
{
  Track * midi_track =
    TRACKLIST->tracks[midi_track_pos];
  ZRegion * midi_region =
    midi_track->lanes[0]->regions[0];
  g_assert_cmpint (
    midi_track->lanes[0]->num_regions, ==, 2);
  ArrangerObject * midi_region_obj =
    (ArrangerObject *) midi_region;

  Track * audio_track =
    TRACKLIST->tracks[audio_track_pos];
  ZRegion * audio_region =
    audio_track->lanes[0]->regions[0];
  g_assert_cmpint (
    audio_track->lanes[0]->num_regions, ==, 1);
  ArrangerObject * audio_region_obj =
    (ArrangerObject *) audio_region;

  Position start, end;
  position_set_to_bar (
    &start, MIDI_REGION_START_BAR);
  position_set_to_bar (
    &end, MIDI_REGION_END_BAR);
  g_assert_cmppos (&midi_region_obj->pos, &start);
  g_assert_cmppos (&midi_region_obj->end_pos, &end);
  position_set_to_bar (
    &start, AUDIO_REGION_START_BAR);
  position_set_to_bar (
    &end, AUDIO_REGION_END_BAR);
  g_assert_cmppos (&audio_region_obj->pos, &start);
  g_assert_cmppos (&audio_region_obj->end_pos, &end);
}

static void
check_after_insert (void)
{
  /* get objects */
  Track * midi_track =
    TRACKLIST->tracks[midi_track_pos];
  ZRegion * midi_region =
    midi_track->lanes[0]->regions[0];
  ArrangerObject * midi_region_obj =
    (ArrangerObject *) midi_region;

  /* get new audio regions */
  Track * audio_track =
    TRACKLIST->tracks[audio_track_pos];
  g_assert_cmpint (
    audio_track->lanes[0]->num_regions, ==, 2);
  ZRegion * audio_region1 =
    audio_track->lanes[0]->regions[0];
  ZRegion * audio_region2 =
    audio_track->lanes[0]->regions[1];
  ArrangerObject * audio_region_obj1 =
    (ArrangerObject *) audio_region1;
  ArrangerObject * audio_region_obj2 =
    (ArrangerObject *) audio_region2;

  /* check midi region positions */
  Position midi_region_start_after_expected,
           midi_region_end_after_expected;
  position_set_to_bar (
    &midi_region_start_after_expected,
    MIDI_REGION_START_BAR + RANGE_SIZE_IN_BARS);
  position_set_to_bar (
    &midi_region_end_after_expected,
    MIDI_REGION_END_BAR + RANGE_SIZE_IN_BARS);
  g_assert_cmppos (
    &midi_region_obj->pos,
    &midi_region_start_after_expected);
  g_assert_cmppos (
    &midi_region_obj->end_pos,
    &midi_region_end_after_expected);

  /* check audio region positions */
  Position audio_region1_end_after_expected,
           audio_region2_start_after_expected,
           audio_region2_end_after_expected;
  position_set_to_bar (
    &audio_region1_end_after_expected,
    RANGE_START_BAR);
  position_set_to_bar (
    &audio_region2_start_after_expected,
    RANGE_END_BAR);
  position_set_to_bar (
    &audio_region2_end_after_expected,
    AUDIO_REGION_END_BAR + RANGE_SIZE_IN_BARS);
  g_assert_cmppos (
    &audio_region_obj1->end_pos,
    &audio_region1_end_after_expected);
  g_assert_cmppos (
    &audio_region_obj2->pos,
    &audio_region2_start_after_expected);
  g_assert_cmppos (
    &audio_region_obj2->end_pos,
    &audio_region2_end_after_expected);

  /* get expected transport positions */
  Position playhead_after_expected,
           loop_end_after_expected;
  position_set_to_bar (
    &playhead_after_expected,
    PLAYHEAD_BEFORE + RANGE_SIZE_IN_BARS);
  position_set_to_bar (
    &loop_end_after_expected,
    LOOP_END_BEFORE + RANGE_SIZE_IN_BARS);

  /* check that they match with actual positions */
  g_assert_cmppos (
    &playhead_after_expected,
    &TRANSPORT->playhead_pos);
  g_assert_cmppos (
    &loop_end_after_expected,
    &TRANSPORT->loop_end_pos);
}

static void
test_insert_silence (void)
{
  test_prepare_common ();

  /* create inset silence action */
  Position start, end;
  position_set_to_bar (
    &start, RANGE_START_BAR);
  position_set_to_bar (
    &end, RANGE_END_BAR);
  UndoableAction * ua =
    range_action_new_insert_silence (&start, &end);

  /* verify that number of objects is as expected */
  RangeAction * ra = (RangeAction *) ua;
  g_assert_cmpint (
    arranger_selections_get_num_objects (
      (ArrangerSelections *) ra->sel_before), ==, 3);

  check_before_insert ();

  /* perform action */
  undo_manager_perform (UNDO_MANAGER, ua);

  check_after_insert ();

  /* undo and verify things are back to previous
   * state */
  undo_manager_undo (UNDO_MANAGER);

  check_before_insert ();

  undo_manager_redo (UNDO_MANAGER);

  check_after_insert ();

  test_helper_zrythm_cleanup ();
}

static void
check_after_remove (void)
{
  /* get objects */
  Track * midi_track =
    TRACKLIST->tracks[midi_track_pos];
  ZRegion * midi_region =
    midi_track->lanes[0]->regions[0];
  ArrangerObject * midi_region_obj =
    (ArrangerObject *) midi_region;

  /* get new audio regions */
  Track * audio_track =
    TRACKLIST->tracks[audio_track_pos];
  g_assert_cmpint (
    audio_track->lanes[0]->num_regions, ==, 2);
  ZRegion * audio_region1 =
    audio_track->lanes[0]->regions[0];
  ZRegion * audio_region2 =
    audio_track->lanes[0]->regions[1];
  ArrangerObject * audio_region_obj1 =
    (ArrangerObject *) audio_region1;
  ArrangerObject * audio_region_obj2 =
    (ArrangerObject *) audio_region2;

  /* check midi region positions */
  Position midi_region_start_after_expected,
           midi_region_end_after_expected;
  position_set_to_bar (
    &midi_region_start_after_expected,
    MIDI_REGION_START_BAR + RANGE_SIZE_IN_BARS);
  position_set_to_bar (
    &midi_region_end_after_expected,
    MIDI_REGION_END_BAR + RANGE_SIZE_IN_BARS);
  g_assert_cmppos (
    &midi_region_obj->pos,
    &midi_region_start_after_expected);
  g_assert_cmppos (
    &midi_region_obj->end_pos,
    &midi_region_end_after_expected);

  /* check audio region positions */
  Position audio_region1_end_after_expected,
           audio_region2_start_after_expected,
           audio_region2_end_after_expected;
  position_set_to_bar (
    &audio_region1_end_after_expected,
    RANGE_START_BAR);
  position_set_to_bar (
    &audio_region2_start_after_expected,
    RANGE_END_BAR);
  position_set_to_bar (
    &audio_region2_end_after_expected,
    AUDIO_REGION_END_BAR + RANGE_SIZE_IN_BARS);
  g_assert_cmppos (
    &audio_region_obj1->end_pos,
    &audio_region1_end_after_expected);
  g_assert_cmppos (
    &audio_region_obj2->pos,
    &audio_region2_start_after_expected);
  g_assert_cmppos (
    &audio_region_obj2->end_pos,
    &audio_region2_end_after_expected);

  /* get expected transport positions */
  Position playhead_after_expected,
           loop_end_after_expected;
  position_set_to_bar (
    &playhead_after_expected,
    PLAYHEAD_BEFORE - RANGE_SIZE_IN_BARS);
  position_set_to_bar (
    &loop_end_after_expected,
    LOOP_END_BEFORE - RANGE_SIZE_IN_BARS);

  /* check that they match with actual positions */
  g_assert_cmppos (
    &playhead_after_expected,
    &TRANSPORT->playhead_pos);
  g_assert_cmppos (
    &loop_end_after_expected,
    &TRANSPORT->loop_end_pos);
}

static void
test_remove_range (void)
{
  test_prepare_common ();

  /* create remove range action */
  Position start, end;
  position_set_to_bar (
    &start, RANGE_START_BAR);
  position_set_to_bar (
    &end, RANGE_END_BAR);
  UndoableAction * ua =
    range_action_new_remove (&start, &end);

  /* verify that number of objects is as expected */
  RangeAction * ra = (RangeAction *) ua;
  g_assert_cmpint (
    arranger_selections_get_num_objects (
      (ArrangerSelections *) ra->sel_before), ==, 3);

  check_before_insert ();

  /* perform action */
  undo_manager_perform (UNDO_MANAGER, ua);

  check_after_remove ();

  /* undo and verify things are back to previous
   * state */
  undo_manager_undo (UNDO_MANAGER);

  check_before_insert ();

  undo_manager_redo (UNDO_MANAGER);

  check_after_insert ();

  test_helper_zrythm_cleanup ();
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

#define TEST_PREFIX "/actions/range/"

  g_test_add_func (
    TEST_PREFIX "test insert silence",
    (GTestFunc) test_insert_silence);
  g_test_add_func (
    TEST_PREFIX "test remove range",
    (GTestFunc) test_remove_range);

  return g_test_run ();
}
