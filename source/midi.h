/**
 *	@brief	MIDI�Đ��N���X
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
	char *pData;			// nType��0�ȊO�̏ꍇ��delete����
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


//! MIDI�Đ��N���X
class	CMidi
{
  protected:
			CRITICAL_SECTION	m_stCriticalSection;	//!< �N���e�B�J���Z�N�V����
				HANDLE			m_hThread;				//!< �X���b�h�n���h��
				unsigned long	m_nThreadId;			//!< �X���b�hID
				HANDLE			m_hEvent;				//!< �C�x���g
				EVENT_QUE		m_stQue[EVT_MAX];		//!< �C�x���g�L���[

				CMidiEvent*		m_pHeader;				//!< �f�[�^�擪�A�h���X
				CMidiEvent*		m_pNowPoint;			//!< ���݂̃f�[�^�|�C���^
		union {
				unsigned short	m_nAttribute;			//!< ����
			struct {
				unsigned short	m_nAttr_Pause	: 1;	//!< �|�[�Y���t���O
				unsigned short	m_nAttr_UpdateV	: 1;	//!< ���ʍX�V�t���O
				unsigned short	m_nAttr_UpdateP	: 1;	//!< �p���|�b�g�X�V�t���O
				unsigned short	m_nIsPlayQue	: 1;	//!< �Đ��������t���O
				unsigned short	m_nAttr_pad		:12;
			};
		};
				unsigned short	m_nTimeBias;			//!< ����\�o�C�A�X
				unsigned long	m_nTime;				//!< ����\
				unsigned long	m_nTempo;				//!< �e���|
				unsigned long	m_nLoop;				//!< ���[�v��
				unsigned long	m_nNowTime;				//!< ���݂̍Đ�����
				unsigned long	m_nNowTimeRest;			//!< ���݂̍Đ��]�莞��
				unsigned long	m_nNextTime;			//!< ���̃C�x���g�𑗏o���鎞��
				unsigned long	m_nPrevTime;			//!< �O�񏈗����s��������
				unsigned long	m_nTimeRest;			//!< �]�莞��
				unsigned long	m_nTotalTime;			//!< �g�[�^������
				unsigned int	m_nTimerID;				//!< �^�C�}�[ID
				HMIDIOUT		m_hMidiOut;

				unsigned char	m_nPanpot[16];			//!< �`�����l���p��
				unsigned short	m_nVolume[16];			//!< �`�����l���{�����[��
				unsigned short	m_nMasterVolume;		//!< �}�X�^�[�{�����[��
				unsigned short	m_nMuteFlag;			//!< �~���[�g�t���O
				unsigned char	m_nMasterPanpot;		//!< �}�X�^�[�p���|�b�g
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
								//! �Đ������ǂ���
								/**
								 *	@return	�Đ����̏ꍇ��true��Ԃ�
								 */
				bool			IsPlaying(void)	const	{
									return (m_nIsPlayQue == true || m_pNowPoint != NULL);
								}
								//! �����Ԃ̎擾
								/**
								 *	@return	������(�P�ʂ�ms)
								 */
				unsigned long	GetTotalTime(void)	const	{
									return (m_pHeader != NULL)? m_nTotalTime : 0;
								}
								//! �Đ����Ԃ̎擾
								/**
								 *	@return �Đ�����(�P�ʂ�ms)
								 */
				unsigned long	GetPlayingTime(void)	const	{
									return (m_pHeader != NULL)? m_nNowTime : 0;
								}

								//! ���[�v�񐔂̐ݒ�
				void			SetLoopCounter(unsigned int nLoop) {
									if (m_pHeader != NULL) {
										m_nLoop = nLoop;
									}
								}
								//! ���[�v�񐔂̎擾
				unsigned long	GetLoopCounter(void)	const	{
									return (m_pHeader != NULL)? m_nLoop : 0;
								}

				unsigned short	SetMuteFlag(unsigned short nFlag);
				unsigned short	SetMuteFlag(unsigned int nChannel, bool bFlag);
								//! �~���[�g�t���O�̎擾
				unsigned short	GetMuteFlag(void)	const	{
									return m_nMuteFlag;
								}

								//! �e���|�X�s�[�h�̐ݒ�
								/**
								 *	@param	nSpeed	[in] �e���|�X�s�[�h(1000=1.0)
								 */
				void			SetTimeSpeed(unsigned long nSpeed) {
									m_nTimeBias = (unsigned short)((nSpeed == 0)? 1 : nSpeed);
								}
								//! �e���|�X�s�[�h�̎擾
								/**
								 *	@return	�e���|�X�s�[�h(1000=1.0)
								 */
				unsigned long	GetTimeSpeed(void)	const	{
									return m_nTimeBias;
								}

								//! �e���|�̎擾
				unsigned long	GetRealTempo(void)	const	{
									return m_nTempo;
								}
								//! �e���|�̎擾
				unsigned long	GetTempo(void)	const	{
									return (60 * 1000000) / m_nTempo;
								}

								//! �}�X�^�[�{�����[���̐ݒ�
								/**
								 *	@param	nVol	[in] �}�X�^�[�{�����[��(0�`256)
								 */
				void			SetMasterVolume(unsigned short nVol) {
									m_nMasterVolume = ((nVol > 256)? 256 : nVol);
									m_nAttr_UpdateV = 1;
								}
								//! �}�X�^�[�{�����[���̎擾
				unsigned short	GetMasterVolume(void)	const	{
									return m_nMasterVolume;
								}

								//! �`�����l���{�����[���̐ݒ�
								/**
								 *	�e�`�����l���̃{�����[�����w�肵�܂��B
								 *	�e�`�����l���̃{�����[����MIDI�f�[�^�ɂ���ď㏑�������\��������܂��B
								 *
								 *	@param	nChannel	[in] �`�����l��(0�`15)
								 *	@param	nVol		[in] �`�����l���{�����[��(0�`127)
								 */
				void			SetChannelVolume(unsigned long nChannel, unsigned short nVol) {
									if (nChannel >= 16)		return;
									m_nVolume[nChannel] = (nVol > 127)? 127 : nVol;
									m_nAttr_UpdateV = 1;
								}
								//! �`�����l���{�����[���̎擾
								/**
								 *	@return		�`�����l���{�����[��
								 */
				unsigned short	GetChannelVolume(unsigned long nChannel)	const	{
									return (nChannel < 16)? m_nVolume[nChannel] : 100;
								}

								//! �}�X�^�[�p���|�b�g�̐ݒ�
								/**
								 *	@param	nPan	[in] �}�X�^�[�p���|�b�g(0�`127)
								 */
				void			SetMasterPanpot(unsigned char nPan) {
									m_nMasterPanpot = ((nPan > 127)? 127 : nPan);
									m_nAttr_UpdateP = 1;
								}
								//! �}�X�^�[�p���|�b�g�̎擾
				unsigned char	GetMasterPanpot(void)	const	{
									return m_nMasterPanpot;
								}

								//! �`�����l���p���|�b�g�̐ݒ�
								/**
								 *	�e�`�����l���̃p���|�b�g���w�肵�܂��B
								 *	�e�`�����l���̃p���|�b�g��MIDI�f�[�^�ɂ���ď㏑�������\��������܂��B
								 *
								 *	@param	nChannel	[in] �`�����l��(0�`15)
								 *	@param	nPan		[in] �`�����l���p���|�b�g(0�`127)
								 */
				void			SetChannelPanpot(unsigned long nChannel, unsigned char nPan) {
									if (nChannel >= 16)		return;
									m_nPanpot[nChannel] = (nPan > 127)? 127 : nPan;
									m_nAttr_UpdateP = 1;
								}
								//! �`�����l���p���|�b�g�̎擾
								/**
								 *	@return		�`�����l���p���|�b�g
								 */
				unsigned char	GetChannelPanpot(unsigned long nChannel)	const	{
									return (nChannel < 16)? m_nPanpot[nChannel] : 64;
								}

	static		bool			CheckMidiHeader(const unsigned char* pData);
};


/*  Bottom of midi.h  */
