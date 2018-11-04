/*
 * Copyright (C) 2018 - Felix Schmidt
 *
 * This file is free software; you can redistribute it and/or
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef _countersh_
#define _countersh_


/*! \ingroup counters */
/*! @{ */

#define CNT_SLOTS	4

#define MAX_NAME_LEN	64

/*! MIB/Counter descriptor */
struct ath_counter_desc
{
    /*! Counter name */
    const char	name[MAX_NAME_LEN];

    /*! Offset from counter base address */
    unsigned	off;

    /*! Counter size in bytes (4 or 8) */
    unsigned	sz;
};

/*! Counter history */
struct ath_counter_state
{
    /*! Last known total counter */
    uint64_t	sum[4];

    /*! Last read time in usec since epoch (0: has not yet been read) */
    uint64_t	lastReadTime[4];

    /*! Maximum rate */
    uint64_t    max_rate_per_sec;
};

enum prtg_mode
{
    PRTG_OFF = 0,
    PRTG_ABS = 1,
    PRTG_REL = 2,
    PRTG_RATE = 3,
    PRTG_MAXRATE = 4,
    PRTG_RATE_AND_MAX = 5
};


extern uint64_t ullTime(void);
char *fmt1000 (uint64_t val, char *str, size_t slen);
extern void cntShow (char *port_id, uint64_t val, uint64_t *psum, const char *name, 
                     int cor, int dtime, int showAll, uint64_t *max_rate);
extern enum prtg_mode prtg_mode(void);
extern void set_prtg_mode(enum prtg_mode mode);

extern int netdev_counters (int port, const char *filter, int all, int slot, int reset);
extern int pp_counters (int port, const char *filter, int all, int slot, int reset);
extern int psw_counters (int port, const char *filter, int all, int slot, int reset);
extern int ath_counters (void *devp, int port, const char *filter, int all, int slot, int reset);
extern void *get_shm(unsigned int key, size_t size, int *cnt_initialized);
extern void *ath_st_alloc(void);

/*! @} */
#endif
