/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"
#include "midi_device_multistream.h"

static uint8_t packet[4];
static bool packet_ok = false;
static uint8_t packet_bytes_to_stream = 0;
uint32_t tud_midi_demux_stream_read (uint8_t* cable_num, void* buffer, uint32_t bufsize)
{
  uint8_t stream_total = 0;
  uint32_t nread = 0;
  uint8_t* buf8 = (uint8_t*)buffer;
  uint8_t current_cable = (packet[0] >> 4) & 0xf; // assume the static packet buffer contains a valid packet

  if (packet_ok && packet_bytes_to_stream > 0)
  {
    // already read some bytes but could not fit them in the last buffer. Try again
    nread = (uint8_t) tu_min32(packet_bytes_to_stream, bufsize);
    memcpy(buf8, packet+4-nread, nread);
    buf8 += nread;
    nread += packet_bytes_to_stream;
    packet_bytes_to_stream -= nread;
    *cable_num = current_cable;
    if (packet_bytes_to_stream > 0)
    {
      return nread; // still could not fit the whole packet in the buffer
    }
    packet_ok = tud_midi_packet_read(packet);
    current_cable = (packet[0] >> 4) & 0xf;
    if (!packet_ok || current_cable != *cable_num)
    {
      // new packet switches cable number; need to return
      return nread;
    }
  }
  if (!packet_ok)
  {
    // nead to read a packet and figure out its cable number
    packet_ok = tud_midi_packet_read(packet);
    current_cable = (packet[0] >> 4) & 0xf;
  }
  if (packet_ok)
  {
    *cable_num = current_cable;
    // while the packet is good and the cable number did not change
    while (packet_ok && current_cable == *cable_num)
    {
      uint8_t const code_index = packet[0] & 0x0f;

      // MIDI 1.0 Table 4-1: Code Index Number Classifications
      switch(code_index)
      {
        case MIDI_CIN_MISC:
        case MIDI_CIN_CABLE_EVENT:
          // These are reserved and unused, possibly issue somewhere, skip this packet
          packet_ok = false;
          return 0;
        break;

        case MIDI_CIN_SYSEX_END_1BYTE:
        case MIDI_CIN_1BYTE_DATA:
          stream_total = 1;
        break;

        case MIDI_CIN_SYSCOM_2BYTE     :
        case MIDI_CIN_SYSEX_END_2BYTE  :
        case MIDI_CIN_PROGRAM_CHANGE   :
        case MIDI_CIN_CHANNEL_PRESSURE :
          stream_total = 2;
        break;

        default:
          stream_total = 3;
        break;
      }
      // if the data in the new packet will fit in the read buffer,
      // copy it and keep going
      uint8_t byte_count = (uint8_t) tu_min32(stream_total, (bufsize-nread));
      memcpy(buf8, packet+1, byte_count);
      nread += byte_count;
      buf8 += byte_count;
      if (stream_total > byte_count)
      {
        // ran out of space for this packet in the buffer
        // record how many bytes are left and return how many we copied.
        packet_bytes_to_stream = stream_total - byte_count;
        return nread;
      }
      // try to read the next packet; if none available, packet_ok will be false
      packet_ok = tud_midi_packet_read(packet);
      // assume it worked and extract the cable number for the packet
      current_cable = (packet[0] >> 4) & 0xf;
    }
  }
  return nread;
}