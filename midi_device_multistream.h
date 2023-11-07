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
#pragma once
#include <stdint.h>
#include <boost/preprocessor/enum.hpp>
// USB MIDI allows up to 16 virtual cable "streams" per USB endpoint
// The following macros for for a multi-stream interface still assume one
// IN endpoint and one OUT endpoint but permit up to 16 streams per endpoint
// The TUD_MIDI_MULTI_* macros that follow use the Boost Preprocessor macro library
// to implement 1-16 virtual cables for the USB IN endpoint
// and 1-16 virtual cablces for the USB OUT endpoint. It is not necessary
// to make the number of cables for the IN endpoint match the number of cables
// for the OUT endpoint

// Assign jack ID numbers for all External IN and Embedded OUT Jacks
// - _cablenum is the virtual cable number associated with the Jack pair
#define TUD_MIDI_MULTI_JACKID_IN_EXT(_cablenum) \
  (uint8_t)((_cablenum) * 2 + 1)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB(_cablenum) \
  (uint8_t)((_cablenum) * 2 + 2)

// Assign jack ID numbers for all External OUT and Embedded IN Jacks
// - _cablenum is the nth virtual cable number associated with the Jack pair, where n starts from 1
// - _numcables_in is the number of External IN Jacks.
#define TUD_MIDI_MULTI_JACKID_IN_EMB(_cablenum, _numcables_in) \
  (uint8_t)((_cablenum) * 2 + 1 + ((_numcables_in) * 2))
#define TUD_MIDI_MULTI_JACKID_OUT_EXT(_cablenum, _numcables_in) \
  (uint8_t)((_cablenum) * 2 + 2 + ((_numcables_in) * 2))

#ifndef CFG_TUD_MIDI_FIRST_PORT_STRIDX
#define CFG_TUD_MIDI_FIRST_PORT_STRIDX 0
#endif

#if CFG_TUD_MIDI_FIRST_PORT_STRIDX
#define EMB_IN_JACK_STRIDX(n) (n+(CFG_TUD_MIDI_FIRST_PORT_STRIDX))
#define EMB_OUT_JACK_STRIDX(_numcables_in, n) (n+_numcables_in+(CFG_TUD_MIDI_FIRST_PORT_STRIDX))
#else
#define EMB_IN_JACK_STRIDX(n) 0
#define EMB_OUT_JACK_STRIDX(_numcables_in, n) 0
#endif


#define TUD_MIDI_MULTI_JACK_IN_DESC(_cablenum, _stridx)\
  /* MS In Jack (External) */\
  6, TUSB_DESC_CS_INTERFACE, MIDI_CS_INTERFACE_IN_JACK, MIDI_JACK_EXTERNAL, TUD_MIDI_MULTI_JACKID_IN_EXT(_cablenum), 0,\
  /* MS Out Jack (Embedded), connected to In Jack External */\
  9, TUSB_DESC_CS_INTERFACE, MIDI_CS_INTERFACE_OUT_JACK, MIDI_JACK_EMBEDDED, TUD_MIDI_MULTI_JACKID_OUT_EMB(_cablenum), 1, TUD_MIDI_MULTI_JACKID_IN_EXT(_cablenum), 1, _stridx

#define TUD_MIDI_MULTI_DESC_JACK_LEN(_numcables) ((6 + 9) * (_numcables))

#define TUD_MIDI_MULTI_JACK_OUT_DESC(_cablenum, _numcables_in, _stridx)\
  /* MS In Jack (Embedded) */\
  6, TUSB_DESC_CS_INTERFACE, MIDI_CS_INTERFACE_IN_JACK, MIDI_JACK_EMBEDDED, TUD_MIDI_MULTI_JACKID_IN_EMB(_cablenum, _numcables_in), _stridx,\
  /* MS Out Jack (External), connected to In Jack Embedded */\
  9, TUSB_DESC_CS_INTERFACE, MIDI_CS_INTERFACE_OUT_JACK, MIDI_JACK_EXTERNAL, TUD_MIDI_MULTI_JACKID_OUT_EXT(_cablenum, _numcables_in), 1, TUD_MIDI_MULTI_JACKID_IN_EMB(_cablenum, _numcables_in), 1, 0

#define TUD_MIDI_MULTI_DESC_LEN(_numcables_in, _numcables_out) (TUD_MIDI_DESC_HEAD_LEN + TUD_MIDI_MULTI_DESC_JACK_LEN(_numcables_in) +\
                                                                TUD_MIDI_MULTI_DESC_JACK_LEN(_numcables_out) +\
                                                                TUD_MIDI_DESC_EP_LEN(_numcables_in) + TUD_MIDI_DESC_EP_LEN(_numcables_out))

