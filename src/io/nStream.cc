//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "stream.h"
#include "string/stringTable.h"
#include "graphics/color.h"
#include "math/mPoint.h"
#include "math/mMatrix.h"
#include "math/mTransform.h"

#ifndef _INC_STDARG
#include <stdarg.h>
#endif

Stream::Stream()
 : m_streamStatus(Closed)
{
   //
}

Stream::~Stream()
{
   //
}

const char* Stream::getStatusString(const Status in_status)
{
   switch (in_status) {
      case Ok:
         return "StreamOk";
      case IOError:
         return "StreamIOError";
      case EOS:
         return "StreamEOS";
      case IllegalCall:
         return "StreamIllegalCall";
      case Closed:
         return "StreamClosed";
      case UnknownError:
         return "StreamUnknownError";

     default:
      return "Invalid Stream::Status";
   }
}

void Stream::writeString(const char *string, S32 maxLen)
{
   S32 len = string ? dStrlen(string) : 0;
   if(len > maxLen)
      len = maxLen;

   write(U8(len));
   if(len)
      write(len, string);
}

bool Stream::writeFormattedBuffer(const char *format, ...)
{
   char buffer[4096];
   va_list args;
   va_start(args, format);
   const S32 length = vsprintf(buffer, format, args);

   // Sanity!
   AssertFatal(length <= sizeof(buffer), "writeFormattedBuffer - String format exceeded buffer size.  This will cause corruption.");

   return write( length, buffer );
}

void Stream::readString(char buf[256])
{
   U8 len;
   read(&len);
   read(S32(len), buf);
   buf[len] = 0;
}

const char *Stream::readSTString(bool casesens)
{
   char buf[256];
   readString(buf);
   return StringTable->insert(buf, casesens);
}

void Stream::readLongString(U32 maxStringLen, char *stringBuf)
{
   U32 len;
   read(&len);
   if(len > maxStringLen)
   {
      m_streamStatus = IOError;
      return;
   }
   read(len, stringBuf);
   stringBuf[len] = 0;
}

void Stream::writeLongString(U32 maxStringLen, const char *string)
{
   U32 len = dStrlen(string);
   if(len > maxStringLen)
      len = maxStringLen;
   write(len);
   write(len, string);
}

void Stream::readLine(U8 *buffer, U32 bufferSize)
{
   bufferSize--;  // account for NULL terminator
   U8 *buff = buffer;
   U8 *buffEnd = buff + bufferSize;
   *buff = '\r';

   // strip off preceding white space
   while ( *buff == '\r' )
   {
      if ( !read(buff) || *buff == '\n' )
      {
         *buff = 0;
         return;
      }
   }

   // read line
   while ( buff != buffEnd && read(++buff) && *buff != '\n' )
   {
      if ( *buff == '\r' )
      {

#if defined(TORQUE_OS_OSX)
      U32 pushPos = getPosition(); // in case we need to back up.
      if (read(buff)) // feeling free to overwrite the \r as the NULL below will overwrite again...
          if (*buff != '\n') // then push our position back.
             setPosition(pushPos);
       break; // we're always done after seeing the CR...
#else
      buff--; // 'erases' the CR of a CRLF
#endif

      }
   }
   *buff = 0;
}

void Stream::writeLine(U8 *buffer)
{
   write(dStrlen((const char*)buffer), buffer);
   write(2, "\r\n");
}

bool Stream::write(const ColorI& rColor)
{
   bool success = write(rColor.red);
   success     |= write(rColor.green);
   success     |= write(rColor.blue);
   success     |= write(rColor.alpha);

   return success;
}

bool Stream::write(const ColorF& rColor)
{
   ColorI temp = rColor;
   return write(temp);
}

bool Stream::read(ColorI* pColor)
{
   bool success = read(&pColor->red);
   success     |= read(&pColor->green);
   success     |= read(&pColor->blue);
   success     |= read(&pColor->alpha);

   return success;
}

bool Stream::read(ColorF* pColor)
{
   ColorI temp;
   bool success = read(&temp);

   *pColor = temp;
   return success;
}

bool Stream::copyFrom(Stream *other)
{
   U8 buffer[1024];
   U32 numBytes = other->getStreamSize() - other->getPosition();
   while((other->getStatus() != Stream::EOS) && numBytes > 0)
   {
      U32 numRead = numBytes > sizeof(buffer) ? sizeof(buffer) : numBytes;
      if(! other->read(numRead, buffer))
         return false;

      if(! write(numRead, buffer))
         return false;

      numBytes -= numRead;
   }

   return true;
}

bool Stream::writePoint3F(const Point3F& rPoint)
{
   bool success = write(rPoint.x);
   success     |= write(rPoint.y);
   success     |= write(rPoint.z);

   return success;
}

bool Stream::readPoint3F(Point3F* pPoint)
{
   bool success = read(&pPoint->x);
   success     |= read(&pPoint->y);
   success     |= read(&pPoint->z);

   return success;
}

bool Stream::writeMatrixF(const MatrixF& rMat)
{
   bool success = false;
   
   for (U32 n = 0; n < 16; ++n)
      success |= write(rMat.m[n]);

   return success;
}

bool Stream::readMatrixF(MatrixF* pMat)
{
   bool success = false;
   
   for (U32 n = 0; n < 16; ++n)
      success |= read(&pMat->m[n]);

   return success;
}

bool Stream::writeTransform(const Transform& rTransform)
{
   bool success = false;

   success |= writePoint3F(rTransform.getPosition());
   success |= writePoint3F(rTransform.getRotationEuler());
   success |= writePoint3F(rTransform.getScale());

   return success;
}

bool Stream::readTransform(Transform* pTransform)
{
   bool success = false;

   Point3F position;
   success |= readPoint3F(&position);

   VectorF rotationEuler;
   success |= readPoint3F(&rotationEuler);

   VectorF scale;
   success |= readPoint3F(&scale);

   pTransform->set(position, rotationEuler, scale);

   return success;
}