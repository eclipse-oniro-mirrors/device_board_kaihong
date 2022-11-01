/*
 * Broadcom Secure Standard Library.
 *
 * Copyright (C) 1999-2019, Broadcom.
 *
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 *
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions
 * of the license of that module.  An independent module is a module which is
 * not derived from this software.  The special exception does not apply to any
 * modifications of the software.
 *
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else /* BCMDRIVER */
#include <stddef.h>
#include <string.h>
#endif /* else BCMDRIVER */

#include <bcmstdlib_s.h>
#include <bcmutils.h>

/*
 * __SIZE_MAX__ value is depending on platform:
 * Firmware Dongle: RAMSIZE (Dongle Specific Limit).
 * LINUX NIC/Windows/MACOSX/Application: OS Native or
 * 0xFFFFFFFFu if not defined.
 */
#ifndef SIZE_MAX
#ifndef __SIZE_MAX__
#define __SIZE_MAX__ 0xFFFFFFFFu
#endif /* __SIZE_MAX__ */
#define SIZE_MAX __SIZE_MAX__
#endif /* SIZE_MAX */
#define RSIZE_MAX (SIZE_MAX >> 1u)

/**
 * strlcat_s - Concatenate a %NUL terminated string with a sized buffer
 * @dest: Where to concatenate the string to
 * @src: Where to copy the string from
 * @size: size of destination buffer
 * return: string length of created string (i.e. the initial length of dest plus
 * the length of src) not including the NUL char, up until size
 *
 * Unlike strncat(), strlcat() take the full size of the buffer (not just the
 * number of bytes to copy) and guarantee to NUL-terminate the result (even when
 * there's nothing to concat). If the length of dest string concatinated with
 * the src string >= size, truncation occurs.
 *
 * Compatible with *BSD: the result is always a valid NUL-terminated string that
 * fits in the buffer (unless, of course, the buffer size is zero).
 *
 * If either src or dest is not NUL-terminated, dest[size-1] will be set to NUL.
 * If size < strlen(dest) + strlen(src), dest[size-1] will be set to NUL.
 * If size == 0, dest[0] will be set to NUL.
 */
size_t strlcat_s(char *dest, const char *src, size_t size)
{
    char *d = dest;
    const char *s = src; /* point to the start of the src string */
    size_t n = size;
    size_t dlen;
    size_t bytes_to_copy = 0;

    if (dest == NULL) {
        return 0;
    }

    /* set d to point to the end of dest string (up to size) */
    while (n != 0 && *d != '\0') {
        d++;
        n--;
    }
    dlen = (size_t)(d - dest);

    if (s != NULL) {
        size_t slen = 0;

        /* calculate src len in case it's not null-terminated */
        n = size;
        while (n-- != 0 && *(s + slen) != '\0') {
            ++slen;
        }

        n = size - dlen; /* maximum num of chars to copy */
        if (n != 0) {
            /* copy relevant chars (until end of src buf or given size is
             * reached) */
            bytes_to_copy = MIN(slen - (size_t)(s - src), n - 1);
            (void)memcpy(d, s, bytes_to_copy);
            d += bytes_to_copy;
        }
    }
    if (n == 0 && dlen != 0) {
        --d; /* nothing to copy, but NUL-terminate dest anyway */
    }
    *d = '\0'; /* NUL-terminate dest */

    return (dlen + bytes_to_copy);
}
