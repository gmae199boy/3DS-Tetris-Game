/*---------------------------------------------------------------------------*
  File: SoundPlayer.h

   サウンド再生
                                                                 2014.09.04
                                                                 Youhei
 *---------------------------------------------------------------------------*/
#ifndef _SOUND_PLAYER_H_
#define _SOUND_PLAYER_H_


#include <nn.h> // nn::snd::Bcwav::WaveInfo
#include <vector>


class SoundPlayer
{
private:
	// ストリーミングの読み込みポジションを管理するクラス
	class _StreamPosition
	{
	public:
		_StreamPosition();

		// ポジションなどの情報をリセットする
		//
		// loopHeadFrame : ループ先頭位置
		// loopTailFrame : ループ終端位置
		//
		void reset(u32 loopHeadFrame, u32 loopTailFrame);

		// ポジションなどの情報をリセットする
		//
		void reset(void);

		// ポジションの更新
		//
		// size : 要求する移動量(バイト)
		//
		// 戻り値 : 実際に移動した移動量(サンプル)
		s32 update(s32 size, s32 channelCount);

		// プロパティ
		const s32& position(void) const { return _position; }
		const bool& isLooped(void) const { return _isLooped; }

	private:
		u32 _loopHeadFrame; // ループ開始フレーム
		u32 _loopTailFrame;	// ループ終了フレーム
		s32 _position;		// 読み込みポジション(サンプル)
		bool _isLooped;		// ループフラグ
		NN_PADDING3;
	};

private:
	SoundPlayer() {}

public:
	SoundPlayer(nn::fnd::ExpHeap* pDeviceHeap);
	virtual ~SoundPlayer();

	void preLoadStream(const char* pFileName);
	void playStream(bool bLoop = true);
	void stopStream(void);
	void pauseStream(void); // ポーズ中だったら再生するよ
	void setStreamVolume(float volume);
	float getStreamVolume(void);

	// ファイル読み込みスレッドの状態
	enum StreamFileReadState
	{
		STREAM_FILE_READ_STATE_NONE,
		STREAM_FILE_READ_STATE_LOAD,
		STREAM_FILE_READ_STATE_MAX
	};

	const StreamFileReadState& getStreamFileReadState(void) const { return _streamFileReadState; }


	// 戻り値：bcwav整理番号
	int loadBcwav(const char* pFileName);

	void unloadAllBcwav(void);

	// 戻り値：voice管理番号 ループ音を止めるときに使う -1だったら再生できなかったということ
	int play(int bcwavNum, bool bLoop = false);

	void stop(int voiceNum);


private:
	void _streamVoiceInitialize(void); // ストリーミングサウンド用voiceの初期化
	void _streamReadInitialize(void); // ストリーミングの初期化

private:
	static int _refCount;

	//
	// ストリーム再生用
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ストリーム再生に用いるバッファ数とバッファサイズ
    static const int _STREAM_WAVE_BUFFER_NUM = 4;
    static const size_t _STREAM_WAVE_BUFFER_SIZE = 2048;

	enum StreamChannelIndex
	{
		STREAM_CHANNEL_INDEX_LEFT,
		STREAM_CHANNEL_INDEX_RIGHT,
		STREAM_CHANNEL_INDEX_MAX
	};

	// デバイスヒープ管理オブジェクトへのポインタ
	static nn::fnd::ExpHeap* _pDeviceHeap;

	// ファイルリーダー
	static nn::fs::FileReader _fileReader;

	// 波形データバッファ
	static nn::snd::WaveBuffer _streamWaveBufferInfo[_STREAM_WAVE_BUFFER_NUM][STREAM_CHANNEL_INDEX_MAX];
	static void* _pStreamWaveBuffer[_STREAM_WAVE_BUFFER_NUM];

	// チャンネル数
	static s32 _streamChannelCount;

	// 現在再生中のバッファ番号
	static s32 _streamCurrentBufferId;

	// 波形情報
	static nn::snd::Bcwav::WaveInfo _streamWaveInfo;

	// ADPCM情報
	static nn::snd::Bcwav::DspAdpcmInfo _streamAdpcmInfo[STREAM_CHANNEL_INDEX_MAX];

	// ファイルヘッダーサイズ
	static size_t _streamFileHeaderSize;

	static _StreamPosition _streamPosition; // ストリーミング位置管理

	// ストリーム用Voice
	static nn::snd::Voice* _pStreamVoice[STREAM_CHANNEL_INDEX_MAX];

	// ストリーム読み込み状況
	static StreamFileReadState _streamFileReadState;

	// ストリーム再生ボリューム
	static f32 _streamVolume;

	// ストリーム再生ループフラグ
	static bool _bLoop;

	//
	// オンメモリ（ワンショット・ジングル）用
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// 同時再生数
	static const int _ON_MEMORY_VOICE_MAX = 24; // マニュアル手元にないからわかんねー

	// オンメモリ用Voice
	static nn::snd::Voice* _pOnMemoryVoice[_ON_MEMORY_VOICE_MAX];

	// オンメモリ用波形データバッファ管理構造体
	struct OnMemoryWaveBuffer
	{
		nn::snd::WaveBuffer info[2]; // ループ用らしい
		void* pBcwavBuffer;
	};

	// オンメモリ用波形データ管理リスト
	static std::vector<OnMemoryWaveBuffer> _onMemoryBuffers;

	//
	// サウンドスレッド関連
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static nn::os::CriticalSection _criticalSection; // クリティカルセクション

	// サウンドスレッド / ファイルリードスレッド間の通信用キュー
    static nn::os::BlockingQueue _queue;
    static const size_t _QUEUE_BUFFER_SIZE = _STREAM_WAVE_BUFFER_NUM;
    static uptr _queueBuffer[_QUEUE_BUFFER_SIZE];

	static nn::os::Thread _fileReadThread;  // ファイル読み込みスレッド

	static const int _FILE_READ_THREAD_PRIORITY = 4;		 // ファイル読み込みスレッドのプライオリティ（高め）
	static const size_t _FILE_READ_THREAD_STACK_SIZE = 2048; // ファイルリードスレッドスタックサイズ(2KB)

	static bool _bFreadThreadFlag;

	// ファイル読み込みスレッド用関数
	static void _freadThreadFunc(uptr arg);

	static const int _SOUND_THREAD_PRIORITY = 2;		 // サウンドスレッドのプライオリティ（高め）
	static const size_t _SOUND_THREAD_STACK_SIZE = 2048; // サウンドスレッドスタックサイズ(2KB)

	static bool _bSoundThreadFlag;

	static nn::os::Thread _soundThread;  // サウンドスレッド
	
	// サウンドスレット用関数
	static void _soundThreadFunc(uptr arg);
};

#endif // _SOUND_PLAYER_H_
