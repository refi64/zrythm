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

#include <stdlib.h>

#include "utils/string.h"

#include <glib.h>

static void
test_get_int_after_last_space ()
{
  const char * strs[] = {
    "helloЧитатьハロー・ワールド 1",
    "testハロー・ワールドest 22",
    "testハロー22",
    "",
    "testハロー 34 56",
  };

  int ret =
    string_get_int_after_last_space (
      strs[0], NULL);
  g_assert_cmpint (ret, ==, 1);
  ret =
    string_get_int_after_last_space (
      strs[1], NULL);
  g_assert_cmpint (ret, ==, 22);
  ret =
    string_get_int_after_last_space (
      strs[2], NULL);
  g_assert_cmpint (ret, ==, -1);
  ret =
    string_get_int_after_last_space (
      strs[4], NULL);
  g_assert_cmpint (ret, ==, 56);
}

static void
test_contains_substr ()
{
  const char * strs[] = {
    "helloЧитатьハロー・ワールド 1",
    "testハロー・ワールドABc 22",
    "testハロー22",
    "",
    "testハロー 34 56",
  };

  g_assert_true (
    string_contains_substr (strs[0], "Читать"));
  g_assert_true (
    string_contains_substr_case_insensitive (
      strs[1], "abc"));
  g_assert_false (
    string_contains_substr (strs[0], "Чатитать"));
  g_assert_false (
    string_contains_substr_case_insensitive (
      strs[1], "abd"));
}

static void
test_is_equal ()
{
  g_assert_true (
    string_is_equal ("", ""));
  g_assert_true (
    string_is_equal (NULL, NULL));
  g_assert_true (
    string_is_equal ("abc", "abc"));
  g_assert_false (
    string_is_equal ("abc", "aabc"));
  g_assert_false (
    string_is_equal ("", "aabc"));
  g_assert_false (
    string_is_equal (NULL, ""));
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

#define TEST_PREFIX "/utils/general/"

  g_test_add_func (
    TEST_PREFIX "test get int after last space",
    (GTestFunc) test_get_int_after_last_space);
  g_test_add_func (
    TEST_PREFIX "test contains substr",
    (GTestFunc) test_contains_substr);
  g_test_add_func (
    TEST_PREFIX "test is equal",
    (GTestFunc) test_is_equal);

  return g_test_run ();
}

