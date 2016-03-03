/**
 *	@brief	MIDI再生クラス
 *	@file	midi.cpp
 *	@author	sazanta
 */
#include	"midi.h"

#pragma comment (lib, "winmm.lib")


//! MIDIイベント保持クラス
class CMidiEvent
{
  public:
	CMidiEvent*			lpNext;		// 次のイベントへのポインタ
	unsigned long		dwDelta;	// デルタタイム
	unsigned char		state;		// ステータスバイト
	unsigned char		data1;		// 第一データバイト
	unsigned char		data2;		// 第二データバイト
	unsigned char		type;		// タイプ
	unsigned char*		lpData;		// 可変長データ
	long				nData;		// データ長

						CMidiEvent(void) {
							lpNext = NULL;
							lpData = NULL;
							nData  = 0;
						}
						~CMidiEvent(void) {
							delete[] lpData;
						}
};


//! コンストラクタ
CMidi::CMidi(void)
{
	m_pHeader    = NULL;
	m_nAttribute = 0;
	m_nTime      = 120 * 1000;
	m_nTimeBias  = 1000;
	m_nTempo     = 500000;
	m_nTimerID   = 0;
	m_hMidiOut   = NULL;
	for (int i = 0; i < 16; i++) {
		m_nPanpot[i] =  64;
		m_nVolume[i] = 100;
	}
	m_nMasterVolume = 256;
	m_nMuteFlag     = 0;
	m_nMasterPanpot = 64;

	// クリティカルセクションの生成
	::InitializeCriticalSection(&m_stCriticalSection);
	::ZeroMemory(&m_stQue, sizeof(m_stQue));
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_bEnd   = false;
	// 処理スレッド生成
	m_hThread = ::CreateThread(NULL, 0, ThreadProc, this, 0, &m_nThreadId);
}


//! デストラクタ
CMidi::~CMidi(void)
{
	// クリティカルセクションの破棄
	::DeleteCriticalSection(&m_stCriticalSection);
	::CloseHandle(m_hEvent);
}


//! MIDIデータの解放
void	CMidi::Release(void)
{
	::EnterCriticalSection(&m_stCriticalSection);
	m_bEnd = true;
	::SetEvent(m_hEvent);
	::LeaveCriticalSection( &m_stCriticalSection );
	::WaitForSingleObject(m_hThread, INFINITE);
}


//! MIDIファイルのアタッチ
/**
 *	MIDIファイルのアタッチを行います。
 *
 *	@param	pFileName	[in] SMFファイルパス
 *	@param	nOfs		[in] SMFファイルオフセット
 *	@param	nSize		[in] SMFファイルサイズ(0の場合は自動計算)
 *
 *	@return	正しくアタッチできた場合はtrueを返す
 */
bool	CMidi::AttachMidiFile(const TCHAR* pFileName, unsigned int nOfs, unsigned int nSize)
{
	EVENT_QUE Que = { EVT_LOAD };

	size_t	nLen;
#ifdef _UNICODE
	nLen = (wcslen(pFileName) + 1) * 2;
#else
	nLen = strlen(pFileName) + 1;
#endif
	Que.Data_Load.pData  = new char[nLen];
	::CopyMemory(Que.Data_Load.pData, pFileName, nLen);
	Que.Data_Load.nSize  = nSize;
	Que.Data_Load.nStart = nOfs;
	Que.Data_Load.nType  = 1;

	// キューに追加
	return AddQue(&Que);
}


//! MIDIデータのアタッチ
/**
 *	MIDIデータのアタッチを行います。
 *	関数を抜けた後は、データを削除しても構いません。
 *
 *	@param	pData	[in] SMFデータポインタ
 *	@param	nSize	[in] SMFデータサイズ
 *
 *	@return	正しくアタッチできた場合はtrueを返す
 */
bool	CMidi::AttachMidiData(const unsigned char* pData, unsigned int nSize)
{
	EVENT_QUE Que = { EVT_LOAD };

	Que.Data_Load.pData = new char[nSize];
	if (Que.Data_Load.pData == NULL)		return false;
	::CopyMemory(Que.Data_Load.pData, pData, nSize);
	Que.Data_Load.nSize = nSize;
	Que.Data_Load.nType = 0;

	// キューに追加
	return AddQue(&Que);
}


