/**
 *	@brief	MIDI�Đ��N���X
 *	@file	midi.cpp
 *	@author	sazanta
 */
#include	"midi.h"

#pragma comment (lib, "winmm.lib")


//! MIDI�C�x���g�ێ��N���X
class CMidiEvent
{
  public:
	CMidiEvent*			lpNext;		// ���̃C�x���g�ւ̃|�C���^
	unsigned long		dwDelta;	// �f���^�^�C��
	unsigned char		state;		// �X�e�[�^�X�o�C�g
	unsigned char		data1;		// ���f�[�^�o�C�g
	unsigned char		data2;		// ���f�[�^�o�C�g
	unsigned char		type;		// �^�C�v
	unsigned char*		lpData;		// �ϒ��f�[�^
	long				nData;		// �f�[�^��

						CMidiEvent(void) {
							lpNext = NULL;
							lpData = NULL;
							nData  = 0;
						}
						~CMidiEvent(void) {
							delete[] lpData;
						}
};


//! �R���X�g���N�^
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

	// �N���e�B�J���Z�N�V�����̐���
	::InitializeCriticalSection(&m_stCriticalSection);
	::ZeroMemory(&m_stQue, sizeof(m_stQue));
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_bEnd   = false;
	// �����X���b�h����
	m_hThread = ::CreateThread(NULL, 0, ThreadProc, this, 0, &m_nThreadId);
}


//! �f�X�g���N�^
CMidi::~CMidi(void)
{
	// �N���e�B�J���Z�N�V�����̔j��
	::DeleteCriticalSection(&m_stCriticalSection);
	::CloseHandle(m_hEvent);
}


//! MIDI�f�[�^�̉��
void	CMidi::Release(void)
{
	::EnterCriticalSection(&m_stCriticalSection);
	m_bEnd = true;
	::SetEvent(m_hEvent);
	::LeaveCriticalSection( &m_stCriticalSection );
	::WaitForSingleObject(m_hThread, INFINITE);
}


//! MIDI�t�@�C���̃A�^�b�`
/**
 *	MIDI�t�@�C���̃A�^�b�`���s���܂��B
 *
 *	@param	pFileName	[in] SMF�t�@�C���p�X
 *	@param	nOfs		[in] SMF�t�@�C���I�t�Z�b�g
 *	@param	nSize		[in] SMF�t�@�C���T�C�Y(0�̏ꍇ�͎����v�Z)
 *
 *	@return	�������A�^�b�`�ł����ꍇ��true��Ԃ�
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

	// �L���[�ɒǉ�
	return AddQue(&Que);
}


//! MIDI�f�[�^�̃A�^�b�`
/**
 *	MIDI�f�[�^�̃A�^�b�`���s���܂��B
 *	�֐��𔲂�����́A�f�[�^���폜���Ă��\���܂���B
 *
 *	@param	pData	[in] SMF�f�[�^�|�C���^
 *	@param	nSize	[in] SMF�f�[�^�T�C�Y
 *
 *	@return	�������A�^�b�`�ł����ꍇ��true��Ԃ�
 */
bool	CMidi::AttachMidiData(const unsigned char* pData, unsigned int nSize)
{
	EVENT_QUE Que = { EVT_LOAD };

	Que.Data_Load.pData = new char[nSize];
	if (Que.Data_Load.pData == NULL)		return false;
	::CopyMemory(Que.Data_Load.pData, pData, nSize);
	Que.Data_Load.nSize = nSize;
	Que.Data_Load.nType = 0;

	// �L���[�ɒǉ�
	return AddQue(&Que);
}


//! �Đ�
bool	CMidi::Play(unsigned long nLoop)
{
	EVENT_QUE Que = { EVT_PLAY };

	Que.Data_Play.nStartPos = 0;
	Que.Data_Play.nLoopCount = nLoop;

	// �L���[�ɒǉ�
	return AddQue(&Que);
}


//! ��~
bool	CMidi::Stop(void)
{
	EVENT_QUE Que = { EVT_STOP };

	// �L���[�ɒǉ�
	return AddQue(&Que);
}


//! �|�[�Y
bool	CMidi::Pause(void)
{
	EVENT_QUE Que = { EVT_PAUSE };

	// �L���[�ɒǉ�
	return AddQue(&Que);
}


