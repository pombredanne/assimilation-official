/**
 * @file
 * @brief Header file for various basic CDP (Cisco Discovery Protocol definitions and accessor functions.
 * There is a lot that's not documented and not understood about this protocol.
 * But what we understand is somewhat documented here.
 *
 * This file is part of the Assimilation Project.
 *
 * @author Copyright &copy; 2011, 2012 - Alan Robertson <alanr@unix.sh>
 * @n
 *  The Assimilation software is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The Assimilation software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the Assimilation Project software.  If not, see http://www.gnu.org/licenses/
 */

#include <projectcommon.h>
#include <glib.h>
/** @defgroup cdp_type_fields CDP: Known values for TLV types
 *  @see get_cdptlv_type
 *  @{ 
 *  @ingroup cdp_format
 *  @ingroup DefineEnums
 */
#define	CDP_TLV_DEVID		0x0001	///< Hostname of the device or hardware serial number in ASCII.
					///< **This is one of the two CDP values our client code pays attention to.
#define	CDP_TLV_ADDRESS		0x0002	///< The L3 address of the interface that has sent the update
#define	CDP_TLV_PORTID		0x0003	///< The port from which the CDP update has been sent.
					///< **This is one of the two CDP values our client code pays attention to.
#define	CDP_TLV_CAPS		0x0004	///< Bit mask describing the functional capabilities of the device
#define	CDP_TLV_VERS		0x0005	///< A character string containing the software version (same as in show version).
#define	CDP_TLV_PLATFORM	0x0006	///< Hardware platform, such as WS-C5000, WS-C6009, or Cisco RSP
#define	CDP_TLV_IPPREFIX	0x0007	///< 4 bytes of IPv4 network prefix + 1 byte of CIDR-style netmask
#define	CDP_TLV_HELLO		0x0008	///< Protocol-Hello - encapsulates some other protocol
#define	CDP_TLV_VTPDOMAIN	0x0009	///< The VTP Domain, if configured on the device
#define	CDP_TLV_NATIVEVLAN	0x000A	///< In dot1q, this is the untagged VLAN
#define	CDP_TLV_DUPLEX		0x000B	///< This field contains the duplex setting of the sending port
#define	CDP_TLV_APPLID		0x000E	///< Appliance ID - auxiliary vlan ID for a VOIP phone
#define	CDP_TLV_POWER		0x0010	///< Power consumption in milliwatts
/// @}

/** @defgroup cdp_capapbility_bits CDP: Capability (CDP_TLV_CAPS) mask bits.
 *  The TLV type CDP_TLV_CAPS value is a field with a set of bits defining the capabilities which a given
 *  switch or router might implement.  The macro definitions below define which capabilities correspond to which
 *  bits in a CDP packet.
 *  @see CDP_TLV_CAPS
 *  @{ 
 *  @ingroup cdp_format
 *  @ingroup DefineEnums
 */
#define CDP_CAPMASK_ROUTER	0x01	///< L3 Router
#define CDP_CAPMASK_TBBRIDGE	0x02	///< TB Bridge
#define CDP_CAPMASK_SPBRIDGE	0x04	///< SP Bridge
#define CDP_CAPMASK_SWITCH	0x08	///< L2 Switch
#define CDP_CAPMASK_HOST	0x10	///< Host
#define CDP_CAPMASK_IGMPFILTER	0x20	///< Does not forward IGMP report packets on non-routerports.
#define CDP_CAPMASK_REPEATER	0x40	///< Repeater
/// @}

WINEXPORT guint8 get_cdp_vers(gconstpointer pktptr, gconstpointer pktend);
WINEXPORT guint8 get_cdp_ttl(gconstpointer pktptr, gconstpointer pktend);
WINEXPORT guint16 get_cdp_cksum(gconstpointer pktptr, gconstpointer pktend);
WINEXPORT guint16 get_cdptlv_type(gconstpointer tlv_vp, gconstpointer pktend);
WINEXPORT gsize get_cdptlv_len(gconstpointer tlv_vp, gconstpointer pktend);
WINEXPORT gsize get_cdptlv_vlen(gconstpointer tlv_vp, gconstpointer pktend);
WINEXPORT gconstpointer get_cdptlv_body(gconstpointer tlv_vp, gconstpointer pktend);
WINEXPORT gconstpointer get_cdptlv_first(gconstpointer pkt, gconstpointer pktend);
WINEXPORT gconstpointer get_cdptlv_next(gconstpointer tlv_vp, gconstpointer pktend);
WINEXPORT gconstpointer get_cdp_chassis_id(gconstpointer packet, gssize* idlength, gconstpointer pktend);
WINEXPORT gconstpointer get_cdp_port_id(gconstpointer packet, gssize* idlength, gconstpointer pktend);
WINEXPORT gboolean is_valid_cdp_packet(gconstpointer packet, gconstpointer pktend);
WINEXPORT gboolean enable_cdp_packets(gboolean enableme);