//! 再生
bool	CMidi::Play(unsigned long nLoop)
{
	EVENT_QUE Que = { EVT_PLAY };

	Que.Data_Play.nStartPos = 0;
	Que.Data_Play.nLoopCount = nLoop;

	// キューに追加
	return AddQue(&Que);
}


//! 停止
bool	CMidi::Stop(void)
{
	EVENT_QUE Que = { EVT_STOP };

	// キューに追加
	return AddQue(&Que);
}


//! ポーズ
bool	CMidi::Pause(void)
{
	EVENT_QUE Que = { EVT_PAUSE };

	// キューに追加
	return AddQue(&Que);
}


//! ミュートフラグの設定
/**
 *	@param	nFlag	[in] ミュートするチャンネルの論理和
 *
 *	@return	設定を行う前の値
 */
unsigned short	CMidi::SetMuteFlag(unsigned short nFlag)
{
	unsigned short	nOld = m_nMuteFlag;
	if (m_nMuteFlag != nFlag) {
		m_nMuteFlag = nFlag;
		m_nAttr_UpdateV = 1;
	}

	return nOld;
}


//! ミュートフラグの設定
unsigned short	CMidi::SetMuteFlag(unsigned int nChannel, bool bFlag)
{
	unsigned short	nOld = m_nMuteFlag;
	if (nChannel < 16) {
		if (bFlag == false)	m_nMuteFlag &= ~(1<<nChannel);
		else				m_nMuteFlag |= (1<<nChannel);
	}
	if (m_nMuteFlag != nOld) {
		m_nAttr_UpdateV = 1;
	}

	return nOld;
}


//! MIDIヘッダのチェック
/**
 *	@return	MIDIヘッダが正しければtrueを返す
 */
bool	CMidi::CheckMidiHeader(const unsigned char* pData)
{
	if (pData[0] != 'M' || pData[1] != 'T' || pData[2] != 'h' || pData[3] != 'd') {
		return false;
	}
	pData += 4;
	if (pData[0] != 0x00 || pData[1] != 0x00 || pData[2] != 0x00 || pData[3] != 0x06) {
		return false;
	}

	return true;
}


bool	CMidi::AddQue(EVENT_QUE* pQue)
{
	// クリティカルセクションに入る
	::EnterCriticalSection(&m_stCriticalSection);
	if (m_bEnd == true) {
		// クリティカルセクションから抜ける
		::LeaveCriticalSection( &m_stCriticalSection );
		return true;
	}

	bool	bRet = false;
	int		nSlot = 0;
	for (; ((nSlot < EVT_MAX) && (m_stQue[nSlot].bUse == true)); nSlot++) {
		if (m_stQue[nSlot].EventType == pQue->EventType) {
			// 先発の同じ要求は却下する
			if (m_stQue[nSlot].EventType == EVT_LOAD && m_stQue[nSlot].Data_Load.nType != 0) {
				delete[] m_stQue[nSlot].Data_Load.pData;
			}
			::MoveMemory(&m_stQue[nSlot], &m_stQue[nSlot + 1], sizeof(EVENT_QUE) * (EVT_MAX - (nSlot + 1)));
			m_stQue[EVT_MAX - 1].bUse = false;
			nSlot--;
		}
	}
	// キューは一杯ではない
	if (nSlot != EVT_MAX) {
		// データコピー
		::CopyMemory(&m_stQue[nSlot], pQue, sizeof(EVENT_QUE));
		// 使用中フラグON
		m_stQue[nSlot].bUse = true;
		// プレイキューの場合は、フラグを立てる
		if (pQue->EventType == EVT_PLAY) m_nIsPlayQue = true;
		// イベントON
		::SetEvent(m_hEvent);
		// 処理成功
		bRet = true;
	}

	// クリティカルセクションから抜ける
	::LeaveCriticalSection(&m_stCriticalSection);

	return bRet;
}


//! データの解放
void	CMidi::_Release(void)
{
	_Stop();
	if (m_pHeader != NULL) {
		ReleaseMain(m_pHeader);
		m_pHeader = NULL;
	}
}


