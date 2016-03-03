/**
 *	Ayameマネージャ
 */

#ifndef ___AYAMEMANAGER_H___
#define ___AYAMEMANAGER_H___


//====================================================================
// INCLUDE
//====================================================================
#include <windows.h>
#include "Ayame.h"

#ifdef _UNICODE
	#define	DEFAULT_AYAME_PATH	L"ayame.dll"
#else
	#define	DEFAULT_AYAME_PATH	"ayame.dll"
#endif

//! Ayameマネージャ
class CAyameManager
{
protected:
	HINSTANCE						m_hAyame;
	AYAME_INITIALIZE_PROC			m_pAyameInitialize;
	AYAME_UNINITIALIZE_PROC			m_pAyameUninitialize;
	AYAME_CREATE_FROM_FILE_PROC		m_pAyameCreateFromFile;
	AYAME_CREATE_FROM_MEMORY_PROC	m_pAyameCreateFromMemory;
	AYAME_CREATE_FROM_URL_PROC		m_pAyameCreateFromUrl;
	AYAME_GETERROR_PROC				m_pAyameGetError;

public:
	//! コンストラクタ
	CAyameManager() {
		m_hAyame = NULL;
		m_pAyameInitialize = NULL;
		m_pAyameUninitialize = NULL;
		m_pAyameCreateFromFile = NULL;
		m_pAyameCreateFromMemory = NULL;
		m_pAyameCreateFromUrl = NULL;
		m_pAyameGetError = NULL;
	}
	//! デストラクタ
	~CAyameManager() {
		Release();
	}

	//! DLLの読み込み
	bool LoadDLL(HWND hWnd, TCHAR* pszPath = DEFAULT_AYAME_PATH, void **ppDirectSound = NULL) {
		if ( m_hAyame != NULL )
		{
			return false;
		}
		// DLLのロード
		m_hAyame = ::LoadLibrary( pszPath );
		if ( m_hAyame == NULL ) return false;

		// 関数のロード
		m_pAyameInitialize			= (AYAME_INITIALIZE_PROC)::GetProcAddress( m_hAyame, "Ayame_Initialize" );
		m_pAyameUninitialize		= (AYAME_UNINITIALIZE_PROC)::GetProcAddress( m_hAyame, "Ayame_Uninitialize" );
		m_pAyameCreateFromFile		= (AYAME_CREATE_FROM_FILE_PROC)::GetProcAddress( m_hAyame, "Ayame_CreateInstanceFromFile" );
		m_pAyameCreateFromMemory	= (AYAME_CREATE_FROM_MEMORY_PROC)::GetProcAddress( m_hAyame, "Ayame_CreateInstanceFromMemory" );
		m_pAyameCreateFromUrl       = (AYAME_CREATE_FROM_URL_PROC)::GetProcAddress( m_hAyame, "Ayame_CreateInstanceFromUrl" );
		m_pAyameGetError			= (AYAME_GETERROR_PROC)::GetProcAddress( m_hAyame, "Ayame_GetLastError" );

		if ( m_pAyameInitialize			== NULL ||
			 m_pAyameUninitialize		== NULL ||
			 m_pAyameCreateFromFile		== NULL ||
			 m_pAyameCreateFromMemory	== NULL ||
			 m_pAyameCreateFromUrl		== NULL ||
			 m_pAyameGetError			== NULL )
		{
			::FreeLibrary(m_hAyame);
			m_hAyame = NULL;
			return false;
		}

		// ドライバーの初期化
		return m_pAyameInitialize( hWnd, ppDirectSound );
	}

	//! DLLの開放
	void Release() {
		if ( m_hAyame != NULL )
		{
			m_pAyameUninitialize();
			::FreeLibrary(m_hAyame);
			m_hAyame = NULL;
		}
	}

	CAyame *CreateInstanceFromFile( const TCHAR *pFileName, unsigned long Start = 0, unsigned long Size = 0, unsigned long Flag = AYAME_LOADFLAG_GLOBAL )
	{
		if ( m_hAyame == NULL || pFileName == NULL )
			return NULL;
		return m_pAyameCreateFromFile( pFileName, Start, Size, Flag );
	}

	CAyame *CreateInstanceFromMemory( const void *pData, unsigned long Size = 0, unsigned long Flag = AYAME_LOADFLAG_GLOBAL )
	{
		if ( m_hAyame == NULL || pData == NULL )
			return NULL;
		return m_pAyameCreateFromMemory( pData, Size, Flag );
	}

	CAyame *CreateInstanceFromUrl( const TCHAR *pUrl, unsigned long Start = 0, unsigned long Size = 0, unsigned long Flag = AYAME_LOADFLAG_GLOBAL )
	{
		if ( m_hAyame == NULL || pUrl == NULL )
			return NULL;
		return m_pAyameCreateFromUrl( pUrl, Start, Size, Flag );
	}

	bool GetLastError( TCHAR *pErrStr, unsigned long Size )
	{
		if ( m_hAyame == NULL )
			return false;
		return m_pAyameGetError( pErrStr, Size );
	}
};

#endif // ___AYAMEMANAGER_H___


