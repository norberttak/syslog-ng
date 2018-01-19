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

#include <sys/mman.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "messages.h"
#include "nondumpable-allocator.h"

#define ALLOCATION_HEADER_SIZE offsetof(Allocation, user_data)
#define BUFFER_TO_ALLOCATION(buffer) (buffer - ALLOCATION_HEADER_SIZE)

typedef struct
{
  gsize alloc_size;
  gsize data_len;
  guint8 user_data[];
} Allocation;

static gpointer
_mmap(gsize len)
{
  gpointer area = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
  if (!area)
    {
      msg_fatal("secret storage: cannot mmap buffer",
                evt_tag_int("len", len),
                evt_tag_errno("errno", errno));
      return NULL;
    }

  if (madvise(area, len, MADV_DONTDUMP) < 0)
    {
      msg_fatal("secret storage: cannot madvisebuffer",
                evt_tag_errno("errno", errno));
      goto err_munmap;
    }

  if (mlock(area, len) < 0)
    {
      msg_fatal("secret storage: cannot lock buffer",
                evt_tag_int("len", len),
                evt_tag_errno("errno", errno));
      goto err_munmap;
    }

  return area;
err_munmap:
  munmap(area, len);
  return NULL;
}

static gsize
round_to_nearest(gsize number, gsize base)
{
  return number + (base - (number % base));
}

gpointer
nondumpable_buffer_alloc(gsize len)
{
  gsize minimum_size = len + ALLOCATION_HEADER_SIZE;
  gsize PAGESIZE = sysconf(_SC_PAGE_SIZE);
  gsize alloc_size = round_to_nearest(minimum_size, PAGESIZE);

  Allocation *buffer = _mmap(alloc_size);
  if (!buffer)
    return NULL;

  buffer->alloc_size = alloc_size;
  buffer->data_len = len;
  return buffer->user_data;
}

void
nondumpable_buffer_free(gpointer buffer)
{
  Allocation *allocation = BUFFER_TO_ALLOCATION(buffer);
  memset(allocation->user_data, 0, allocation->data_len);
  munmap(allocation, allocation->alloc_size);
}

gpointer
nondumpable_buffer_realloc(gpointer buffer, gsize len)
{
  Allocation *allocation = BUFFER_TO_ALLOCATION(buffer);
  if (allocation->alloc_size >= len + ALLOCATION_HEADER_SIZE)
    {
      allocation->data_len = len;
      return allocation->user_data;
    }

  gpointer new_buffer = nondumpable_buffer_alloc(len);
  memmove(new_buffer, allocation->user_data, allocation->data_len);
  nondumpable_buffer_free(buffer);
  return new_buffer;
}