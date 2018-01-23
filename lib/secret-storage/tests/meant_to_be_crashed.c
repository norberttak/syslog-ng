/*
 * Copyright (c) 2018 Balabit
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

#include "nondumpable-allocator.h"
#include "secret-storage.h"

#include "lib/compat/glib.h"
#include <stdio.h>
#include <stdlib.h>

void
break_point_for_gdb()
{
}

/*
  The motivation is the secret must not appear as a literal text
  inside the source code, otherwise it would be dumped into the
  core. Instead: a transformed secret is read form argv, and
  transformed back here in the code.
*/
void copy_by_character_indexes(char *buffer, char *secret)
{
  for (int i = 0; secret[i]; i++)
    buffer[i] = secret[i] - 1;
}

int main(int argc, char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "Usage: %s <secret> (secret -> rotating with one character : alma -> bmnb)\n", argv[0]);
      exit(1);
    }

  secret_storage_init();
  char *tmp_secret = nondumpable_buffer_alloc(100);

  copy_by_character_indexes(tmp_secret, argv[1]);

  secret_storage_store_string("secret_key", tmp_secret);
  break_point_for_gdb();
}
