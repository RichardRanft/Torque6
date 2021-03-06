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

#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _PLATFORMASSERT_H_
#include "platform/platformAssert.h"
#endif

#ifndef _TEXTURE_OBJECT_H_
#include "graphics/TextureObject.h"
#endif

#ifndef _TEXTURE_DICTIONARY_H_
#include "graphics/TextureDictionary.h"
#endif

//-----------------------------------------------------------------------------

#define MaximumProductSupportedTextureWidth 2048
#define MaximumProductSupportedTextureHeight MaximumProductSupportedTextureWidth

class TextureManager
{
   friend class TextureHandle;
   friend class TextureDictionary;

public:
    /// Texture manager event codes.
    enum TextureEventCode
    {
        BeginZombification,
        BeginResurrection,
        EndResurrection,
    };

    typedef void (*TextureEventCallback)(const TextureEventCode eventCode, void *userData);

    /// Textrue manager state.
    enum ManagerState
    {
        NotInitialized = 0,
        Alive,
        Dead,
        Resurrecting,
    };

private:
    static S32 mMasterTextureKeyIndex;
    static ManagerState mManagerState;
    static bool mForce16BitTexture;
    static bool mAllowTextureCompression;
    static bool mDisableTextureSubImageUpdates;

public:
    static bool mDGLRender;

public:
    static void create();
    static void destroy();
    static ManagerState getManagerState( void ) { return mManagerState; }

    static void killManager();
    static void resurrectManager();
    static void flush();
    static void refresh( const char *textureName );

    static U32  registerEventCallback(TextureEventCallback, void *userData);
    static void unregisterEventCallback(const U32 callbackKey);

    static StringTableEntry getUniqueTextureKey( void );

    static void dumpMetrics( void );

    static TextureObject* loadTexture(const char *textureName, TextureHandle::TextureHandleType type, U32 flags, bool checkOnly = false, bool force16Bit = false );
    static void freeTexture( TextureObject* pTextureObject );

private:
    static void postTextureEvent(const TextureEventCode eventCode);

    static void createBGFXTexture( TextureObject* pTextureObject );
    static TextureObject* registerTexture(const char *textureName, GBitmap* pNewBitmap, TextureHandle::TextureHandleType type, U32 flags);
    static void refresh(TextureObject* pTextureObject);

    static GBitmap* loadBitmap(const char *textureName, bool recurse = true, bool nocompression = false);
    static GBitmap* createPowerOfTwoBitmap( GBitmap* pBitmap );

    static void swizzleRGBtoBGRA(U32 width, U32 height, const U8* src, U8* dest);
    static void swizzleRGBtoRGBA(U32 width, U32 height, const U8* src, U8* dest);
    static bgfx::TextureHandle getMipMappedTexture(StringTableEntry _textureKey, U32 width, U32 height, const U8* _src, U32 _flags = BGFX_TEXTURE_NONE, bool _swizzleToBGRA = true);
    static const bgfx::Memory* generateMipMappedTexture(U32 _numMips, U32 _width, U32 _height, const U8* _src, bool _swizzleToBGRA = true);
};

#endif // _TEXTURE_MANAGER_H_