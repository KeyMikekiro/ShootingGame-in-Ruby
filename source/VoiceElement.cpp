/**
 *  @file		VoiceElement.cpp
 *  @brief		単一のサウンド管理クラス
 *  @author		Sazanta
 */

// インクルード
#include	"VoiceElement.h"


INT32			CVoiceElement::m_snGlobalVol = cnVoiceElm_VolumeMax;


//!	コンストラクタ
CVoiceElement::CVoiceElement(void)
{
	m_dwNumber       = 0;
	m_dwUnique       = 0;
	m_bFinishRequest = false;
	m_bPauseRequest  = false;
	m_bPause         = false;
	m_bVolumeFade    = false;
	m_cVolume.SetNowValue(cnVoiceElm_VolumeMax);
	m_cPanpot.SetNowValue(cnVoiceElm_PanpotCenter);
#if defined(NUM_OF_SUB_VOLUME) && (NUM_OF_SUB_VOLUME > 0)
	for (int i = 0; i < NUM_OF_SUB_VOLUME; i++) {
		m_cSubVolume[i].SetNowValue(cnVoiceElm_VolumeMax);
	}
#endif
}


//!	定例処理
/**
 *	毎フレーム行う処理です。<br>
 *	あまり間隔が空かないよう呼び出してください。<br>
 *
 *	@param	nSyncTick	[in] 前回からの経過時間[tick]
 */
void CVoiceElement::Run(INT32 nSyncTick)
{
	INT32	nValue;

	// オートパン処理
	if (m_cPanpot.Run(nSyncTick, nValue) == true) {
		m_cPanpot.SetNowValue(nValue);
		_SetPan(nValue);
	}

	INT32	nSubVol = cnVoiceElm_VolumeMax;
	bool	bSubChange = false;
#if defined(NUM_OF_SUB_VOLUME) && (NUM_OF_SUB_VOLUME > 0)
	// サブボリュームフェード処理
	for (int i = 0; i < NUM_OF_SUB_VOLUME; i++) {
		if (m_cSubVolume[i].Run(nSyncTick, nValue) == true) {
			nSubVol *= nValue;
			nSubVol /= cnVoiceElm_VolumeRange;
			m_cSubVolume[i].SetNowValue(nValue);
			bSubChange = true;
		}
	}
#endif

	// メインボリュームフェード処理
	if (m_cVolume.Run(nSyncTick, nValue) == true) {
		_SetVolume(nValue * m_snGlobalVol * nSubVol / (cnVoiceElm_VolumeRange * cnVoiceElm_VolumeRange));
		if (m_bVolumeFade == true) {
			m_cVolume.SetNowValue(nValue);
		}
		if (m_cVolume.IsFade() == false) {
			// フェード終了
			if (m_bFinishRequest == true) {
				Stop();
                m_bFinishRequest = false;
			} else if (m_bPauseRequest == true) {
				_Pause();
				m_bPauseRequest = false;
				m_bPause        = true;
			}
			m_bVolumeFade = false;
		}
	} else {
		if (bSubChange != false) {
			_SetVolume(nValue * m_snGlobalVol * nSubVol / (cnVoiceElm_VolumeRange * cnVoiceElm_VolumeRange));
		}
	}
}


//!	データの解放
/**
 *	データの解放を行います。<br>
 */
void CVoiceElement::Release(void)
{
	Stop();
	_Release();
}


//!	再生終了チェック
/**
 *	再生が終了しているかどうかを取得します。<br>
 *
 *	@return	終了している場合はtrueを返す
 */
bool CVoiceElement::IsFinished(void)	const
{
	if (_IsFinished() == true) {
		return true;
	}
	if (m_bFinishRequest == true && IsFade() == false) {
		return true;
	}

	return false;
}


//!	フェードの中断
void	CVoiceElement::AbortFade(void)
{
	if (IsFade() == true) {
		m_cVolume.ResetTime();
		m_bVolumeFade = false;
	}
}


