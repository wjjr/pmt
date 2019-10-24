/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../../types.h"
#include "../../log.h"

static __inline struct line find_line(const byte *const buffer, const usize buffer_size, const usize buffer_index, const usize file_offset) {
    struct line line;
    usize i;
    const byte *lf_pointer;

    for (line.beg = -1, i = buffer_index + 1; i > 0; --i)
        if (buffer[i - 1] == LF) {
            line.beg = (ssize) (file_offset - (buffer_index - i));
            break;
        }

    lf_pointer = memchr(&buffer[buffer_index], LF, buffer_size - buffer_index);
    line.end = lf_pointer ? (ssize) ((usize) (lf_pointer - &buffer[buffer_index]) + file_offset) : -1;

    return line;
}

static __inline void file_seek(const struct file *const file, ssize offset, int whence) {
    if ((ftell(file->fp) + offset) < 0)
        whence = SEEK_SET, offset = 0;

    if (fseek(file->fp, (long) offset, whence) != 0)
        die(EXIT_FAILURE, errno, "%s", file->name);
}

static __inline usize file_read(const struct file *const file, void *const buffer, const usize buffer_capacity) {
    usize bytes_read = fread(buffer, 1, buffer_capacity, file->fp);

    if (bytes_read == 0 && ferror(file->fp))
        die(EXIT_FAILURE, EIO, "%s", file->name);

    return bytes_read;
}

void print_file_line(const struct file *const file, const usize file_offset, const byte *const buffer, const usize buffer_capacity,
                     const usize buffer_size, const usize buffer_index, const bool print_byte_offset, struct line *last_line) {
    struct line line;
    fpos_t fpos;
    usize i, bytes_read, total_bytes_read;
    byte *tmp_buffer;

    if ((ssize) file_offset < last_line->end)
        return;

    fgetpos(file->fp, &fpos);
    tmp_buffer = malloc(buffer_capacity);
    line = find_line(buffer, buffer_size, buffer_index, file_offset);

    if (line.beg < 0) {
        file_seek(file, (ssize) -(buffer_size), SEEK_CUR);

        for (i = 1; line.beg < 0; ++i) {
            file_seek(file, (ssize) -(buffer_capacity * i), SEEK_CUR);

            if (ftell(file->fp) != 0) {
                bytes_read = file_read(file, tmp_buffer, buffer_capacity);

                if (bytes_read > 0)
                    line.beg = (find_line(tmp_buffer, bytes_read, bytes_read, file_offset)).beg;
                else
                    line.beg = 0;
            } else
                line.beg = 0;
        }

        fsetpos(file->fp, &fpos);
    }

    if (line.end < 0) {
        file_seek(file, (ssize) -(buffer_size), SEEK_CUR);

        for (i = 1; line.end < 0; ++i) {
            file_seek(file, (ssize) (buffer_capacity * i), SEEK_CUR);

            bytes_read = file_read(file, tmp_buffer, buffer_capacity);

            if (bytes_read > 0)
                line.end = (find_line(tmp_buffer, bytes_read, 0, file_offset)).end;
            else
                line.end = (ssize) file->size;
        }

        fsetpos(file->fp, &fpos);
    }

    if (last_line->beg == last_line->end || last_line->beg != line.beg) {
        if (print_byte_offset)
            printf("%" PRIuSIZ ":", line.beg);

        file_seek(file, line.beg, SEEK_SET);

        for (i = (usize) (line.end - line.beg), total_bytes_read = 0; total_bytes_read < i; total_bytes_read += bytes_read) {
            bytes_read = file_read(file, tmp_buffer, buffer_capacity);

            if (bytes_read > 0)
                printf("%.*s", (int) MIN(i - total_bytes_read, bytes_read), tmp_buffer);
        }

        putc(LF, stdout);

        last_line->beg = line.beg;
        last_line->end = line.end;

        fsetpos(file->fp, &fpos);
    }

    free(tmp_buffer);
}