bool	CMidi::_AttachMidiFile(const char* pFileName, unsigned int nOfs, unsigned int nSize)
{
	HANDLE	hFile = ::CreateFile((LPCTSTR)pFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	if (nSize == 0) {
		nSize = ::GetFileSize(hFile, NULL);
		if (nSize <= nOfs) {
			::CloseHandle(hFile);
			return false;
		}
		nSize -= nOfs;
	}

	char*	pBuff = new char[nSize];
	if (pBuff == NULL) {
		::CloseHandle(hFile);
		return false;
	}

	if (::SetFilePointer(hFile, nOfs, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		delete[] pBuff;
		::CloseHandle(hFile);
		return false;
	}
	unsigned long	nReadSize;
	::ReadFile(hFile, pBuff, nSize, &nReadSize, NULL);
	::CloseHandle(hFile);
	if (nReadSize != nSize) {
		delete[] pBuff;
		return false;
	}
	bool	bRet = _AttachMidiData((const unsigned char*)pBuff, nSize);
	delete[] pBuff;

	return bRet;
}


bool	CMidi::_AttachMidiData(const unsigned char* pData, unsigned int nSize)
{
	if (nSize <= 14 || CheckMidiHeader(pData) == false) {
		// データ不正
		return false;
	}

	_Release();
	unsigned char*	pTmpData = (unsigned char*)(pData + 8);
	unsigned short	nFormat = ((unsigned short)pTmpData[0] << 8) | pTmpData[1];
	pTmpData += 2;

	unsigned short	nTrack = ((unsigned short)pTmpData[0] << 8) | pTmpData[1];
	pTmpData += 2;

	unsigned short	nTime = ((unsigned short)pTmpData[0] << 8) | pTmpData[1];
	if (nTime == 0 || nTrack > 128) {
		// データ不正
		return false;
	}
	pTmpData += 2;
	nSize -= 14;

	CMidiEvent*	lpaEvent[128];
	for (int i = 0; i < nTrack; i++) {
		if (ReadTrack(&pTmpData, &lpaEvent[i], nSize) == false) {
			// トラック読み込み失敗
			for (int j = 0; j < (i + 1); j++) {
				ReleaseMain(lpaEvent[j]);
			}
			return false;
		}
	}

	if (nFormat == 1 && nTrack != 1) {
		if ((m_pHeader = Marge(lpaEvent, nTrack)) == NULL) {
			return false;
		}
	} else {
		m_pHeader = lpaEvent[0];
	}
	m_nTempo = 500000;
	m_nTime  = nTime * 1000;
	m_nTotalTime = CalcTotalTime();

	return true;
}


bool	CMidi::_Play(unsigned int nLoop)
{
	if (m_pNowPoint != NULL || m_pHeader == NULL) {
		return false;
	}
	MMRESULT mr;
	if (m_hMidiOut == NULL) {
		mr = midiOutOpen(&m_hMidiOut, MIDIMAPPER, 0, 0, CALLBACK_NULL);
		if (mr != MMSYSERR_NOERROR) {
			return false;
		}
	}
	TIMECAPS	stCaps;
	mr = timeGetDevCaps(&stCaps, sizeof(TIMECAPS));
	if (mr != TIMERR_NOERROR) {
		return false;
	}
	m_pNowPoint    = m_pHeader;
	m_nTempo       = 500000; // テンポのデフォルト値
	m_nNextTime    = 0;
	m_nPrevTime    = timeGetTime();
	m_nNowTime     = 0;
	m_nNowTimeRest = 0;
	m_nTimeRest    = 0;
	m_nLoop        = nLoop;
	for (int i = 0; i < 16; i++) {
		m_nVolume[i] = 100;
	}
	m_nAttr_UpdateV = 1;
	m_nAttr_UpdateP = 1;
	m_nIsPlayQue    = 0;
	m_nTimerID = timeSetEvent(stCaps.wPeriodMin, 0, TimeCallbackProc, (unsigned long)this, TIME_PERIODIC);
	if (m_nTimerID == NULL) {
		return false;
	}

	return true;
}


bool	CMidi::_Stop(void)
{
	if (m_nTimerID != 0) {
		timeKillEvent(m_nTimerID);
		m_nTimerID = 0;
	}
	if (m_hMidiOut != NULL) {
		midiOutReset(m_hMidiOut); // 再生中の音を消す
		midiOutClose(m_hMidiOut);
		m_hMidiOut = NULL;
	}
	m_nNowTime    = 0;
	m_pNowPoint   = NULL;
	m_nAttr_Pause = 0;

	return true;
}


void	CMidi::_Pause(void)
{
	if (m_pNowPoint != NULL) {
		m_nAttr_Pause = (m_nAttr_Pause == 0)? 1 : 0;
		midiOutReset(m_hMidiOut); // 再生中の音を消す
		m_nAttr_UpdateV = 1;
	}
}


//! イベントクラスの解放
void	CMidi::ReleaseMain(CMidiEvent* pEvent)
{
	while (pEvent != NULL) {
		CMidiEvent*	pTmp = pEvent;
		pEvent = pEvent->lpNext;
		delete pTmp;
	}
}


//! トラックの解析
bool	CMidi::ReadTrack(unsigned char** ppData, CMidiEvent** lplpEvent, unsigned int& nSize)
{
	unsigned char*	pData = *ppData;
	unsigned char	statePrev = 0; // 前のイベントのステータスバイト
	CMidiEvent*		lpEvent;

	if (pData[0] != 'M' || pData[1] != 'T' || pData[2] != 'r' || pData[3] != 'k' || nSize <= 8) {
		// データ不正
		*lplpEvent = NULL;
		return false;
	}
	nSize -= 4 + 4;
	unsigned long	nLen = pData[7] | ((unsigned long)pData[6] << 8) | ((unsigned long)pData[5] << 16) | ((unsigned long)pData[4] << 24);
	if (nLen > nSize) {
		// データ不正
		*lplpEvent = NULL;
		return false;
	}
	pData += 4 + 4;

	lpEvent    = new CMidiEvent();	// 最初のイベントのメモリを確保
	*lplpEvent = lpEvent;			// lplpEventは常に最初のイベントを指す
	while (1) {
		// デルタタイムを読み込む
		if ((pData = ReadDelta(pData, lpEvent->dwDelta, nSize)) == NULL) {
			// データ不正
			return false;
		}

		lpEvent->state = *pData;
		if ((lpEvent->state & 0x80) == 0) {	// ランニングステータスか
			lpEvent->state = statePrev;		// 一つ前のイベントのステータスバイトを代入
		} else {
			pData++;
			nSize--;
		}

		switch (lpEvent->state & 0xf0) {	// ステータスバイトを基にどのイベントか判別
		case 0x80:
		case 0x90:
		case 0xa0:
		case 0xb0:
		case 0xe0:
			lpEvent->data1 = *pData++;
			lpEvent->data2 = *pData++;
			nSize -= 2;
			break;
		case 0xc0:
		case 0xd0:
			lpEvent->data1 = *pData++;
			lpEvent->data2 = 0;
			nSize--;
			break;

		case 0xf0:
			unsigned long	dw;
			if (lpEvent->state == 0xf0 || lpEvent->state == 0xf7) {
				// SysExイベント
				if ((pData = ReadDelta(pData, dw, nSize)) == NULL) {
					return false;
				}
				if (lpEvent->state == 0xf0)	dw++;
				lpEvent->nData  = dw;
				lpEvent->lpData = new unsigned char[dw];
				if (lpEvent->state == 0xf0) {
					lpEvent->lpData[0] = 0xf0;						// 可変長データの先頭は0xF0
					dw--;
					memcpy(lpEvent->lpData + 1, pData, dw);
				} else {
					memcpy(lpEvent->lpData, pData, dw);
				}
				pData += dw;
				nSize -= dw;
			} else if (lpEvent->state == 0xff) {
				// メタイベント
				lpEvent->type = *pData++;		// typeの取得
				nSize--;
				if ((pData = ReadDelta(pData, dw, nSize)) == NULL) {
					// データ不正
					return false;
				}

				lpEvent->nData = dw;
				if (dw != 0) {
					lpEvent->lpData = new unsigned char[dw];
					memcpy(lpEvent->lpData, pData, dw);
					pData += dw;
					nSize -= dw;
				}

				if (lpEvent->type == 0x2f) {
					// トラックの終端
					*ppData = pData;
					return true;
				}
			}
			break;

		default:
			// ステータス不正
			return false;
		}
		statePrev = lpEvent->state;

		lpEvent->lpNext = new CMidiEvent();
		lpEvent         = lpEvent->lpNext;
		if (lpEvent == NULL) {
			// メモリ不足
			return false;
		}
	}

	return false;
}


//! デルタタイムの取得
unsigned char*	CMidi::ReadDelta(unsigned char* pData, unsigned long& nDelta, unsigned int& nSize)
{
	nDelta = 0;
	for (int i = 0; i < sizeof(unsigned long); i++) {
		unsigned char	nTmp = *pData++;
		if (nSize-- == 0) {
			return NULL;
		}
		nDelta = (nDelta << 7) | (nTmp & 0x7f);
		if ((nTmp & 0x80) == 0) {
			break;		// MSBが立っていないならば、次のバイトはデルタタイムではないので抜ける
		}
	}

	return pData;
}


//! トラックデータのマージ
CMidiEvent*	CMidi::Marge(CMidiEvent** lplpEvent, unsigned short nTrack)
{
	unsigned long	dwPrevAbsolute = 0;								// 一つ前の絶対時間
	CMidiEvent*		lpEvent        = new CMidiEvent();				// 現在のイベント
	unsigned long*	lpdwTotal      = new unsigned long[nTrack];		// 各トラックの絶対時間
	CMidiEvent**	pEventBase     = new CMidiEvent*[nTrack];

	if (lpEvent == NULL || lpdwTotal == NULL || pEventBase == NULL) {
		// メモリ不足
		delete lpEvent;
		delete[] lpdwTotal;
		delete[] pEventBase;
		return NULL;
	}

	memcpy(pEventBase, lplpEvent, sizeof(CMidiEvent*) * nTrack);
	memset(lpdwTotal, 0, sizeof(unsigned long) * nTrack);
	CMidiEvent*	lpHead = lpEvent;
	while (1) {
		int				nIndex     = -1;				// トラックのインデックス
		unsigned long	dwAbsolute = (unsigned long)-1; // 絶対時間(0xFFFFFFFF)

		// 最も絶対時間が低いイベントを見つける
 		for (int i = 0; i < nTrack; i++) {
			if (pEventBase[i]->lpNext == NULL) {
				continue;		// トラックの終端まで走査した
			}

			if ((lpdwTotal[i] + pEventBase[i]->dwDelta) < dwAbsolute) {
				nIndex = i;
				dwAbsolute = lpdwTotal[i] + pEventBase[i]->dwDelta;
			}
		}

		if (nIndex == -1) {
			// 全てのトラックを走査したのでTrackOfEndを付加する
			lpEvent->state   = 0xff;
			lpEvent->data1   = 0x2f;
			lpEvent->data2   = 0x00;
			lpEvent->dwDelta = 0;
			break;
		}

		if (pEventBase[nIndex]->state != 0xff || pEventBase[nIndex]->data1 != 0x2f) {		// TrackOfEndははじく
			lpEvent->state   = pEventBase[nIndex]->state;
			lpEvent->data1   = pEventBase[nIndex]->data1;
			lpEvent->data2   = pEventBase[nIndex]->data2;
			lpEvent->type    = pEventBase[nIndex]->type;
			lpEvent->nData   = pEventBase[nIndex]->nData;
			lpEvent->dwDelta = dwAbsolute - dwPrevAbsolute;
			if (lpEvent->nData != 0) {
				lpEvent->lpData = new unsigned char[lpEvent->nData];
				if (lpEvent->lpData == NULL) {
					// メモリ不足
					ReleaseMain(lpHead);
					for (int j = 0; j < nTrack; j++) {
						ReleaseMain(pEventBase[j]);
					}
					delete[] lpdwTotal;
					delete[] pEventBase;
					return NULL;
				}
				memcpy(lpEvent->lpData, pEventBase[nIndex]->lpData, lpEvent->nData);
			}
			lpEvent->lpNext = new CMidiEvent();
			lpEvent         = lpEvent->lpNext;
			if (lpEvent == NULL) {
				// メモリ不足
				ReleaseMain(lpHead);
				for (int j = 0; j < nTrack; j++) {
					ReleaseMain(pEventBase[j]);
				}
				delete[] lpdwTotal;
				delete[] pEventBase;
				return NULL;
			}
		}
		dwPrevAbsolute = dwAbsolute;
		lpdwTotal[nIndex] += pEventBase[nIndex]->dwDelta; // 各トラックの絶対時間を更新
		pEventBase[nIndex] = pEventBase[nIndex]->lpNext;
	}
	for (int j = 0; j < nTrack; j++) {
		ReleaseMain(lplpEvent[j]);
	}
	delete[] pEventBase;
	delete[] lpdwTotal;

	return lpHead;
}


//! 総時間の計算
unsigned long	CMidi::CalcTotalTime(void)	const
{
	if (m_pHeader == NULL) {
		return 0;
	}
	unsigned long	nTotalTime = 0;
	unsigned long	nTempo = 500000;
	unsigned long	nRest = 0;
	unsigned long	nTime = m_nTime;
	CMidiEvent*		pData = m_pHeader;
	while (pData != NULL) {
		unsigned long	nDelta = pData->dwDelta;
		if (pData->state == 0xff && pData->type == 0x51) {
			// セットテンポ
			nTempo = (unsigned long)(pData->lpData[2] | ((unsigned long)pData->lpData[1] << 8) | ((unsigned long)pData->lpData[0] << 16));
		}
		if (nDelta != 0 ) {
			unsigned long	nTmp   = (nDelta * nTempo) + nRest;
			unsigned long	nMtime = nTmp / nTime;
			nRest = nTmp % nTime;
			nTotalTime += nMtime;
		}
		pData = pData->lpNext;
	}

	return nTotalTime;
}


//! コールバック関数
void CALLBACK CMidi::TimeCallbackProc(unsigned int nID, unsigned int nMsg, unsigned long nUser, unsigned long nReserve1, unsigned long nReserve2)
{
	CMidi*	pClass = (CMidi*)nUser;

	if (pClass == NULL || pClass->m_pNowPoint == NULL)	return;

	unsigned long	nTime = timeGetTime();
	if (pClass->m_nAttr_Pause != 0) {
		// ポーズ中
		pClass->m_nPrevTime  = nTime;
		return;
	}

	// ボリューム設定
	if (pClass->m_nAttr_UpdateV != 0) {
		for (int i = 0; i < 16; i++) {
			unsigned long	nVol;
			if ((pClass->m_nMuteFlag & (1<<i)) == 0) {
				nVol = ((pClass->m_nMasterVolume * pClass->m_nVolume[i]) << 8) & 0x7f0000;
			} else {
				nVol = 0;
			}
			midiOutShortMsg(pClass->m_hMidiOut, nVol | (0x07 << 8) | (0xb0 + i));
		}
		pClass->m_nAttr_UpdateV = 0;
	}
	// パンポット設定
	if (pClass->m_nAttr_UpdateP != 0) {
		for (int i = 0; i < 16; i++) {
			long	nPan = (long)pClass->m_nPanpot[i] + ((pClass->m_nMasterPanpot * 2) - 128);
			nPan  = (nPan < 0)? 0 : ((nPan > 127)? 127 : nPan);
			midiOutShortMsg(pClass->m_hMidiOut, (nPan << 16) | (0x0a << 8) | (0xb0 + i));
		}
		pClass->m_nAttr_UpdateP = 0;
	}

	unsigned long	nTmp = (((nTime - pClass->m_nPrevTime) * pClass->m_nTimeBias) + pClass->m_nNowTimeRest);
	pClass->m_nNowTimeRest = (nTmp % 1000);
	pClass->m_nNowTime += nTmp / 1000;
	pClass->m_nPrevTime = nTime;
	while (1) {
		if (pClass->m_nNextTime > pClass->m_nNowTime)		break;
		if (pClass->m_pNowPoint->state == 0xff) {
			// メタイベント
			if (pClass->m_pNowPoint->type == 0x51) {
				// セットテンポ
				pClass->m_nTempo = (unsigned long)(pClass->m_pNowPoint->lpData[2] | ((unsigned long)pClass->m_pNowPoint->lpData[1] << 8) | ((unsigned long)pClass->m_pNowPoint->lpData[0] << 16));
			}
		} else if (pClass->m_pNowPoint->state == 0xf0) {
			// SysExイベント
			MIDIHDR	mh = {0};

			mh.lpData          = (LPSTR)pClass->m_pNowPoint->lpData;
			mh.dwFlags         = 0;
			mh.dwBufferLength  = pClass->m_pNowPoint->nData;
			mh.dwBytesRecorded = pClass->m_pNowPoint->nData;

			midiOutPrepareHeader(pClass->m_hMidiOut, &mh, sizeof(MIDIHDR));
			midiOutLongMsg(pClass->m_hMidiOut, &mh, sizeof(MIDIHDR));
			while ((mh.dwFlags & MHDR_DONE) == 0);
			midiOutUnprepareHeader(pClass->m_hMidiOut, &mh, sizeof(MIDIHDR));
		} else {
			// MIDIイベント
			unsigned char	nStatus = pClass->m_pNowPoint->state;
			unsigned long	nData2  = pClass->m_pNowPoint->data2;
			if ((nStatus & 0xf0) == 0xb0) {
				int	nCh = (nStatus & 0x0f);
				if (pClass->m_pNowPoint->data1 == 0x07) {
					// ボリューム
					pClass->m_nVolume[nCh] = (unsigned short)nData2;
					if ((pClass->m_nMuteFlag & (1<<nCh)) != 0) {
						// ミュート
						nData2 = 0;
					} else {
						// ベロシティ計算
						nData2 *= pClass->m_nMasterVolume;
						nData2 >>= 8;
					}
				} else if (pClass->m_pNowPoint->data1 == 0x0a) {
					// パンポット
					pClass->m_nPanpot[nCh] = (unsigned char)nData2;
					long	nPan = (long)nData2 + ((pClass->m_nMasterPanpot * 2) - 128);
					nPan = (nPan < 0)? 0 : ((nPan > 127)? 127 : nPan);
					nData2 = (unsigned long)nPan;
				}
			}
			unsigned long	dwMsg = (unsigned long)(nStatus | ((unsigned long)pClass->m_pNowPoint->data1 << 8) | (nData2 << 16));
			midiOutShortMsg(pClass->m_hMidiOut, dwMsg);
		}
		pClass->m_pNowPoint = pClass->m_pNowPoint->lpNext;
		if (pClass->m_pNowPoint != NULL) {
			unsigned long	nTmp   = (pClass->m_pNowPoint->dwDelta * pClass->m_nTempo) + pClass->m_nTimeRest;
			unsigned long	nMtime = nTmp / pClass->m_nTime;
			pClass->m_nTimeRest = nTmp % pClass->m_nTime;
			if (nMtime != 0) {
				pClass->m_nNextTime += nMtime;
			}
		} else {
			// データ終了
			if ((pClass->m_nLoop + 1) > 1) {
				pClass->m_nLoop--;
			}
			if (pClass->m_nLoop == 0) {
				timeKillEvent(pClass->m_nTimerID);
				pClass->m_nTimerID = 0;
				break;
			} else {
				pClass->m_nNextTime = 0;
				pClass->m_nNowTime  = 0;
				pClass->m_nNowTimeRest = 0;
				pClass->m_nTimeRest = 0;
				pClass->m_pNowPoint = pClass->m_pHeader;
			}
		}
	}
}


void	CMidi::ThreadMain()
{
	while (true) {
		// イベント待ち
		unsigned long Result = ::WaitForSingleObject(m_hEvent, INFINITE);

		if (m_bEnd == true)		return;

		while (true) {
			// クリティカルセクションに入る
			::EnterCriticalSection(&m_stCriticalSection);

			if (m_stQue[0].bUse == false) {
				// クリティカルセクションから抜けてループからも抜ける
				::LeaveCriticalSection(&m_stCriticalSection);
				break;
			}

			EVENT_QUE	Que;
			EVENT_QUE*	pQue = &Que;
			::CopyMemory(pQue, &m_stQue[0], sizeof(EVENT_QUE));

			// キューを詰める
			::MoveMemory(&m_stQue[0], &m_stQue[1], sizeof(EVENT_QUE) * (EVT_MAX - 1));
			m_stQue[EVT_MAX - 1].bUse = false;

			// クリティカルセクションから抜ける
			::LeaveCriticalSection(&m_stCriticalSection);
			// イベント処理
			switch (pQue->EventType) {
			case EVT_LOAD:
				if ( pQue->Data_Load.nType == 0 ) {
					_AttachMidiData((unsigned char*)pQue->Data_Load.pData, pQue->Data_Load.nSize);
				} else if ( pQue->Data_Load.nType == 1 ) {
					_AttachMidiFile(pQue->Data_Load.pData, pQue->Data_Load.nStart, pQue->Data_Load.nSize);
				}
				delete[] pQue->Data_Load.pData;
				break;
			case EVT_PLAY:
				_Play(pQue->Data_Play.nLoopCount);
				break;
			case EVT_STOP:
				_Stop();
				break;
			case EVT_PAUSE:
				_Pause();
				break;
			case EVT_RELEASE:
				return;
			}
		}
	}
}


//! スレッド処理
unsigned long CALLBACK CMidi::ThreadProc(void* pParameter)
{
	CMidi* pMidi = (CMidi*)pParameter;

	// メイン関数呼び出し
	pMidi->ThreadMain();
	pMidi->_Release();

	// メモリから解放
	delete pMidi;

	// スレッド終了
	ExitThread(0);
}


/*  Bottom of midi.h  */
