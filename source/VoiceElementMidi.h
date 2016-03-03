/**
 *  @file		VoiceElementMidi.h
 *  @brief		単一のサウンド管理クラス
 *  @author		Sazanta
 */
#ifndef __VOICEELEMENTMIDI_H__
#define	__VOICEELEMENTMIDI_H__

// インクルード
#include	"midi.h"
#include	"VoiceElement.h"


//! CVoiceElementMidiクラス
/**
 *	このクラスは、CMidiに対してのCVoiceElement操作を提供する派生クラスです。<br>
 *	MIDIファイルの再生を行います。<br>
 */
class CVoiceElementMidi : public CVoiceElement
{
  protected:
			CMidi*				m_pMidi;

			void				_Release(void) {
									if (m_pMidi != NULL) {
										m_pMidi->Release();
										m_pMidi = NULL;
									}
								}
			void				_SetPan(INT32 nPan) {
									if (m_pMidi != NULL) {
										m_pMidi->SetMasterPanpot(64 + (INT32)(nPan / 2));
									}
								}
			void				_SetVolume(INT32 nVolume) {
									if (m_pMidi != NULL) {
										m_pMidi->SetMasterVolume((UINT16)(nVolume * 2));
									}
								}
			void				_Pause(void) {
									if (m_pMidi != NULL) {
										m_pMidi->Pause();
									}
								}
			bool				_Load(const char* pSoundData, UINT32 nDataSize, UINT32 nFlag) {
									if (m_pMidi == NULL) {
										m_pMidi = new CMidi;
									}
									return m_pMidi->AttachMidiData((const unsigned char*)pSoundData, nDataSize);
								}
			bool				_Load(const TCHAR* pFileName, UINT32 nFlag, UINT32 nOffset, UINT32 nDataSize) {
									if (m_pMidi == NULL) {
										m_pMidi = new CMidi;
									}
									return m_pMidi->AttachMidiFile(pFileName, nOffset, nDataSize);
								}
			bool				_IsPlaying(void)	const	{
									if (m_pMidi == NULL)	return false;
									return m_pMidi->IsPlaying();
								}
			bool				_IsFinished(void)	const	{
									if (m_pMidi == NULL)	return true;
									return (IsPause() == false && m_pMidi->IsPlaying() == false);
								}
			void				_Play(UINT32 nLoop, UINT32 nLoopInSample = 0, UINT32 nStartPos = 0) {
									if (m_pMidi != NULL) {
										m_pMidi->Play(nLoop);
									}
								}
			void				_Stop(void) {
									if (m_pMidi != NULL) {
										m_pMidi->Stop();
									}
								}
  public:
								CVoiceElementMidi(void) {
									m_pMidi = NULL;
								}
	virtual						~CVoiceElementMidi(void) {
									_Release();
								}
};


#endif		// __VOICEELEMENTMIDI_H__
/*  Bottom of VoiceElementMidi.h  */
