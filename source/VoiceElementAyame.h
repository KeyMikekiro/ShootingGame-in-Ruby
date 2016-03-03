/**
 *  @file		VoiceElementAyame.h
 *  @brief		単一のサウンド管理クラス
 *  @author		Sazanta
 */
#ifndef __VOICEELEMENTAYAME_H__
#define	__VOICEELEMENTAYAME_H__

// インクルード
#include	"AyameManager.h"
#include	"VoiceElement.h"


//! CVoiceElementAyameクラス
/**
 *	このクラスは、Ayameを使用したCVoiceElement操作を提供する派生クラスです。<br>
 */
class CVoiceElementAyame : public CVoiceElement
{
  public:
	static	CAyameManager		m_sAyameMgr;

  protected:
			CAyame*				m_pAyame;			//!< Ayameインスタンス

			void				_Release(void) {
									if (m_pAyame != NULL) {
										m_pAyame->Release();
										m_pAyame = NULL;
									}
								}
			void				_SetPan(INT32 nPan) {
									if (m_pAyame != NULL) {
										m_pAyame->SetPan((FLOAT)nPan * 100.0f * (1.0f / 128.0f));
									}
								}
			void				_SetVolume(INT32 nVolume) {
									if (m_pAyame != NULL) {
										m_pAyame->SetVolume((FLOAT)nVolume * 100.0f * (1.0f / 128.0f));
									}
								}
			void				_Pause(void) {
									if (m_pAyame != NULL) {
										m_pAyame->Pause();
									}
								}
			bool				_Load(const char* pSoundData, UINT32 nDataSize, UINT32 nFlag) {
									m_pAyame = m_sAyameMgr.CreateInstanceFromMemory(pSoundData, nDataSize, nFlag);
									return (m_pAyame != NULL);
								}
			bool				_Load(const TCHAR* pFileName, UINT32 nFlag, UINT32 nOffset, UINT32 nDataSize) {
									m_pAyame = m_sAyameMgr.CreateInstanceFromFile((TCHAR*)pFileName, nOffset, nDataSize, nFlag);
									return (m_pAyame != NULL);
								}
			bool				_IsPlaying(void)	const	{
									if (m_pAyame == NULL)	return false;
									return m_pAyame->IsPlay();
								}
			bool				_IsFinished(void)	const	{
									if (m_pAyame == NULL)	return true;
									return (IsPause() == false && m_pAyame->IsPlay() == false);
								}
			void				_Play(UINT32 nLoop, UINT32 nLoopInSample = 0, UINT32 nStartPos = 0) {
									if (m_pAyame != NULL) {
										m_pAyame->Play(nLoop, nLoopInSample, nStartPos);
									}
								}
			void				_Stop(void){
									if (m_pAyame != NULL) {
										m_pAyame->Stop();
									}
								}
			void				_Prefetch(void){
									if (m_pAyame != NULL) {
										m_pAyame->Prefetch();
									}
								}
			void				_Predecode(void){
									if (m_pAyame != NULL) {
										m_pAyame->Predecode();
									}
								}

  public:
								CVoiceElementAyame(void) {
									m_pAyame = NULL;
								}
	virtual						~CVoiceElementAyame(void) {
//									_Release();
								}
};


#endif		// __VOICEELEMENTAYAME_H__
/*  Bottom of VoiceElementAyame.h  */
