/**
 *	@brief	MIDI再生クラス
 *	@file	midi.h
 *	@author	sazanta
 */
#include	<windows.h>
#include	<mmsystem.h>

class	CMidiEvent;

enum eEventType
{
	EVT_LOAD,
	EVT_PLAY,
	EVT_STOP,
	EVT_PAUSE,
	EVT_RELEASE,
	EVT_MAX,
};

struct EVENT_QUE_DATA_LOAD
{
	int nType;				// 0-Memory 1-File 2-Url
	char *pData;			// nTypeが0以外の場合はdeleteする
	unsigned long nSize;
	unsigned long nStart;
};

struct EVENT_QUE_DATA_PLAY
{
	unsigned long nStartPos;
	unsigned long nLoopCount;
};

struct EVENT_QUE
{
	eEventType	EventType;
	bool		bUse;
  union{
	EVENT_QUE_DATA_LOAD		Data_Load;
	EVENT_QUE_DATA_PLAY		Data_Play;
  };
};


//! MIDI再生クラス
class	CMidi
{
  protected:
			CRITICAL_SECTION	m_stCriticalSection;	//!< クリティカルセクション
				HANDLE			m_hThread;				//!< スレッドハンドル
				unsigned long	m_nThreadId;			//!< スレッドID
				HANDLE			m_hEvent;				//!< イベント
				EVENT_QUE		m_stQue[EVT_MAX];		//!< イベントキュー

				CMidiEvent*		m_pHeader;				//!< データ先頭アドレス
				CMidiEvent*		m_pNowPoint;			//!< 現在のデータポインタ
		union {
				unsigned short	m_nAttribute;			//!< 属性
			struct {
				unsigned short	m_nAttr_Pause	: 1;	//!< ポーズ中フラグ
				unsigned short	m_nAttr_UpdateV	: 1;	//!< 音量更新フラグ
				unsigned short	m_nAttr_UpdateP	: 1;	//!< パンポット更新フラグ
				unsigned short	m_nIsPlayQue	: 1;	//!< 再生準備中フラグ
				unsigned short	m_nAttr_pad		:12;
			};
		};
				unsigned short	m_nTimeBias;			//!< 分解能バイアス
				unsigned long	m_nTime;				//!< 分解能
				unsigned long	m_nTempo;				//!< テンポ
				unsigned long	m_nLoop;				//!< ループ回数
				unsigned long	m_nNowTime;				//!< 現在の再生時間
				unsigned long	m_nNowTimeRest;			//!< 現在の再生余り時間
				unsigned long	m_nNextTime;			//!< 次のイベントを送出する時間
				unsigned long	m_nPrevTime;			//!< 前回処理を行った時間
				unsigned long	m_nTimeRest;			//!< 余り時間
				unsigned long	m_nTotalTime;			//!< トータル時間
				unsigned int	m_nTimerID;				//!< タイマーID
				HMIDIOUT		m_hMidiOut;

				unsigned char	m_nPanpot[16];			//!< チャンネルパン
				unsigned short	m_nVolume[16];			//!< チャンネルボリューム
				unsigned short	m_nMasterVolume;		//!< マスターボリューム
				unsigned short	m_nMuteFlag;			//!< ミュートフラグ
				unsigned char	m_nMasterPanpot;		//!< マスターパンポット
	volatile	bool			m_bEnd;

  protected:
	virtual						~CMidi(void);
				bool			AddQue(EVENT_QUE* pQue);
				void			_Release(void);
				bool			_AttachMidiFile(const char* pFileName, unsigned int nOfs, unsigned int nSize);
				bool			_AttachMidiData(const unsigned char* pData, unsigned int nSize);
				bool			_Play(unsigned int nLoop);
				bool			_Stop(void);
				void			_Pause(void);
				void			ReleaseMain(CMidiEvent* pEvent);
				bool			ReadTrack(unsigned char** ppData, CMidiEvent** lplpEvent, unsigned int& nSize);
				CMidiEvent*		Marge(CMidiEvent** lplpEvent, unsigned short nTrack);
				unsigned char*	ReadDelta(unsigned char* pData, unsigned long& nDelta, unsigned int& nSize);
				unsigned long	CalcTotalTime(void)	const;
				void			ThreadMain();
	static void CALLBACK		TimeCallbackProc(unsigned int nID, unsigned int nMsg, unsigned long nUser, unsigned long nReserve1, unsigned long nReserve2);
	static unsigned long CALLBACK ThreadProc(void *pParameter);

  public:
								CMidi(void);

				void			Release(void);
				bool			AttachMidiFile(const TCHAR* pFileName, unsigned int nOfs = 0, unsigned int nSize = 0);
				bool			AttachMidiData(const unsigned char* pData, unsigned int nSize);

