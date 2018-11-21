/* Simple tool to access registers of atheros switch on FB6490.
 *
 * Copyright (C) 2017 - Felix Schmidt
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
#ifndef _athtoolh_
#define _athtoolh_

#include "ar8216.h"
#include "ar8327.h"

#define verb_printf(lvl, ...) if (_ath_verbose > lvl) { printf(__VA_ARGS__); }

#define MAX_PORTS	6


/*! \ingroup athtool */
/*! @{ */

extern int _ath_verbose;

/*! L2 Address table entry
 */
struct ath_arl_entry {
	/*! Bit mask of ports */
        uint32_t ports;

	/*! Flags. Bit mask of
	 * - AR8327_ATU_PRI
	 * - AR8327_ATU_PRI_OVR
	 * - AR8327_ATU_SVL_LEARNED
	 * - AR8327_ATU_MIRROR
	 * - AR8327_ATU_DROP
	 * - AR8327_ATU_LEAKY
	 * - AR8327_ATU_REDIR_CPU
	 * - AR8327_ATU_COPY_CPU
	 */
	uint32_t flags;

	/*! VLAN ID */
	uint32_t vid;

	/*! MAC address */
        uint8_t mac[6];
};


/*! Atheros switch device handle */
struct ath_dev
{
    /*! Device ID (register 0) */
    uint32_t	dev_id;

    /*! Number of ports */
    int		num_ports;

    /*! controlled device instance. */
    unsigned    instance;

    /*! Description of recent API error */
    const char *ath_err;

    /*! Pointer to counter list (shared memory segment 0xfefec002+instance) */
    struct ath_counter_state *cnt_state;

    /*! Whether counters have been initialized */
    int		cnt_initialized;

    /*! Content of /proc/driver/avmnet/ar8327 */
    char	*file_buffer;

    /*! Read time of file_buffer */
    uint64_t 	 rtime;

    /*! Register access callout
     *
     * \param dev this
     * \param reg register offset
     * \param mask which bits to modify (0 = read-only, 0xffffffff = write-only)
     * \param value write value, masked with \a mask
     * \param optional error pointer. Value not modified on success, i.e. should be
     *	    initialized to 0 before calling.
     *
     * \returns Value written to register, or read value if mask is 0. 0xffffffff in case of
     * error (indicated by *err written to a value not 0).
     */
    uint32_t (*ath_rmw) (struct ath_dev *dev, uint32_t reg, uint32_t mask, uint32_t value, int *err);
};


#define SETERR(_s)  dev->ath_err = _s;
#define PRERR(...) {fprintf (stderr, "athtool :: ");\
    fprintf (stderr, __VA_ARGS__);\
    fprintf (stderr, " :: ERROR[%d] :: %s\n", dev->instance, dev->ath_err ? dev->ath_err : "(none)");\
    fflush(stderr);}

#define FILEBUF_SIZE	10000

extern uint32_t ath_rmw (struct ath_dev *dev, uint32_t reg, uint32_t mask, uint32_t value, int *err);
extern void ath_vlan_show (struct ath_dev *dev);
extern int ath_vlan_create (struct ath_dev *dev, uint32_t vid, uint32_t attr);
extern int ath_vlan_delete (struct ath_dev *dev, uint32_t vid);
extern int ath_vlan_port_rm (struct ath_dev *dev, uint32_t vid, uint32_t port);
extern int ath_vlan_port_add (struct ath_dev *dev, uint32_t vid, uint32_t port, uint32_t mode);
extern uint32_t ath_attr_set_port (struct ath_dev *dev, uint32_t attr, uint32_t port, uint32_t mode);
extern int ath_pvid_port (struct ath_dev *dev, uint32_t port, uint32_t vid);
extern int ath_svid_port (struct ath_dev *dev, uint32_t port, uint32_t vid);
extern int ath_port_vlan_attr_parse(char *attr_list, uint32_t *value, uint32_t *mask);
extern int ath_vattr_port (struct ath_dev *dev, uint32_t port, uint32_t attr, uint32_t mask);


extern void ath_arl_dump (struct ath_dev *dev);
extern int ath_arl_flags_parse (struct ath_dev *dev, struct ath_arl_entry *entry, char *entry_spec);
extern int ath_arl_add (struct ath_dev *dev, struct ath_arl_entry *entry);
extern int ath_arl_rm (struct ath_dev *dev, struct ath_arl_entry *entry);

#endif


/*! @} */
