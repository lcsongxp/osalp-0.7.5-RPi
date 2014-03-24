/*
 * Copyright: (C) 1999-2001 Bruce W. Forsberg
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 *   Bruce Forsberg  forsberg@tns.net
 *
 */

/*! \class aflibLameFile
    \brief Derived class to write MP3 audio files using the Lame encoder.
 
  This class will write audio files of the MP3 (MPEG Audio Compression
  Format) format. This module uses the lame encoder to encode MP3 formats.
  The Lame encoder can be obtained from http://www.sulaco.org/mp3 . Also the encoder
  must be located in the PATH of your environment in order for this module to
  find it.
*/


#ifndef _AFLIBLAMEFILE_H
#define _AFLIBLAMEFILE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "aflibFile.h"
#include "stdio.h"
#include "stdlib.h"


class aflibLameFile : public aflibFile {

public:

   aflibLameFile();

   ~aflibLameFile();

   aflibStatus
   afopen(
      const char * file,
      aflibConfig* cfg);

   aflibStatus
   afcreate(
      const char * file,
      const aflibConfig& cfg);

   aflibStatus
   afread(
      aflibData& data,
      long long position = -1);

   aflibStatus
   afwrite(
      aflibData& data,
      long long position = -1);

   bool
   isDataSizeSupported(aflib_data_size size);
 
   bool
   isEndianSupported(aflib_data_endian end);
 
   bool
   isSampleRateSupported(int& rate);

private:


FILE*    _fd;
long     _length_value;
int      _sample_rate;

};


#endif