#define TUD_MIDI_MULTI_DESC_JACK_IN_ENUM_DESC(z, n, data) TUD_MIDI_MULTI_JACK_IN_DESC(n, EMB_IN_JACK_STRIDX(n))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_ENUM_DESC(z, n, _numcables_in) TUD_MIDI_MULTI_JACK_OUT_DESC(n, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, n))
#define TUD_MIDI_MULTI_DESC_JACK_DESC(_numcables_in, _numcables_out)\
  BOOST_PP_ENUM(_numcables_in, TUD_MIDI_MULTI_DESC_JACK_IN_ENUM_DESC, 0),\
  BOOST_PP_ENUM(_numcables_out, TUD_MIDI_MULTI_DESC_JACK_OUT_ENUM_DESC, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_ENUM_EMB(z, n, _numcables_in) TUD_MIDI_MULTI_JACKID_IN_EMB(n, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_OUT_ENUM_EMB(z, n, data) TUD_MIDI_MULTI_JACKID_OUT_EMB(n)
#define TUD_MIDI_MULTI_DESC_JACKID_IN_EMB(_numcables_out, _numcables_in) BOOST_PP_ENUM(_numcables_out, TUD_MIDI_MULTI_JACKID_IN_ENUM_EMB, _numcables_in)
#define TUD_MIDI_MULTI_DESC_JACKID_OUT_EMB(_numcables_in) BOOST_PP_ENUM(_numcables_in, TUD_MIDI_MULTI_JACKID_OUT_ENUM_EMB, 0)

#define TUD_MIDI_MULTI_DESC_HEAD(_itfnum,  _stridx, _numcables_in, _numcables_out) \
  /* Audio Control (AC) Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 0, TUSB_CLASS_AUDIO, AUDIO_SUBCLASS_CONTROL, AUDIO_FUNC_PROTOCOL_CODE_UNDEF, _stridx,\
  /* AC Header */\
  9, TUSB_DESC_CS_INTERFACE, AUDIO_CS_AC_INTERFACE_HEADER, U16_TO_U8S_LE(0x0100), U16_TO_U8S_LE(0x0009), 1, (uint8_t)((_itfnum) + 1),\
  /* MIDI Streaming (MS) Interface */\
  9, TUSB_DESC_INTERFACE, (uint8_t)((_itfnum) + 1), 0, 2, TUSB_CLASS_AUDIO, AUDIO_SUBCLASS_MIDI_STREAMING, AUDIO_FUNC_PROTOCOL_CODE_UNDEF, 0,\
  /* MS Header */\
  7, TUSB_DESC_CS_INTERFACE, MIDI_CS_INTERFACE_HEADER, U16_TO_U8S_LE(0x0100), U16_TO_U8S_LE(7 +\
     TUD_MIDI_MULTI_DESC_JACK_LEN(_numcables_in) + TUD_MIDI_MULTI_DESC_JACK_LEN(_numcables_out)+\
     TUD_MIDI_DESC_EP_LEN(_numcables_in) + TUD_MIDI_DESC_EP_LEN(_numcables_out))

// MIDI multi-stream descriptor.
// - _itfnum is the interface number for the configuration descriptor
// - _stridx is the index of the string that describes the interface
// - _epout is the address of the Bulk OUT endpoint
// - _epin is the address of the Bulk IN endpoint
// - _epsize is the maximum transfer size for the Bulk endpoints
// - _numcables_in Number of Embedded IN Jacks connected to corresponding External Jack Out (routes to the host OUT endpoint)
// - _numcables_out Number of Embedded OUT Jacks connected to corresponding External Jack In (routes to the Host IN endpoint)
#define TUD_MIDI_MULTI_DESCRIPTOR(_itfnum, _stridx, _epout, _epin, _epsize, _numcables_in, _numcables_out) \
  TUD_MIDI_MULTI_DESC_HEAD(_itfnum, _stridx, _numcables_in, _numcables_out),\
  TUD_MIDI_MULTI_DESC_JACK_DESC(_numcables_in, _numcables_out),\
  TUD_MIDI_DESC_EP(_epout, _epsize, _numcables_out),\
  TUD_MIDI_MULTI_DESC_JACKID_IN_EMB(_numcables_out, _numcables_in),\
  TUD_MIDI_DESC_EP(_epin, _epsize, _numcables_in),\
  TUD_MIDI_MULTI_DESC_JACKID_OUT_EMB(_numcables_in)

// Return the number of bytes read in the stream and set *cable_num to the cable number in the stream.
// Return 0 when when there are no more streams or stream fragments in the receive FIFO
// If cable_num is NULL, then this function behaves like to tud_midi_stream_read()
uint32_t tud_midi_demux_stream_read  (uint8_t* cable_num, void* buffer, uint32_t bufsize);