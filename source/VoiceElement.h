/**
 *  @file		VoiceElement.h
 *  @brief		単一のサウンド管理クラス
 *  @author		Sazanta
 */
#ifndef __VOICEELEMENT_H__
#define	__VOICEELEMENT_H__
#include	<windows.h>

#define	NUM_OF_SUB_VOLUME	0			//!< サブボリュームの数

const	INT32		cnVoiceElm_VolumeMin	= 0;				//!< 最小ボリューム値
const	INT32		cnVoiceElm_VolumeMax	= 128;				//!< 最大ボリューム値
const	INT32		cnVoiceElm_VolumeRange	= (cnVoiceElm_VolumeMax - cnVoiceElm_VolumeMin);	//!< ボリューム範囲
const	INT32		cnVoiceElm_PanpotCenter	= 0;				//!< パンセンター値
const	INT32		cnVoiceElm_PanpotMaxR	= 128;				//!< 右パン最大値
const	INT32		cnVoiceElm_PanpotMaxL	= -128;				//!< 左パン最大値
const	INT32		cnVoiceElm_PanpotRange	= (cnVoiceElm_PanpotMaxR - cnVoiceElm_PanpotMaxL);	//!< パン範囲
const	INT32		cnVoiceElm_PanpotHalfRange	= (cnVoiceElm_PanpotRange / 2);					//!< パン範囲(ハーフ)


//! CVoiceElementクラス
/**
 *	このクラスは、サウンドのフェード・オートパンといった時間変位を行うためのクラスです。<br>
 *	ポーズ時にフェードを行う際、ボリュームを保存するので、ポーズ解除時に元のボリュームに戻すことが出来ます。<br>
 *	またマスターボリュームをサポートするための機構も備えています。<br>
 *	１つのクラスにつき、１つのボイスを管理することが出来ます。<br>
 *	単体でも使用することは出来ますが、通常はサウンドマネージャ等によって管理されます。<br>
 *	このクラス自体は、インターフェイスを提供するだけですので、実際にはこのクラスから派生したクラスを利用します。<br>
 */
class CVoiceElement
{
	class CFadeValue
	{
		private:
			INT32				m_nNowValue;				//!< 現在の値
			INT32				m_nNowTick;					//!< 現在のタイム
			INT32				m_nTotalTick;				//!< 総タイム
			INT32				m_nStartVal;				//!< 開始値
			INT32				m_nDistVal;					//!< 範囲値

		public:
								CFadeValue(void) {
									m_nNowTick = 0;
								}
								~CFadeValue(void) {}

			bool				IsFade(void)	const	{
									return (m_nNowTick > 0);
								}
			void				ResetTime(void) {
									m_nNowTick = 0;
								}
			INT32				GetNowValue(void)	const	{
									return m_nNowValue;
								}
			void				SetNowValue(INT32 nValue) {
									m_nNowValue = nValue;
								}
			void				SetParam(INT32 nTick, INT32 nStart, INT32 nEnd) {
									m_nNowTick   = nTick;
									m_nTotalTick = nTick;
									m_nStartVal  = nEnd;
									m_nDistVal   = nStart - nEnd;
								}

			INT32				GetCalcValue(void)	const	{
									return ((m_nDistVal * m_nNowTick) / m_nTotalTick) + m_nStartVal;
								}
			bool				Run(INT32 nTick, INT32& nValue) {
									if (m_nNowTick <= 0)		return false;
									m_nNowTick -= nTick;
									if (m_nNowTick < 0) {
										m_nNowTick = 0;
									}
									nValue = GetCalcValue();
									return true;
								}
	};

  protected:
			UINT32				m_dwNumber;					//!< ボイスナンバー
			UINT32				m_dwUnique;					//!< 識別ＩＤ
			bool				m_bFinishRequest;			//!< フェード後終了要求
			bool				m_bPauseRequest;			//!< フェード後ポーズ要求
			bool				m_bPause;					//!< ポーズ中フラグ
			bool				m_bVolumeFade;				//!< ボリュームフェード中フラグ

			CFadeValue			m_cVolume;					//!< ボリューム
#if defined(NUM_OF_SUB_VOLUME) && (NUM_OF_SUB_VOLUME > 0)
			CFadeValue			m_cSubVolume[NUM_OF_SUB_VOLUME];	//!< サブボリューム
#endif
			CFadeValue			m_cPanpot;					//!< パンポット

	static	INT32				m_snGlobalVol;				//!< グローバルボリューム

  protected:
			void				AbortFade(void);
			void				InternalFade(INT32 nVolume, INT32 nTick = 0, INT32 nStartVolume = -1);
			INT32				InternalGetVolume(void)	const;
			INT32				CalculateSubVolume(void)	const;
			void				VolumeClamp(INT32& nVolume)	const	{
									nVolume = (nVolume < cnVoiceElm_VolumeMin)? cnVoiceElm_VolumeMin : ((nVolume > cnVoiceElm_VolumeMax)? cnVoiceElm_VolumeMax : nVolume);
								}

