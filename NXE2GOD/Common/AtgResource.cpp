//--------------------------------------------------------------------------------------
// AtgResource.cpp
//
// Loads resources from an XPR (Xbox Packed Resource) file
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include "AtgDevice.h"
#include "AtgResource.h"
#include "AtgUtil.h"

namespace ATG
{


// Global access to the main D3D device
extern D3DDevice* g_pd3dDevice;


//--------------------------------------------------------------------------------------
// Magic values to identify XPR files
//--------------------------------------------------------------------------------------
struct XPR_HEADER
{
    DWORD dwMagic;
    DWORD dwHeaderSize;
    DWORD dwDataSize;
};

const DWORD XPR2_MAGIC_VALUE = 0x58505232;

const DWORD eXALLOCAllocatorId_AtgResource = eXALLOCAllocatorId_GameMax;


//--------------------------------------------------------------------------------------
// Name: PackedResource
//--------------------------------------------------------------------------------------
PackedResource::PackedResource()
{
    m_pSysMemData = NULL;
    m_dwSysMemDataSize = 0L;
    m_pVidMemData = NULL;
    m_dwVidMemDataSize = 0L;
    m_pResourceTags = NULL;
    m_dwNumResourceTags = 0L;
    m_bInitialized = FALSE;
}


//--------------------------------------------------------------------------------------
// Name: PackedResource
//--------------------------------------------------------------------------------------
PackedResource::~PackedResource()
{
    Destroy();
}


//--------------------------------------------------------------------------------------
// Name: GetData
// Desc: Loads all the texture resources from the given XPR.
//--------------------------------------------------------------------------------------
VOID* PackedResource::GetData( const CHAR* strName ) const
{
    if( NULL == m_pResourceTags || NULL == strName )
        return NULL;

    for( DWORD i = 0; i < m_dwNumResourceTags; i++ )
    {
        if( !_stricmp( strName, m_pResourceTags[i].strName ) )
        {
            return &m_pSysMemData[m_pResourceTags[i].dwOffset];
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------
// Name: AllocateContiguousMemory()
// Desc: Wrapper for XMemAlloc
//--------------------------------------------------------------------------------------
static __forceinline void* AllocateContiguousMemory( DWORD Size, DWORD Alignment,
                                                     DWORD Protection = XALLOC_MEMPROTECT_WRITECOMBINE )
{
    return XMemAlloc( Size, MAKE_XALLOC_ATTRIBUTES( 0, 0, 0, 0, eXALLOCAllocatorId_AtgResource,
                                                    Alignment, Protection, 0,
                                                    XALLOC_MEMTYPE_PHYSICAL ) );
}


//--------------------------------------------------------------------------------------
// Name: FreeContiguousMemory()
// Desc: Wrapper for XMemFree
//--------------------------------------------------------------------------------------
static __forceinline VOID FreeContiguousMemory( VOID* pData )
{
    return XMemFree( pData, MAKE_XALLOC_ATTRIBUTES( 0, 0, 0, 0, eXALLOCAllocatorId_AtgResource,
                                                    0, 0, 0, XALLOC_MEMTYPE_PHYSICAL ) );
}


//--------------------------------------------------------------------------------------
// Name: Create
// Desc: Loads all the texture resources from the given XPR.
//--------------------------------------------------------------------------------------
HRESULT PackedResource::Create( const CHAR* strFilename )
{
    // Open the file
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BOOL isEmbedded = FALSE;
    DWORD dwNumBytesRead = 0;
	PBYTE pbData = NULL;
	DWORD dwSectSz = 0;

	if(strncmp(strFilename, "embed:\\", strlen("embed:\\")) == 0)
		isEmbedded = TRUE;

	if(isEmbedded)
	{
		HMODULE hmHand = GetModuleHandle(NULL);
		if(hmHand == NULL)
		{
			ATG_PrintError( "File <%s> not found, could not get module handle\n", strFilename );
			return E_FAIL;
		}
		if(!XGetModuleSection(hmHand, &strFilename[7], (PVOID*)&pbData, &dwSectSz))
		{
			ATG_PrintError( "Embedded file <%s> not found\n", &strFilename[7] );
			return E_FAIL;
		}
	}
	else
	{
		hFile = CreateFile( strFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL );
		if( hFile == INVALID_HANDLE_VALUE )
		{
			ATG_PrintError( "File <%s> not found\n", strFilename );
			return E_FAIL;
		}
	}

    // Read in and verify the XPR magic header
    XPR_HEADER xprh;
	if(isEmbedded)
	{
		memcpy(&xprh, pbData, sizeof(XPR_HEADER));
	}
	else
	{
		if( !ReadFile( hFile, &xprh, sizeof( XPR_HEADER ), &dwNumBytesRead, NULL ) )
		{
			ATG_PrintError( "Error reading XPR header in file <%s>\n", strFilename );
			CloseHandle( hFile );
			return E_FAIL;
		}
	}

#ifdef _PC
    XGEndianSwapMemory( &xprh, &xprh, XGENDIAN_8IN32, sizeof(DWORD), sizeof(XPR_HEADER)/sizeof(DWORD) );
#endif

    if( xprh.dwMagic != XPR2_MAGIC_VALUE )
    {
        ATG_PrintError( "Invalid Xbox Packed Resource (.xpr) file: Magic = 0x%08lx\n", xprh.dwMagic );
		if(!isEmbedded)
			CloseHandle( hFile );
        return E_FAIL;
    }

    // Compute memory requirements
    m_dwSysMemDataSize = xprh.dwHeaderSize;
    m_dwVidMemDataSize = xprh.dwDataSize;

    // Allocate memory
    m_pSysMemData = new BYTE[m_dwSysMemDataSize];
    if( m_pSysMemData == NULL )
    {
        ATG_PrintError( "Could not allocate system memory.\n" );
        m_dwSysMemDataSize = 0;
        return E_FAIL;
    }
    m_pVidMemData = ( BYTE* )AllocateContiguousMemory( m_dwVidMemDataSize, XALLOC_PHYSICAL_ALIGNMENT_4K );
    if( m_pVidMemData == NULL )
    {
        ATG_PrintError( "Could not allocate physical memory.\n" );
        m_dwSysMemDataSize = 0;
        m_dwVidMemDataSize = 0;
        delete[] m_pSysMemData;
        m_pSysMemData = NULL;
        return E_FAIL;
    }

    // Read in the data from the file
	if(isEmbedded)
	{
		DWORD offset = sizeof(XPR_HEADER);
		memcpy(m_pSysMemData, &pbData[offset], m_dwSysMemDataSize);
		offset += m_dwSysMemDataSize;
		memcpy(m_pVidMemData, &pbData[offset], m_dwVidMemDataSize);
	}
	else
	{
		if( !ReadFile( hFile, m_pSysMemData, m_dwSysMemDataSize, &dwNumBytesRead, NULL ) ||
			!ReadFile( hFile, m_pVidMemData, m_dwVidMemDataSize, &dwNumBytesRead, NULL ) )
		{
			ATG_PrintError( "Unable to read Xbox Packed Resource (.xpr) file\n" );
			CloseHandle( hFile );
			return E_FAIL;
		}

		// Done with the file
		CloseHandle( hFile );
	}

#ifdef _PC
    XGEndianSwapData( m_pSysMemData, m_pSysMemData, XGENDIAN_8IN32 );
#endif

    // Extract resource table from the header data
    m_dwNumResourceTags = *( DWORD* )( m_pSysMemData + 0 );
    m_pResourceTags = ( RESOURCE* )( m_pSysMemData + 4 );

#ifdef _PC
    XGEndianSwapMemory( m_pResourceTags, m_pResourceTags, XGENDIAN_8IN32, sizeof(DWORD), m_dwNumResourceTags * ( sizeof(RESOURCE) / sizeof(DWORD) ) );
#endif

    // Patch up the resources
    for( DWORD i = 0; i < m_dwNumResourceTags; i++ )
    {
        m_pResourceTags[i].strName = ( CHAR* )( m_pSysMemData + ( DWORD )m_pResourceTags[i].strName );

        // Fixup the texture memory
        if( ( m_pResourceTags[i].dwType & 0xffff0000 ) == ( RESOURCETYPE_TEXTURE & 0xffff0000 ) )
        {
            D3DTexture* pTexture = ( D3DTexture* )&m_pSysMemData[m_pResourceTags[i].dwOffset];
#ifdef _PC
            XGEndianSwapTextureHeader( pTexture );
#endif

            // Adjust Base address according to where memory was allocated
            XGOffsetBaseTextureAddress( pTexture, m_pVidMemData, m_pVidMemData );

            // Let PIX know the name of the texture
            PIXSetTextureName(pTexture, m_pResourceTags[i].strName);
        }
    }

    m_bInitialized = TRUE;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: GetResourceTags
// Desc: Retrieves the resource tags
//--------------------------------------------------------------------------------------
VOID PackedResource::GetResourceTags( DWORD* pdwNumResourceTags,
                                      RESOURCE** ppResourceTags ) const
{
    if( pdwNumResourceTags )
        ( *pdwNumResourceTags ) = m_dwNumResourceTags;

    if( ppResourceTags )
        ( *ppResourceTags ) = m_pResourceTags;
}


//--------------------------------------------------------------------------------------
// Name: Destroy
// Desc: Cleans up the packed resource data
//--------------------------------------------------------------------------------------
VOID PackedResource::Destroy()
{
    delete[] m_pSysMemData;
    m_pSysMemData = NULL;
    m_dwSysMemDataSize = 0L;

    if( m_pVidMemData != NULL )
        FreeContiguousMemory( m_pVidMemData );
    m_pVidMemData = NULL;
    m_dwVidMemDataSize = 0L;

    m_pResourceTags = NULL;
    m_dwNumResourceTags = 0L;

    m_bInitialized = FALSE;
}

//--------------------------------------------------------------------------------------
// Name: Initialized
// Desc: Indicates whether the packed resource has been successfully initialized
//--------------------------------------------------------------------------------------
BOOL PackedResource::Initialized() const
{
    return m_bInitialized;
}

} // namespace ATG