//! �~���[�g�t���O�̐ݒ�
/**
 *	@param	nFlag	[in] �~���[�g����`�����l���̘_���a
 *
 *	@return	�ݒ���s���O�̒l
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


//! �~���[�g�t���O�̐ݒ�
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


//! MIDI�w�b�_�̃`�F�b�N
/**
 *	@return	MIDI�w�b�_�����������true��Ԃ�
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
	// �N���e�B�J���Z�N�V�����ɓ���
	::EnterCriticalSection(&m_stCriticalSection);
	if (m_bEnd == true) {
		// �N���e�B�J���Z�N�V�������甲����
		::LeaveCriticalSection( &m_stCriticalSection );
		return true;
	}

	bool	bRet = false;
	int		nSlot = 0;
	for (; ((nSlot < EVT_MAX) && (m_stQue[nSlot].bUse == true)); nSlot++) {
		if (m_stQue[nSlot].EventType == pQue->EventType) {
			// �攭�̓����v���͋p������
			if (m_stQue[nSlot].EventType == EVT_LOAD && m_stQue[nSlot].Data_Load.nType != 0) {
				delete[] m_stQue[nSlot].Data_Load.pData;
			}
			::MoveMemory(&m_stQue[nSlot], &m_stQue[nSlot + 1], sizeof(EVENT_QUE) * (EVT_MAX - (nSlot + 1)));
			m_stQue[EVT_MAX - 1].bUse = false;
			nSlot--;
		}
	}
	// �L���[�͈�t�ł͂Ȃ�
	if (nSlot != EVT_MAX) {
		// �f�[�^�R�s�[
		::CopyMemory(&m_stQue[nSlot], pQue, sizeof(EVENT_QUE));
		// �g�p���t���OON
		m_stQue[nSlot].bUse = true;
		// �v���C�L���[�̏ꍇ�́A�t���O�𗧂Ă�
		if (pQue->EventType == EVT_PLAY) m_nIsPlayQue = true;
		// �C�x���gON
		::SetEvent(m_hEvent);
		// ��������
		bRet = true;
	}

	// �N���e�B�J���Z�N�V�������甲����
	::LeaveCriticalSection(&m_stCriticalSection);

	return bRet;
}


//! �f�[�^�̉��
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
		// �f�[�^�s��
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
		// �f�[�^�s��
		return false;
	}
	pTmpData += 2;
	nSize -= 14;

	CMidiEvent*	lpaEvent[128];
	for (int i = 0; i < nTrack; i++) {
		if (ReadTrack(&pTmpData, &lpaEvent[i], nSize) == false) {
			// �g���b�N�ǂݍ��ݎ��s
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
	m_nTempo       = 500000; // �e���|�̃f�t�H���g�l
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
		midiOutReset(m_hMidiOut); // �Đ����̉�������
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
		midiOutReset(m_hMidiOut); // �Đ����̉�������
		m_nAttr_UpdateV = 1;
	}
}


//! �C�x���g�N���X�̉��
void	CMidi::ReleaseMain(CMidiEvent* pEvent)
{
	while (pEvent != NULL) {
		CMidiEvent*	pTmp = pEvent;
		pEvent = pEvent->lpNext;
		delete pTmp;
	}
}


//! �g���b�N�̉��
bool	CMidi::ReadTrack(unsigned char** ppData, CMidiEvent** lplpEvent, unsigned int& nSize)
{
	unsigned char*	pData = *ppData;
	unsigned char	statePrev = 0; // �O�̃C�x���g�̃X�e�[�^�X�o�C�g
	CMidiEvent*		lpEvent;

	if (pData[0] != 'M' || pData[1] != 'T' || pData[2] != 'r' || pData[3] != 'k' || nSize <= 8) {
		// �f�[�^�s��
		*lplpEvent = NULL;
		return false;
	}
	nSize -= 4 + 4;
	unsigned long	nLen = pData[7] | ((unsigned long)pData[6] << 8) | ((unsigned long)pData[5] << 16) | ((unsigned long)pData[4] << 24);
	if (nLen > nSize) {
		// �f�[�^�s��
		*lplpEvent = NULL;
		return false;
	}
	pData += 4 + 4;

	lpEvent    = new CMidiEvent();	// �ŏ��̃C�x���g�̃��������m��
	*lplpEvent = lpEvent;			// lplpEvent�͏�ɍŏ��̃C�x���g���w��
	while (1) {
		// �f���^�^�C����ǂݍ���
		if ((pData = ReadDelta(pData, lpEvent->dwDelta, nSize)) == NULL) {
			// �f�[�^�s��
			return false;
		}

		lpEvent->state = *pData;
		if ((lpEvent->state & 0x80) == 0) {	// �����j���O�X�e�[�^�X��
			lpEvent->state = statePrev;		// ��O�̃C�x���g�̃X�e�[�^�X�o�C�g����
		} else {
			pData++;
			nSize--;
		}

		switch (lpEvent->state & 0xf0) {	// �X�e�[�^�X�o�C�g����ɂǂ̃C�x���g������
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
				// SysEx�C�x���g
				if ((pData = ReadDelta(pData, dw, nSize)) == NULL) {
					return false;
				}
				if (lpEvent->state == 0xf0)	dw++;
				lpEvent->nData  = dw;
				lpEvent->lpData = new unsigned char[dw];
				if (lpEvent->state == 0xf0) {
					lpEvent->lpData[0] = 0xf0;						// �ϒ��f�[�^�̐擪��0xF0
					dw--;
					memcpy(lpEvent->lpData + 1, pData, dw);
				} else {
					memcpy(lpEvent->lpData, pData, dw);
				}
				pData += dw;
				nSize -= dw;
			} else if (lpEvent->state == 0xff) {
				// ���^�C�x���g
				lpEvent->type = *pData++;		// type�̎擾
				nSize--;
				if ((pData = ReadDelta(pData, dw, nSize)) == NULL) {
					// �f�[�^�s��
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
					// �g���b�N�̏I�[
					*ppData = pData;
					return true;
				}
			}
			break;

		default:
			// �X�e�[�^�X�s��
			return false;
		}
		statePrev = lpEvent->state;

		lpEvent->lpNext = new CMidiEvent();
		lpEvent         = lpEvent->lpNext;
		if (lpEvent == NULL) {
			// �������s��
			return false;
		}
	}

	return false;
}


//! �f���^�^�C���̎擾
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
			break;		// MSB�������Ă��Ȃ��Ȃ�΁A���̃o�C�g�̓f���^�^�C���ł͂Ȃ��̂Ŕ�����
		}
	}

	return pData;
}


//! �g���b�N�f�[�^�̃}�[�W
CMidiEvent*	CMidi::Marge(CMidiEvent** lplpEvent, unsigned short nTrack)
{
	unsigned long	dwPrevAbsolute = 0;								// ��O�̐�Ύ���
	CMidiEvent*		lpEvent        = new CMidiEvent();				// ���݂̃C�x���g
	unsigned long*	lpdwTotal      = new unsigned long[nTrack];		// �e�g���b�N�̐�Ύ���
	CMidiEvent**	pEventBase     = new CMidiEvent*[nTrack];

	if (lpEvent == NULL || lpdwTotal == NULL || pEventBase == NULL) {
		// �������s��
		delete lpEvent;
		delete[] lpdwTotal;
		delete[] pEventBase;
		return NULL;
	}

	memcpy(pEventBase, lplpEvent, sizeof(CMidiEvent*) * nTrack);
	memset(lpdwTotal, 0, sizeof(unsigned long) * nTrack);
	CMidiEvent*	lpHead = lpEvent;
	while (1) {
		int				nIndex     = -1;				// �g���b�N�̃C���f�b�N�X
		unsigned long	dwAbsolute = (unsigned long)-1; // ��Ύ���(0xFFFFFFFF)

		// �ł���Ύ��Ԃ��Ⴂ�C�x���g��������
 		for (int i = 0; i < nTrack; i++) {
			if (pEventBase[i]->lpNext == NULL) {
				continue;		// �g���b�N�̏I�[�܂ő�������
			}

			if ((lpdwTotal[i] + pEventBase[i]->dwDelta) < dwAbsolute) {
				nIndex = i;
				dwAbsolute = lpdwTotal[i] + pEventBase[i]->dwDelta;
			}
		}

		if (nIndex == -1) {
			// �S�Ẵg���b�N�𑖍������̂�TrackOfEnd��t������
			lpEvent->state   = 0xff;
			lpEvent->data1   = 0x2f;
			lpEvent->data2   = 0x00;
			lpEvent->dwDelta = 0;
			break;
		}

		if (pEventBase[nIndex]->state != 0xff || pEventBase[nIndex]->data1 != 0x2f) {		// TrackOfEnd�͂͂���
			lpEvent->state   = pEventBase[nIndex]->state;
			lpEvent->data1   = pEventBase[nIndex]->data1;
			lpEvent->data2   = pEventBase[nIndex]->data2;
			lpEvent->type    = pEventBase[nIndex]->type;
			lpEvent->nData   = pEventBase[nIndex]->nData;
			lpEvent->dwDelta = dwAbsolute - dwPrevAbsolute;
			if (lpEvent->nData != 0) {
				lpEvent->lpData = new unsigned char[lpEvent->nData];
				if (lpEvent->lpData == NULL) {
					// �������s��
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
				// �������s��
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
		lpdwTotal[nIndex] += pEventBase[nIndex]->dwDelta; // �e�g���b�N�̐�Ύ��Ԃ��X�V
		pEventBase[nIndex] = pEventBase[nIndex]->lpNext;
	}
	for (int j = 0; j < nTrack; j++) {
		ReleaseMain(lplpEvent[j]);
	}
	delete[] pEventBase;
	delete[] lpdwTotal;

	return lpHead;
}


//! �����Ԃ̌v�Z
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
			// �Z�b�g�e���|
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


//! �R�[���o�b�N�֐�
void CALLBACK CMidi::TimeCallbackProc(unsigned int nID, unsigned int nMsg, unsigned long nUser, unsigned long nReserve1, unsigned long nReserve2)
{
	CMidi*	pClass = (CMidi*)nUser;

	if (pClass == NULL || pClass->m_pNowPoint == NULL)	return;

	unsigned long	nTime = timeGetTime();
	if (pClass->m_nAttr_Pause != 0) {
		// �|�[�Y��
		pClass->m_nPrevTime  = nTime;
		return;
	}

	// �{�����[���ݒ�
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
	// �p���|�b�g�ݒ�
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
			// ���^�C�x���g
			if (pClass->m_pNowPoint->type == 0x51) {
				// �Z�b�g�e���|
				pClass->m_nTempo = (unsigned long)(pClass->m_pNowPoint->lpData[2] | ((unsigned long)pClass->m_pNowPoint->lpData[1] << 8) | ((unsigned long)pClass->m_pNowPoint->lpData[0] << 16));
			}
		} else if (pClass->m_pNowPoint->state == 0xf0) {
			// SysEx�C�x���g
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
			// MIDI�C�x���g
			unsigned char	nStatus = pClass->m_pNowPoint->state;
			unsigned long	nData2  = pClass->m_pNowPoint->data2;
			if ((nStatus & 0xf0) == 0xb0) {
				int	nCh = (nStatus & 0x0f);
				if (pClass->m_pNowPoint->data1 == 0x07) {
					// �{�����[��
					pClass->m_nVolume[nCh] = (unsigned short)nData2;
					if ((pClass->m_nMuteFlag & (1<<nCh)) != 0) {
						// �~���[�g
						nData2 = 0;
					} else {
						// �x���V�e�B�v�Z
						nData2 *= pClass->m_nMasterVolume;
						nData2 >>= 8;
					}
				} else if (pClass->m_pNowPoint->data1 == 0x0a) {
					// �p���|�b�g
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
			// �f�[�^�I��
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
		// �C�x���g�҂�
		unsigned long Result = ::WaitForSingleObject(m_hEvent, INFINITE);

		if (m_bEnd == true)		return;

		while (true) {
			// �N���e�B�J���Z�N�V�����ɓ���
			::EnterCriticalSection(&m_stCriticalSection);

			if (m_stQue[0].bUse == false) {
				// �N���e�B�J���Z�N�V�������甲���ă��[�v�����������
				::LeaveCriticalSection(&m_stCriticalSection);
				break;
			}

			EVENT_QUE	Que;
			EVENT_QUE*	pQue = &Que;
			::CopyMemory(pQue, &m_stQue[0], sizeof(EVENT_QUE));

			// �L���[���l�߂�
			::MoveMemory(&m_stQue[0], &m_stQue[1], sizeof(EVENT_QUE) * (EVT_MAX - 1));
			m_stQue[EVT_MAX - 1].bUse = false;

			// �N���e�B�J���Z�N�V�������甲����
			::LeaveCriticalSection(&m_stCriticalSection);
			// �C�x���g����
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


//! �X���b�h����
unsigned long CALLBACK CMidi::ThreadProc(void* pParameter)
{
	CMidi* pMidi = (CMidi*)pParameter;

	// ���C���֐��Ăяo��
	pMidi->ThreadMain();
	pMidi->_Release();

	// ������������
	delete pMidi;

	// �X���b�h�I��
	ExitThread(0);
}


/*  Bottom of midi.h  */