	virtual	void				_Release(void) {}
	virtual	void				_SetPan(INT32 nPan) {}
	virtual	void				_SetVolume(INT32 nVolume) {}
	virtual	void				_SetSpeedRate(INT32 nRate) {}
	virtual	void				_Pause(void) {}
	virtual	bool				_Load(const char* pSoundData, UINT32 nDataSize, UINT32 nType) {
									return false;
								}
	virtual	bool				_Load(const TCHAR* pFileName, UINT32 nType, UINT32 nOffset, UINT32 nDataSize) {
									return false;
								}
	virtual	bool				_IsPlaying(void)	const	{
									return false;
								}
	virtual	bool				_IsFinished(void)	const	{
									return true;
								}
	virtual	void				_Play(UINT32 nLoop, UINT32 nLoopInSample = 0, UINT32 nStartPos = 0) {}
	virtual	void				_Stop(void) {}
	virtual	void				_Prefetch(void) {}
	virtual	void				_Predecode(void) {}

  public:
								CVoiceElement(void);
								//!	デストラクタ
	virtual						~CVoiceElement(void) {}

			void				Run(INT32 nTick);

								//! ボイスナンバーの設定
			void				SetNumber(UINT32 dwNumber) {
									m_dwNumber = dwNumber;
								}
								//! ボイスナンバーの取得
			UINT32				GetNumber(void)	const	{
									return m_dwNumber;
								}

								//! 識別IDの設定
			void				SetUnique(UINT32 dwUnique) {
									m_dwUnique = dwUnique;
								}
								//! 識別IDの取得
			UINT32				GetUnique(void)	const	{
									return m_dwUnique;
								}

								//!	データの作成
								/**
								 *	データの作成を行います。<br>
								 */
			bool				Load(const char* pSoundData, UINT32 nDataSize, UINT32 nFlag) {
									Release();
									return _Load(pSoundData, nDataSize, nFlag);
								}
								//!	データの作成
								/**
								 *	データの作成を行います。<br>
								 */
			bool				Load(const TCHAR* pFileName, UINT32 nFlag, UINT32 nOffset, UINT32 nDataSize) {
									Release();
									return _Load(pFileName, nFlag, nOffset, nDataSize);
								}
			void				Release(void);

								//! 再生中かどうかチェック
								/**
								 *	再生中かどうかを取得します。<br>
								 *
								 *	@return	再生中ならtrue
								 */
			bool				IsPlaying(void)	const
								{
									return _IsPlaying();
								}
								//!	停止完了チェック
								/**
								 *	停止が完了したかどうかを取得します。<br>
								 *
								 *	@return	停止完了ならtrue
								 */
			bool				IsFinished(void)	const;
								//!	フェード中かどうかチェック
								/**
								 *	フェード中かどうかを取得します。<br>
								 *
								 *	@return	フェード中ならtrue
								 */
			bool				IsFade(void)	const	{
									return m_cVolume.IsFade();
								}
								//!	ポーズ中かどうかチェック
								/**
								 *	ポーズ中かどうかを取得します。<br>
								 *
								 *	@return	ポーズ中ならtrue
								 */
			bool				IsPause(void)	const	{
									return m_bPause;
								}

								//!	マスターボリュームの反映
								/**
								 *	マスターボリュームを即時反映させます。<br>
								 */
			void				ReflectMasterVolume(void) {
									if (IsFade() == false) {
										SetVolume(-1);
									}
								}
			INT32				GetVolume(void)	const;
			void				SetVolume(INT32 nVolume, INT32 nTick = 0, INT32 nStartVolume = -1);
			INT32				GetSubVolume(UINT32 nChannel)	const;
			void				SetSubVolume(UINT32 nChannel, INT32 nVolume, INT32 nTick = 0, INT32 nStartVolume = -1);

			INT32				GetPan(void)	const;
			void				SetPan(INT32 nPan, INT32 nTick = 0, INT32 nStartPan = -(cnVoiceElm_PanpotHalfRange + 1));

			INT32				GetSpeedRate(void)	const	{
									return 4096;
								}
			void				SetSpeedRate(INT32 nRate) {
									_SetSpeedRate(nRate);
								}

			void				Play(UINT32 dwLoop = 0, INT32 nVolume = -1, INT32 nFadeInTick = 0, UINT32 nLoopInSample = 0);
			void				Stop(INT32 nFadeOutTick = 0);
			void				Pause(bool bPause, INT32 nFadeTick = 0);
			void				Prefetch(void) {
                    			    _Prefetch();
								}
			void				Predecode(void) {
                    			    _Predecode();
								}

	static	void				SetGlobalVolume(INT32 nVolume) {
									m_snGlobalVol = (nVolume < cnVoiceElm_VolumeMin)? cnVoiceElm_VolumeMin : ((nVolume > cnVoiceElm_VolumeMax)? cnVoiceElm_VolumeMax : nVolume);
								}
};


#endif		// __VOICEELEMENT_H__
/*  Bottom of VoiceElement.h  */
