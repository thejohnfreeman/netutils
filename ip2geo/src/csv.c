#include <assert.h>
#include <string.h> // strlen
#include <stdlib.h> // malloc
#include <err.h>
#include <errno.h>

#include "io.h"
#include "csv.h"

#define INITIAL_BUFFER_SIZE 64

void csv_ctor(struct csv* csv) {
  csv->buffer = (char*)malloc(INITIAL_BUFFER_SIZE);
  if (!csv->buffer) {
    err(errno, "abort: malloc");
  }
  csv->size  = INITIAL_BUFFER_SIZE;
}

const char* csv_next_row(struct csv* csv, FILE* file) {
  size_t start = 0;

  while (1) {
    /* Space left in the buffer (after what we've already read into it). */
    size_t nspace = csv->size - start;

    /* Try to get to an end-line, EOF, or end-of-buffer. */
    if (!fgets(csv->buffer + start, nspace, file) && ferror(file)) {
      err(errno, "abort: fgets");
    }

    /* Gives us the number of non-null characters from `start`. If adding one
     * (for the null terminator) equals `nspace`, then the buffer was
     * filled. */
    size_t nread = strlen(csv->buffer + start);
    /* If we hit end-of-buffer (because of a long line), resize and read some more. */
    if (nread + 1 == nspace) {
      csv->size <<= 1;
      csv->buffer = (char*)realloc(csv->buffer, csv->size);
      start += nread;
    } else {
      break;
    }
  }

  csv->it = (*csv->buffer == '\0') ? (NULL) : (csv->buffer);

  return csv->buffer;
}

const char* csv_next_field(struct csv* csv) {
  char* begin = csv->it;

  /* Return NULL if there are no fields to read. */
  if (!begin) {
    return NULL;
  }

  /* If `begin` is non-null, we know that we last left `csv` in a state where
   * we believed `begin` started a field. However, that does not mean `begin`
   * is not currently pointing at a comma, new-line, or null character. If it
   * is, that means we have an empty field. */

  /* We're going to erminate the current field by replacing a character with
   * NUL. */
  char* end = begin;

  /* Fields usually end at the next comma, new-line, or null. */
  /* TODO: Support other delimiters. Maybe give a function that can infer the
   * delimiter from the headers (i.e., first line of the file). */
  const char* stoppers = ",\n";

  /* If the field begins with a quote, it ends with the next unescaped quote.
   * If we encounter a null before then, it's an error; be forgiving. */
  if (*begin == '"') {
    stoppers = "\"";
    ++begin;
    ++end;
  }

  /* Wanted to use `strpbrk` here, but it won't return the end of the string,
   * and it won't let us search for the null terminator. Wrote my own
   * `nextof` instead. */

  do {
    end = nextof(end, stoppers);
  } while ((*end == '"') && (*(end - 1) == '\\') && ++end);

  /* Unconditionally terminate the field, but remember the character.  */
  char c = *end;
  *end = '\0';

  /* If we reached a closing quote, move to the next non-space character. If
   * we reached the end of the line, then set the iterator to indicate no
   * more fields. Else, incremement it expecting to see another (possibly
   * empty) field. */
  if (c == '"') {
    c = *++end;
  }
  csv->it = ((c == '\n') || (c == '\0')) ? NULL : (end + 1);

  return begin;
}

/* Binary search to find `key` in the rows of `file` according to `comp`.  */
const char* csv_bsearch(struct csv* csv, FILE* file, const void* key,
    int (*comp)(const void* key, const char* row))
{
  int error = fseek(file, 0, SEEK_END);
  if (error) {
    err(errno, "abort: fseek with SEEK_END");
  }

  long fsize = ftell(file);
  if (fsize < 0) {
    err(errno, "abort: ftell for file size");
  }

  long low  = 0;
  long high = fsize;
  long pos  = 0;
  while (1) {
    assert(low < high);

    /* Seek up or down. */
    long mid = low + ((high - low) >> 1);
    error = fseek(file, mid, SEEK_SET);
    if (error) {
      err(errno, "abort: fseek with SEEK_SET");
    }

    /* Find the row I'm in. */
    frseekln(file);

    /* Make sure it's not the same as the last row. */
    long pos_next = ftell(file);
    if (pos_next == pos) {
      puts("key not found in database");
      return NULL;
    }
    pos = pos_next;

    /* Parse the row. */
    const char* row = csv_next_row(csv, file);
    int branch = comp(key, row);
    if (branch < 0) {
      high = mid;
    } else if (branch > 0) {
      low = mid;
    } else {
      return row;
    }
  }
}

void csv_dtor(struct csv* csv) {
  free(csv->buffer);
  csv->buffer = NULL;
  csv->size   = 0;
  csv->it     = NULL;
}

