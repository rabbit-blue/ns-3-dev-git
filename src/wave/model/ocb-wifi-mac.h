/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 * Copyright (c) 2013 Dalian University of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *         Junling Bu <linlinjavaer@gmail.com>
 */
#ifndef OCB_WIFI_MAC_H
#define OCB_WIFI_MAC_H

#include "ns3/object-factory.h"
#include "ns3/regular-wifi-mac.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/qos-utils.h"
#include "vendor-specific-action.h"

namespace ns3 {
class OrganizationIdentifier;
class WifiMacQueue;
/**
 * \brief STAs communicate with each directly outside the context of a BSS
 * \ingroup wave
 *
 * In OCB mac mode,synchronization, association, dis-association
 * and authentication are not used.
 */
class OcbWifiMac : public RegularWifiMac
{
public:
  static TypeId GetTypeId (void);
  OcbWifiMac (void);
  virtual ~OcbWifiMac (void);
  /**
   * \param vsc management packet to send.
   * \param peer the address to which the packet should be sent.
   * \param oi Organization Identifier field
   * see 7.3.1.31 Organization Identifier field and 10.3.29 Vendor-specific action
   *
   * management information can be transmitted over vender specific action frame.
   * This will be mainly called by IEEE Std 1609.4 to send WSA from IEEE Std 1609.3
   */
  void SendVsc (Ptr<Packet> vsc, Mac48Address peer, OrganizationIdentifier oi);
  /**
   * \param oi Organization Identifier
   * \param cb callback to invoke whenever a vender specific action frame has been received and must
   *        be forwarded to the higher layers.
   * every vendor specific content should first register its receive callback
   * for examples:
   * IEEE1609.3 uses VSA frame to send WSA(WAVE Service Advertisement) management information
   * uint8_t oi_bytes[5]={0x00, 0x50, 0xC2, 0x4A, 0x40};
   * OrganizationIdentifier oi_16093(oi_bytes,5);
   * ReceiveVscCallback vsc_16093 =......;
   * AddReceiveVscCallback (oi_16093, vsc_16093);
   * so when VSA frame parses the OrganizationIdentifier of IEEE1609.3
   * then the registed callback will be called.
   */
  void AddReceiveVscCallback (OrganizationIdentifier oi, VscCallback cb);

  void RemoveReceiveVscCallback (OrganizationIdentifier oi);

  /*
   * Although Timing Advertisement frame is a specific management frame defined in 802.11p
   * It is mainly used in IEEE Std 1609.4 for channel switch synchronization
   * And simulation can provide GPS synchronization ability, so we will not implement this.
   *
   * void SendTimingAdvertisement ();
   */

  // GetSsid,SetSsid,SetBssid andGetBssid methods will be overwritted to log warn message
  /**
   * \returns the ssid which this MAC layer is going to try to stay in.
   */
  virtual Ssid GetSsid (void) const;
  /**
   * \param ssid the current ssid of this MAC layer.
   */
  virtual void SetSsid (Ssid ssid);
  /**
   * \param bssid the BSSID of the network that this device belongs to.
   */
  virtual void SetBssid (Mac48Address bssid);
  /**
   * \param bssid the BSSID of the network that this device belongs to.
   */
  virtual Mac48Address GetBssid (void) const;
  /**
   * SetLinkUpCallback and SetLinkDownCallback will be overwrited
   * In OCB mode, stations can send packets directly whenever they want
   * so the link is always up and never down even channel switch
   */
  /**
   * \param linkUp the callback to invoke when the link becomes up.
   */
  virtual void SetLinkUpCallback (Callback<void> linkUp);
  /**
   * \param linkDown the callback to invoke when the link becomes down.
   */
  virtual void SetLinkDownCallback (Callback<void> linkDown);
  /**
   * \param packet the packet to send.
   * \param to the address to which the packet should be sent.
   *
   * The packet should be enqueued in a tx queue, and should be
   * dequeued as soon as the channel access function determines that
   * access is granted to this MAC.
   */
  virtual void Enqueue (Ptr<const Packet> packet, Mac48Address to);

  void ConfigureEdca (uint32_t cwmin, uint32_t cwmax, uint32_t aifsn, enum AcIndex ac);

protected:
  virtual void FinishConfigureStandard (enum WifiPhyStandard standard);
private:
  virtual void Receive (Ptr<Packet> packet, const WifiMacHeader *hdr);

  VendorSpecificContentManager m_vscManager;

  friend class ChannelScheduler;
  /**
   * 1609.4 section 6.2.5: To prevent multiple switching devices from
   * attempting to transmit simultaneously at the end of a guard interval,
   * a medium busy shall be declared during the guard interval.
   * So this method will be used in AlternatingAccess to send a medium
   * busy event.
   */
  void NotifyBusy (Time duration);
  /**
   * 1609.4 section 6.2.5: For a device switching channels on channel interval
   * boundaries, at the beginning of a guard interval, the MAC activities on the
   * previous channel may be suspended.
   * So we set WifiMacQueue subclass which could support multiple channel, when
   * channel switching, we will not flush queue automatically, and could switch
   * CCH queue to SCH queue or switch SCH queue to CCH queue.
   */
  void SetWaveEdcaQueue (AcIndex ac, Ptr<EdcaTxopN> edca);
};

}
#endif /* OCB_WIFI_MAC_H */