void	CVoiceElement::InternalFade(INT32 nVolume, INT32 nTick, INT32 nStartVolume)
{
	INT32		nSave = m_cVolume.GetNowValue();
	SetVolume(nVolume, nTick, nStartVolume);
	m_cVolume.SetNowValue(nSave);
	m_bVolumeFade = false;
}


INT32 CVoiceElement::InternalGetVolume(void)	const
{
	return (m_cVolume.IsFade() == true && m_bVolumeFade == false)? m_cVolume.GetCalcValue() : m_cVolume.GetNowValue();
}


INT32 CVoiceElement::CalculateSubVolume(void)	const
{
	INT32	nSubVol = cnVoiceElm_VolumeMax;
#if defined(NUM_OF_SUB_VOLUME) && (NUM_OF_SUB_VOLUME > 0)
	for (int i = 0; i < NUM_OF_SUB_VOLUME; i++) {
		nSubVol *= m_cSubVolume[i].GetNowValue() / cnVoiceElm_VolumeRange;
	}
#endif

	return nSubVol;
}


//!	ボリュームの取得
/**
 *	現在のボリュームを取得します。<br>
 *
 *	@return	ボリューム値
 */
INT32 CVoiceElement::GetVolume(void)	const
{
	return m_cVolume.GetNowValue();
}


//!	ボリュームの設定
/**
 *	ボリュームの設定を行います。<br>
 *
 *	@param	nVolume			[in] ボリューム(0 ～ 100)
 *	@param	nTick			[in] 遷移時間[tick]
 *	@param	nStartVolume	[in] スタートボリューム(省略した場合は現在値を参照)
 */
void CVoiceElement::SetVolume(INT32 nVolume, INT32 nTick, INT32 nStartVolume)
{
	VolumeClamp(nVolume);
	m_bVolumeFade = false;
	if (nTick > 0) {
		if (nStartVolume < 0) {
			nStartVolume = InternalGetVolume();
		}
		m_cVolume.SetParam(nTick, nStartVolume, nVolume);
		m_bVolumeFade = true;
		nVolume = nStartVolume;
	} else {
		m_cVolume.ResetTime();
	}

	m_cVolume.SetNowValue(nVolume);
	_SetVolume(nVolume * m_snGlobalVol * CalculateSubVolume() / (cnVoiceElm_VolumeRange * cnVoiceElm_VolumeRange));
}


//! サブボリュームの取得
/**
 *	サブボリューム値の取得を行います。
 *
 *	@param	nChannel	[in] サブボリュームチャンネル
 *
 *	@return	ボリューム値
 */
INT32 CVoiceElement::GetSubVolume(UINT32 nChannel)	const
{
#if defined(NUM_OF_SUB_VOLUME) && (NUM_OF_SUB_VOLUME > 0)
	return (nChannel < NUM_OF_SUB_VOLUME)? m_cSubVolume[nChannel].GetNowValue() : cnVoiceElm_VolumeMax;
#else
	return cnVoiceElm_VolumeMax;
#endif
}


//! サブボリュームの設定
/**
 *	サブボリューム値の設定を行います。
 *
 *	@param	nChannel		[in] サブボリュームチャンネル
 *	@param	nVolume			[in] ボリューム
 *	@param	nTick			[in] 遷移時間[tick]
 *	@param	nStartVolume	[in] スタートボリューム(省略した場合は現在値を参照)
 */
void CVoiceElement::SetSubVolume(UINT32 nChannel, INT32 nVolume, INT32 nTick, INT32 nStartVolume)
{
#if defined(NUM_OF_SUB_VOLUME) && (NUM_OF_SUB_VOLUME > 0)
	if (nChannel >= NUM_OF_SUB_VOLUME)		return;

	VolumeClamp(nVolume);
	if (nTick > 0) {
		if (nStartVolume < 0) {
			nStartVolume = (m_cSubVolume[nChannel].IsFade() == true)? m_cSubVolume[nChannel].GetCalcValue() : m_cSubVolume[nChannel].GetNowValue();
		}
		m_cSubVolume[nChannel].SetParam(nTick, nStartVolume, nVolume);
		nVolume = nStartVolume;
	} else {
		m_cSubVolume[nChannel].ResetTime();
	}
	m_cSubVolume[nChannel].SetNowValue(nVolume);
	ReflectMasterVolume();
#endif
}


