
/**
 * @file
 * @brief Describes interfaces to Address Frame (AddrFrame) C-Class.
 * AddrFrames are Frames that contain some type of network address.
 * The types of addresses we support are defined by the @ref AddressFamilyNumbers "IETF/IANA Address type assignments".
 * @see AddressFamilyNumbers
 * @author &copy; 2011 - Alan Robertson <alanr@unix.sh>
 * @n
 * Licensed under the GNU Lesser General Public License (LGPL) version 3 or any later version at your option.
 * excluding the provision allowing for relicensing under the GPL at your option.
 */

#ifndef _ADDRFRAME_H
#define _ADDRFRAME_H
#include <frame.h>
#include <netaddr.h>
typedef struct _AddrFrame AddrFrame;

/// This is our @ref AddrFrame object - used for holding @ref NetAddr network addresses.
/// It has some different member functions implementations than its base @ref Frame -
/// mainly for validating packet contents.
///@{
/// @ingroup AddrFrame
struct _AddrFrame {
	Frame		baseclass;
	void(*setaddr)	(AddrFrame* f, guint16 addrtype, gconstpointer addr, gsize addrlen);
	void(*setnetaddr)(AddrFrame* f, NetAddr* addr);
};

#ifdef _MSC_VER
#define EXP_FUNC __declspec( dllexport )
#endif

EXP_FUNC AddrFrame* addrframe_new(guint16 frame_type, gsize framesize);
EXP_FUNC AddrFrame* addrframe_ipv4_new(guint16 frame_type, gconstpointer addr);
EXP_FUNC AddrFrame* addrframe_ipv6_new(guint16 frame_type, gconstpointer addr);
EXP_FUNC AddrFrame* addrframe_mac48_new(guint16 frame_type, gconstpointer addr);
EXP_FUNC AddrFrame* addrframe_mac64_new(guint16 frame_type, gconstpointer addr);
EXP_FUNC Frame* addrframe_tlvconstructor(gconstpointer tlvstart, gconstpointer pktend);

///@}
#endif /* _ADDRFRAME_H */
