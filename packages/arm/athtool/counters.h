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

/*! Counter type */
enum cnt_type
{
    ANY,	/**< Match criterium */
    PKT,	/**< Packets */
    OCTET,	/**< Octets */
    BIT,	/**< Bits */
    ERR		/**< Errors */
};

struct filter
{
    const char *substr;
    enum cnt_type type;
};

struct port_id
{
    const char *name;
    int num;
};

/*! MIB/Counter descriptor */
struct ath_counter_desc
{
    /*! Counter name */
    const char	name[MAX_NAME_LEN];

    /*! Offset from counter base address */
    unsigned	off;

    /*! Counter size in bytes (4 or 8) */
    unsigned	sz;

    /*! What the counter counts */
    enum cnt_type type;
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
                     int cor, int dtime, int showAll, uint64_t *max_rate, enum cnt_type type);
extern enum prtg_mode prtg_mode(void);
extern void set_prtg_mode(enum prtg_mode mode);
extern void set_bps_mode (void);
extern void set_hr_mode (int val);
extern int match_filter(struct ath_counter_desc *desc, struct filter *filter);
extern enum cnt_type str_to_type (const char *str);
extern const char *unit_str(enum cnt_type type);
extern void make_filter(struct filter *filter, char *str);
extern int match_port(struct port_id *port_id, const char *name, int num);
extern void make_port(struct port_id *port_id, const char *name);

extern int netdev_counters (struct port_id *port, struct filter *filter, int all, int slot, int reset);
extern int pp_counters (struct filter *filter, int all, int slot, int reset);
extern int psw_counters (struct port_id *port, struct filter *filter, int all, int slot, int reset);
extern int ath_counters (void *devp, struct port_id *port, struct filter *filter, int all, int slot, int reset);
extern void *get_shm(unsigned int key, size_t size, int *cnt_initialized);
extern void *ath_st_alloc(void);

/*! @} */
#endif