//!	パンの取得
/**
 *	現在のパンを取得します。<br>
 *
 *	@return	パン値
 */
INT32 CVoiceElement::GetPan(void)	const
{
	return m_cPanpot.GetNowValue();
}


//!	パンの設定
/**
 *	パンの設定を行います。<br>
 *
 *	@param	nPan		[in] パン
 *	@param	nTick		[in] 遷移時間
 *	@param	nStartPan	[in] スタートパン(省略した場合は現在値を参照)
 */
void CVoiceElement::SetPan(INT32 nPan, INT32 nTick, INT32 nStartPan)
{
	nPan = (nPan < cnVoiceElm_PanpotMaxL)? cnVoiceElm_PanpotMaxL : ((nPan > cnVoiceElm_PanpotMaxR)? cnVoiceElm_PanpotMaxR : nPan);
	if (nTick > 0) {
		if (nStartPan < cnVoiceElm_PanpotMaxL || nStartPan > cnVoiceElm_PanpotMaxR) {
			nStartPan = m_cPanpot.GetNowValue();
		}
		m_cPanpot.SetParam(nTick, nStartPan, nPan);
		nPan = nStartPan;
	} else {
		m_cPanpot.ResetTime();
	}
	m_cPanpot.SetNowValue(nPan);
	_SetPan(nPan);
}


//!	再生
/**
 *	再生を行います。<br>
 *	nLoopに0を指定すると、1回演奏して終了します。<br>
 *	nLoopInSampleを0以外に設定すると、ループする際のループ開始ポイントを設定できます。<br>
 *	単位はサンプルです。<br>
 *
 *	@param	nLoop			[in] ループ回数(LOOP_INFINITYで無限ループ)
 *	@param	nVolume			[in] ボリューム
 *	@param	nFadeInTick		[in] フェードイン時間[tick]
 *	@param	nLoopInSample	[in] ループインサンプル
 */
void CVoiceElement::Play(UINT32 nLoop, INT32 nVolume, INT32 nFadeInTick, UINT32 nLoopInSample)
{
	Stop();
	SetVolume(nVolume, nFadeInTick, 0);
	m_cVolume.SetNowValue(nVolume);
	m_bVolumeFade = false;

	_Play(nLoop, nLoopInSample);
}


//!	停止
/**
 *	停止します。<br>
 *	フェードアウト時間が0でない場合、フェード終了後停止します。<br>
 *
 *	@param	fFadeOutTime	[in] フェードアウト時間
 */
void CVoiceElement::Stop(INT32 nFadeOutTick)
{
	m_bPauseRequest = false;
	if (nFadeOutTick > 0 && m_bPause == false) {
		InternalFade(0, nFadeOutTick);
		m_bFinishRequest = true;
	} else {
		AbortFade();
		m_bPause = false;
		_Stop();
	}
}


//!	一時停止
/**
 *	一時停止します。<br>
 *
 *	@param	bPause		[in] ポーズする場合はtrue、ポーズを解除する場合はfalse
 *	@param	fFadeTime	[in] フェード時間
 */
void CVoiceElement::Pause(bool bPause, INT32 nFadeTick)
{
	if (bPause == true) {
		// ポーズする
		if (m_bPauseRequest == false && IsPause() == false) {
			m_bFinishRequest = false;
			if (nFadeTick > 0) {
				InternalFade(0, nFadeTick);
				m_bPauseRequest = true;
			} else {
				AbortFade();
				_Pause();
				m_bPause = true;
			}
		}
	} else {
		// ポーズ解除する
		if (m_bPauseRequest == true || IsPause() == true) {
			if (IsPause() == true) {
				m_bPause = false;
				InternalFade(m_cVolume.GetNowValue(), nFadeTick, 0);
				_Pause();
			} else {
				InternalFade(m_cVolume.GetNowValue(), nFadeTick);
			}
			m_bPauseRequest = false;
		}
	}
}


/*  Bottom of VoiceElement.cpp  */