				bool			Play(unsigned long nLoop = 0xffffffff);
				bool			Stop(void);
				bool			Pause(void);
								//! 再生中かどうか
								/**
								 *	@return	再生中の場合はtrueを返す
								 */
				bool			IsPlaying(void)	const	{
									return (m_nIsPlayQue == true || m_pNowPoint != NULL);
								}
								//! 総時間の取得
								/**
								 *	@return	総時間(単位はms)
								 */
				unsigned long	GetTotalTime(void)	const	{
									return (m_pHeader != NULL)? m_nTotalTime : 0;
								}
								//! 再生時間の取得
								/**
								 *	@return 再生時間(単位はms)
								 */
				unsigned long	GetPlayingTime(void)	const	{
									return (m_pHeader != NULL)? m_nNowTime : 0;
								}

								//! ループ回数の設定
				void			SetLoopCounter(unsigned int nLoop) {
									if (m_pHeader != NULL) {
										m_nLoop = nLoop;
									}
								}
								//! ループ回数の取得
				unsigned long	GetLoopCounter(void)	const	{
									return (m_pHeader != NULL)? m_nLoop : 0;
								}

				unsigned short	SetMuteFlag(unsigned short nFlag);
				unsigned short	SetMuteFlag(unsigned int nChannel, bool bFlag);
								//! ミュートフラグの取得
				unsigned short	GetMuteFlag(void)	const	{
									return m_nMuteFlag;
								}

								//! テンポスピードの設定
								/**
								 *	@param	nSpeed	[in] テンポスピード(1000=1.0)
								 */
				void			SetTimeSpeed(unsigned long nSpeed) {
									m_nTimeBias = (unsigned short)((nSpeed == 0)? 1 : nSpeed);
								}
								//! テンポスピードの取得
								/**
								 *	@return	テンポスピード(1000=1.0)
								 */
				unsigned long	GetTimeSpeed(void)	const	{
									return m_nTimeBias;
								}

								//! テンポの取得
				unsigned long	GetRealTempo(void)	const	{
									return m_nTempo;
								}
								//! テンポの取得
				unsigned long	GetTempo(void)	const	{
									return (60 * 1000000) / m_nTempo;
								}

								//! マスターボリュームの設定
								/**
								 *	@param	nVol	[in] マスターボリューム(0～256)
								 */
				void			SetMasterVolume(unsigned short nVol) {
									m_nMasterVolume = ((nVol > 256)? 256 : nVol);
									m_nAttr_UpdateV = 1;
								}
								//! マスターボリュームの取得
				unsigned short	GetMasterVolume(void)	const	{
									return m_nMasterVolume;
								}

								//! チャンネルボリュームの設定
								/**
								 *	各チャンネルのボリュームを指定します。
								 *	各チャンネルのボリュームはMIDIデータによって上書きされる可能性があります。
								 *
								 *	@param	nChannel	[in] チャンネル(0～15)
								 *	@param	nVol		[in] チャンネルボリューム(0～127)
								 */
				void			SetChannelVolume(unsigned long nChannel, unsigned short nVol) {
									if (nChannel >= 16)		return;
									m_nVolume[nChannel] = (nVol > 127)? 127 : nVol;
									m_nAttr_UpdateV = 1;
								}
								//! チャンネルボリュームの取得
								/**
								 *	@return		チャンネルボリューム
								 */
				unsigned short	GetChannelVolume(unsigned long nChannel)	const	{
									return (nChannel < 16)? m_nVolume[nChannel] : 100;
								}

								//! マスターパンポットの設定
								/**
								 *	@param	nPan	[in] マスターパンポット(0～127)
								 */
				void			SetMasterPanpot(unsigned char nPan) {
									m_nMasterPanpot = ((nPan > 127)? 127 : nPan);
									m_nAttr_UpdateP = 1;
								}
								//! マスターパンポットの取得
				unsigned char	GetMasterPanpot(void)	const	{
									return m_nMasterPanpot;
								}

								//! チャンネルパンポットの設定
								/**
								 *	各チャンネルのパンポットを指定します。
								 *	各チャンネルのパンポットはMIDIデータによって上書きされる可能性があります。
								 *
								 *	@param	nChannel	[in] チャンネル(0～15)
								 *	@param	nPan		[in] チャンネルパンポット(0～127)
								 */
				void			SetChannelPanpot(unsigned long nChannel, unsigned char nPan) {
									if (nChannel >= 16)		return;
									m_nPanpot[nChannel] = (nPan > 127)? 127 : nPan;
									m_nAttr_UpdateP = 1;
								}
								//! チャンネルパンポットの取得
								/**
								 *	@return		チャンネルパンポット
								 */
				unsigned char	GetChannelPanpot(unsigned long nChannel)	const	{
									return (nChannel < 16)? m_nPanpot[nChannel] : 64;
								}

	static		bool			CheckMidiHeader(const unsigned char* pData);
};


/*  Bottom of midi.h  */
