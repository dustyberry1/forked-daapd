/*
 * Copyright (C) 2019 Christian Meffert <christian.meffert@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "settings.h"

#include <stdbool.h>
#include <string.h>

#include "db.h"
#include "conffile.h"

// Forward - setting initializers
static bool artwork_spotify_default_getbool(struct settings_option *option);
static bool artwork_discogs_default_getbool(struct settings_option *option);
static bool artwork_coverartarchive_default_getbool(struct settings_option *option);

static struct settings_option webinterface_options[] =
  {
      { "show_composer_now_playing", SETTINGS_TYPE_BOOL },
      { "show_composer_for_genre", SETTINGS_TYPE_STR },
  };

static struct settings_option artwork_options[] =
  {
      { "use_artwork_source_spotify", SETTINGS_TYPE_BOOL, NULL, artwork_spotify_default_getbool, NULL },
      { "use_artwork_source_discogs", SETTINGS_TYPE_BOOL, NULL, artwork_discogs_default_getbool, NULL },
      { "use_artwork_source_coverartarchive", SETTINGS_TYPE_BOOL, NULL, artwork_coverartarchive_default_getbool, NULL },
  };

static struct settings_category categories[] =
  {
      { "webinterface", webinterface_options, ARRAY_SIZE(webinterface_options) },
      { "artwork", artwork_options, ARRAY_SIZE(artwork_options) },
  };


/* ---------------------------- DEFAULT SETTERS ------------------------------*/

static bool
artwork_default_getbool(bool no_cfg_default, const char *cfg_name)
{
  cfg_t *lib = cfg_getsec(cfg, "library");
  const char *name;
  int n_cfg;
  int i;

  n_cfg = cfg_size(lib, "artwork_online_sources");
  if (n_cfg == 0)
    return no_cfg_default;

  for (i = 0; i < n_cfg; i++)
    {
      name = cfg_getnstr(lib, "artwork_online_sources", i);
      if (strcasecmp(name, cfg_name) == 0)
        return true;
    }

  return false;
}

static bool
artwork_spotify_default_getbool(struct settings_option *option)
{
  // Enabled by default, it will only work for premium users anyway. So Spotify
  // probably won't mind, and the user probably also won't mind that we share
  // data with Spotify, since he is already doing it.
  return artwork_default_getbool(true, "spotify");
}

static bool
artwork_discogs_default_getbool(struct settings_option *option)
{
  return artwork_default_getbool(false, "discogs");
}

static bool
artwork_coverartarchive_default_getbool(struct settings_option *option)
{
  return artwork_default_getbool(false, "coverartarchive");
}


/* ------------------------------ IMPLEMENTATION -----------------------------*/

int
settings_categories_count()
{
  return ARRAY_SIZE(categories);
}

struct settings_category *
settings_category_get_byindex(int index)
{
  if (index < 0 || settings_categories_count() <= index)
    return NULL;
  return &categories[index];
}

struct settings_category *
settings_category_get(const char *name)
{
  int i;

  for (i = 0; i < settings_categories_count(); i++)
    {
      if (strcasecmp(name, categories[i].name) == 0)
	return &categories[i];
    }

  return NULL;
}

int
settings_option_count(struct settings_category *category)
{
  return category->count_options;
}

struct settings_option *
settings_option_get_byindex(struct settings_category *category, int index)
{
  if (index < 0 || !category || category->count_options <= index)
    return NULL;

  return &category->options[index];
}

struct settings_option *
settings_option_get(struct settings_category *category, const char *name)
{
  int i;

  if (!category || !name)
    return NULL;

  for (i = 0; i < category->count_options; i++)
    {
      if (strcasecmp(name, category->options[i].name) == 0)
	return &category->options[i];
    }

  return NULL;
}


int
settings_option_getint(struct settings_option *option)
{
  int intval = 0;
  int ret;

  if (!option || option->type != SETTINGS_TYPE_INT)
    return 0;

  ret = db_admin_getint(&intval, option->name);
  if (ret == 0)
    return intval;

  if (option->default_getint)
    return option->default_getint(option);

  return 0;
}

bool
settings_option_getbool(struct settings_option *option)
{
  int intval = 0;
  int ret;

  if (!option || option->type != SETTINGS_TYPE_BOOL)
    return false;

  ret = db_admin_getint(&intval, option->name);
  if (ret == 0)
    return (intval != 0);

  if (option->default_getbool)
    return option->default_getbool(option);

  return false;
}

char *
settings_option_getstr(struct settings_option *option)
{
  char *s = NULL;
  int ret;

  if (!option || option->type != SETTINGS_TYPE_STR)
    return NULL;

  ret = db_admin_get(&s, option->name);
  if (ret == 0)
    return s;

  if (option->default_getstr)
    return option->default_getstr(option);

  return NULL;
}

int
settings_option_setint(struct settings_option *option, int value)
{
  if (!option || option->type != SETTINGS_TYPE_INT)
    return -1;

  return db_admin_setint(option->name, value);
}

int
settings_option_setbool(struct settings_option *option, bool value)
{
  if (!option || option->type != SETTINGS_TYPE_BOOL)
    return -1;

  return db_admin_setint(option->name, value);
}

int
settings_option_setstr(struct settings_option *option, const char *value)
{
  if (!option || option->type != SETTINGS_TYPE_STR)
    return -1;

  return db_admin_set(option->name, value);
}
