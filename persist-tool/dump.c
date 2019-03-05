/*
 * Copyright (c) 2019 Balabit
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "dump.h"
#include "persistable-state-header.h"

static GKeyFile *keyfile = NULL;

static void
print_struct_ini_default_style(const gchar *name, gpointer block, gsize block_size)
{
  PersistableStateHeader *header = (PersistableStateHeader *) block;

  persist_state_dump_header(keyfile, name, header);

  gsize value_list_size = block_size - sizeof(PersistableStateHeader);
  gint list[value_list_size];
  gchar *block_data = (gchar *) ((gchar *)block + sizeof(PersistableStateHeader));
  for (gsize i=0; i<value_list_size; i++)
    {
      list[i] = block_data[i];
    }

  g_key_file_set_integer_list (keyfile, name, "value", list, value_list_size);
}

static void
print_struct_ini_style(gpointer data, gpointer user_data)
{
  PersistEntryHandle handle;
  gsize size;
  guint8 result_version;

  PersistTool *self = (PersistTool *)user_data;
  const gchar *name = (const gchar *)data;

  if (!(handle = persist_state_lookup_entry(self->state, name, &size, &result_version)))
    {
      fprintf(stderr,"Can't lookup for entry \"%s\"\n", name);
      return;
    }

  gpointer block = persist_state_map_entry(self->state, handle);

  PersistStateDumpFunc dump_func = persist_state_get_dump_func(name);
  if (dump_func)
    {
      dump_func(name, block, keyfile);
    }
  else
    {
      print_struct_ini_default_style(name, block, size);
    }

  persist_state_unmap_entry(self->state, handle);
}

static void
print_struct_json_style(gpointer data, gpointer user_data)
{
  PersistEntryHandle handle;
  gsize size;
  guint8 result_version;

  PersistTool *self = (PersistTool *)user_data;
  gchar *name = (gchar *)data;

  if (!(handle = persist_state_lookup_entry(self->state, name, &size, &result_version)))
    {
      fprintf(stderr,"Can't lookup for entry \"%s\"\n", name);
      return;
    }

  gpointer block = persist_state_map_entry(self->state, handle);

  fprintf(stdout,"\n%s = { \"value\": \"", name);
  gchar *block_data = (gchar *) block;
  for (gsize i=0; i<size; i++)
    {
      fprintf(stdout, "%.2X ", block_data[i]&0xff);
    }
  fprintf(stdout,"\" }\n");

  persist_state_unmap_entry(self->state, handle);
}

gint
dump_main(int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf(stderr, "Persist file is a required parameter\n");
      return 1;
    }

  if (!g_file_test(argv[1], G_FILE_TEST_IS_REGULAR | G_FILE_TEST_EXISTS))
    {
      fprintf(stderr, "Persist file doesn't exist; file = %s\n", argv[1]);
      return 1;
    }

  PersistTool *self = persist_tool_new(argv[1], persist_mode_dump);
  if (!self)
    {
      fprintf(stderr,"Error creating persist tool\n");
      return 1;
    }

  keyfile = g_key_file_new();
  GList *keys = g_hash_table_get_keys(self->state->keys);

  g_list_foreach(keys, dump_in_ini_format ? print_struct_ini_style : print_struct_json_style, self);
  g_list_free(keys);

  gchar *keyfile_str = g_key_file_to_data(keyfile, NULL, NULL);
  fprintf(stdout,"%s", keyfile_str);

  g_key_file_free(keyfile);
  g_free(keyfile_str);
  persist_tool_free(self);
  return 0;
}
