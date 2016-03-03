/**
*/

#ifndef ___AYAME_H___
#define ___AYAME_H___


//====================================================================
// INCLUDE
//====================================================================
#include <tchar.h>
#include <dsound.h>


//====================================================================
// DEFINE
//====================================================================
#define AYAME_LOOP_INFINITY		(0xFFFFFFFF)			///< 無限ループ
#define	AYAME_LOADFLAG_GLOBAL	(1<<0)
#define	AYAME_LOADFLAG_STREAM	(0<<1)
#define	AYAME_LOADFLAG_STATIC	(1<<1)


//====================================================================
// TYPEDEF
//====================================================================
typedef bool (*AYAME_INITIALIZE_PROC)( HWND hWnd, void **ppDirectSound );																		///< ドライバー初期化
typedef void (*AYAME_UNINITIALIZE_PROC)( void );																								///< ドライバー解放
typedef bool (*AYAME_GETERROR_PROC)( TCHAR *pErrStr, unsigned long Size );																		///< エラー取得
typedef class CAyame *(*AYAME_CREATE_FROM_FILE_PROC)( const TCHAR *pFileName, unsigned long Start, unsigned long Size, unsigned long Flag );			///< ドライバーからオブジェクト取得
typedef class CAyame *(*AYAME_CREATE_FROM_MEMORY_PROC)( const void *pData, unsigned long Size, unsigned long Flag );										///< ドライバーからオブジェクト取得
typedef class CAyame *(*AYAME_CREATE_FROM_URL_PROC)( const TCHAR *pUrl, unsigned long Start, unsigned long Size, unsigned long Flag );				///< ドライバーからオブジェクト取得


//====================================================================
/**
	@brief 再生用クラス

	DLL用に純粋仮想関数だけの中身のないクラスです
*/
//====================================================================
class CAyame
{
protected:
	virtual ~CAyame(){}												///< デストラクタ
public:
	virtual bool __stdcall IsEmptyQueue( void )				= 0;	///< キューに登録があるかどうか
	virtual bool __stdcall IsReady( void ) const			= 0;	///< 初期化完了済みか？
	virtual bool __stdcall IsNotDecoder( void ) const		= 0;	///< デコーダが存在するかどうか？
	virtual bool __stdcall IsPlayStarted( void ) const		= 0;	///< 再生開始チェック
	virtual bool __stdcall IsPlay( void ) const				= 0;	///< 再生チェック
	virtual bool __stdcall IsPause( void ) const			= 0;	///< 一時停止中チェック
	virtual bool __stdcall Prefetch( void )					= 0;	///< プリフェッチ
	virtual bool __stdcall Predecode( void )				= 0;	///< プリデコード
	virtual bool __stdcall Combine( CAyame *pAyame )		= 0;	///< 連結
	virtual bool __stdcall Play( unsigned long nLoopCount = 0, unsigned long nLoopInSample = 0, unsigned long nStartPos = 0 )	= 0;	///< 再生
	virtual bool __stdcall Stop( void )						= 0;	///< 停止
	virtual bool __stdcall Pause( void )					= 0;	///< 一時停止
	virtual bool __stdcall SetVolume( float fParam )		= 0;	///< ボリューム設定
	virtual bool __stdcall SetPan( float fParam )			= 0;	///< パン設定
	virtual bool __stdcall SetFrequencyRate( float fParam )	= 0;	///< 周波数設定
	virtual bool __stdcall Release( void )					= 0;	///< 解放
	virtual bool __stdcall AddRef( void )					= 0;	///< 参照カウントインクリメント
	virtual unsigned long __stdcall GetPlayingPoint( void )	= 0;	///< おおよその再生位置の取得
	virtual unsigned long __stdcall GetTotalSample( void )	= 0;	///< トータルのサンプル数の取得
};


#endif // ___AYAME_H___
