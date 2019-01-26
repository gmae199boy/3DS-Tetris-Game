#include "SoundPlayer.h"

#include <nn/snd.h>
#include <algorithm>

nn::os::CriticalSection SoundPlayer::_criticalSection;   // クリティカルセクション
s32 SoundPlayer::_streamChannelCount = 2;				 // ステレオ2ch
nn::fs::FileReader SoundPlayer::_fileReader;			 // ファイル読み込みクラス
nn::fnd::ExpHeap* SoundPlayer::_pDeviceHeap = NULL;		 // デバイスヒープ管理クラス

// キューバッファー
uptr SoundPlayer::_queueBuffer[SoundPlayer::_QUEUE_BUFFER_SIZE] = { NULL };

bool SoundPlayer::_bSoundThreadFlag = true;

nn::os::Thread SoundPlayer::_soundThread; // サウンドスレッド

// Voice
nn::snd::Voice* SoundPlayer::_pStreamVoice[SoundPlayer::STREAM_CHANNEL_INDEX_MAX] = { NULL };

bool SoundPlayer::_bFreadThreadFlag = true;

nn::os::Thread SoundPlayer::_fileReadThread; // ファイルリードスレッド

SoundPlayer::_StreamPosition SoundPlayer::_streamPosition; // ストリームポジション管理クラス

nn::snd::Bcwav::WaveInfo SoundPlayer::_streamWaveInfo; // ストリーム音源情報

// ストリームADPCM情報
nn::snd::Bcwav::DspAdpcmInfo SoundPlayer::_streamAdpcmInfo[SoundPlayer::STREAM_CHANNEL_INDEX_MAX];

// ストリーム音源バッファ
void* SoundPlayer::_pStreamWaveBuffer[SoundPlayer::_STREAM_WAVE_BUFFER_NUM] = { NULL };

// ストリームファイルヘッダのサイズ
std::size_t SoundPlayer::_streamFileHeaderSize = 0;

// ストリーム音源バッファ情報
nn::snd::WaveBuffer SoundPlayer::_streamWaveBufferInfo[SoundPlayer::_STREAM_WAVE_BUFFER_NUM][SoundPlayer::STREAM_CHANNEL_INDEX_MAX];

// ストリームカレントバッファ番号
s32 SoundPlayer::_streamCurrentBufferId = 0;

// キュー
nn::os::BlockingQueue SoundPlayer::_queue;

// リファレンスカウンタ
s32 SoundPlayer::_refCount = 0;

// ストリームファイル読み込み状態
SoundPlayer::StreamFileReadState SoundPlayer::_streamFileReadState = SoundPlayer::STREAM_FILE_READ_STATE_NONE;

// ストリーム音量(0～1)
f32 SoundPlayer::_streamVolume = 1.0f;

// ストリームループフラグ
bool SoundPlayer::_bLoop = false;

// オンメモリ用Voice
nn::snd::Voice* SoundPlayer::_pOnMemoryVoice[SoundPlayer::_ON_MEMORY_VOICE_MAX];

// オンメモリ用波形データ管理リスト
std::vector<SoundPlayer::OnMemoryWaveBuffer> SoundPlayer::_onMemoryBuffers;


SoundPlayer::SoundPlayer(nn::fnd::ExpHeap* pDeviceHeap)
{
	if( _refCount++ ) return;

	_pDeviceHeap = pDeviceHeap;

	// dsp, snd の初期化
	NN_UTIL_PANIC_IF_FAILED(nn::dsp::Initialize());
	NN_UTIL_PANIC_IF_FAILED(nn::dsp::LoadDefaultComponent());
	NN_UTIL_PANIC_IF_FAILED(nn::snd::Initialize());

	// クリティカルセクションの初期化
	_criticalSection.Initialize();

	// サウンド出力をステレオに
	nn::snd::SetSoundOutputMode(nn::snd::OUTPUT_MODE_STEREO);

	// マスターボリュームを設定
	nn::snd::SetMasterVolume(1.0);

	/* -------------------------------- */

	// データバッファの準備およびデータ生成
	for (s32 i = 0; i < _STREAM_WAVE_BUFFER_NUM; i++) {   
		_pStreamWaveBuffer[i] = _pDeviceHeap->Allocate(_STREAM_WAVE_BUFFER_SIZE, 32);
		NN_ASSERT(_pStreamWaveBuffer[i]);
	}

	// サウンドスレッドの初期化
	_bSoundThreadFlag = true;
	_soundThread.StartUsingAutoStack(
		_soundThreadFunc,
		reinterpret_cast<uptr>(this),
		_SOUND_THREAD_STACK_SIZE,
		_SOUND_THREAD_PRIORITY
		);

	// ファイル読み込みスレッドの初期化
	_bFreadThreadFlag = true;
	_queue.Initialize(_queueBuffer, _QUEUE_BUFFER_SIZE);
	_fileReadThread.StartUsingAutoStack(
		_freadThreadFunc,
		reinterpret_cast<uptr>(this),
		_FILE_READ_THREAD_STACK_SIZE,
		_FILE_READ_THREAD_PRIORITY
		);
}

SoundPlayer::~SoundPlayer()
{
	if( --_refCount ) return;

    _bSoundThreadFlag = false;
    _soundThread.Join();
    _soundThread.Finalize();

    _bFreadThreadFlag = false;
    _fileReadThread.Join();
    _fileReadThread.Finalize();

    _criticalSection.Finalize();

	unloadAllBcwav();

	for( int i = 0; i < _ON_MEMORY_VOICE_MAX; i++ ) {
		if( _pOnMemoryVoice[i] ) { 
			nn::snd::FreeVoice(_pOnMemoryVoice[i]);
			_pOnMemoryVoice[i] = NULL;
		}
	}

	// データバッファの破棄
	for( s32 i = _STREAM_WAVE_BUFFER_NUM - 1; i >= 0; i-- ) {   
		_pDeviceHeap->Free(_pStreamWaveBuffer[i]);
		_pStreamWaveBuffer[i] = NULL;
	}

	// dsp, snd の終了処理
	NN_UTIL_PANIC_IF_FAILED(nn::snd::Finalize());
	NN_UTIL_PANIC_IF_FAILED(nn::dsp::UnloadComponent());
	nn::dsp::Finalize();
}

SoundPlayer::_StreamPosition::_StreamPosition()
	: _loopHeadFrame(0)
	, _loopTailFrame(0)
	, _position(0)
	, _isLooped(false)
{
}

void SoundPlayer::_StreamPosition::reset(u32 loopHeadFrame, u32 loopTailFrame)
{
	_loopHeadFrame = loopHeadFrame;
	_loopTailFrame = loopTailFrame;

	reset();
}

void SoundPlayer::_StreamPosition::reset(void)
{
	_position = 0;
	_isLooped = false;
}

s32 SoundPlayer::_StreamPosition::update(s32 size, s32 channelCount)
{
	// ループしたかどうかを判定
	_isLooped = ( _position == _loopTailFrame );
	if( _isLooped ) {
		_position = _loopHeadFrame;
	}

	// 要求された移動量(バイト)をサンプル数に変換
	s32 nSamplesRequired = nn::snd::GetSampleLength(size, nn::snd::SAMPLE_FORMAT_ADPCM, channelCount);

	s32 ret; // 移動サンプル数

	// 要求されたサンプル数まるまる移動できるか検証する
	if( nSamplesRequired < _loopTailFrame - _position ) {
		ret = nSamplesRequired; // 移動できるので要求されたサンプル数が移動量
	}
	else {
		// 終了フレームに到達してしまうので要求された分は移動できない
		// よって現在の位置から終了フレームまでの分を移動サンプル数とする
		ret = _loopTailFrame - _position;
	}

	_position += ret; // 移動

	return ret; // 移動したサンプル数を返す
}

void SoundPlayer::_streamVoiceInitialize(void)
{
	// ストリームサウンド用 Voice の初期化(ステレオ2ch)
	for( s32 ch = 0; ch < _streamChannelCount; ch++ ) {

		// (乱暴なvoice初期化...いい...これでいいのだ...虎になれ...がるるるる)

		if( _pStreamVoice[ch] ) nn::snd::FreeVoice(_pStreamVoice[ch]); // 生きてたら殺す

		_pStreamVoice[ch] = nn::snd::AllocVoice(128, NULL, NULL); // 作る
		NN_ASSERT(_pStreamVoice[ch]);

		nn::snd::MixParam mix;
		mix.mainBus[nn::snd::CHANNEL_INDEX_FRONT_LEFT ] = ch == 0 ? 0.707f : 0.0f; // メインボリューム (L)
		mix.mainBus[nn::snd::CHANNEL_INDEX_FRONT_RIGHT] = ( _streamChannelCount == 1 || ch == 1 ) ? 0.707f : 0.0f; // メインボリューム (R)
		_pStreamVoice[ch]->SetMixParam(mix);
		_pStreamVoice[ch]->SetVolume(_streamVolume);

		if( _streamWaveInfo.encoding == nn::snd::Bcwav::ENCODING_DSP_ADPCM ) {
			_pStreamVoice[ch]->SetAdpcmParam(_streamAdpcmInfo[ch].param);
		}
		_pStreamVoice[ch]->SetChannelCount(1);
		_pStreamVoice[ch]->SetSampleFormat(static_cast<nn::snd::SampleFormat>(_streamWaveInfo.encoding));
		_pStreamVoice[ch]->SetSampleRate(_streamWaveInfo.sampleRate);
		_pStreamVoice[ch]->SetPitch(1.0);
	}
}

void SoundPlayer::_streamReadInitialize(void)
{
	// voiceの初期化
	_streamVoiceInitialize();

	// ファイル位置の初期化
	_fileReader.Seek(_streamFileHeaderSize, nn::fs::POSITION_BASE_BEGIN);


	for( s32 i = 0; i < _STREAM_WAVE_BUFFER_NUM; i++ ) {

		// ファイル読み込み依頼
		_queue.Enqueue(reinterpret_cast<uptr>(_pStreamWaveBuffer[i]));

		// 読み込み位置の更新
		s32 sampleLength = _streamPosition.update(_STREAM_WAVE_BUFFER_SIZE, _streamChannelCount);

		// バッファの初期化
		for( s32 ch = 0; ch < _streamChannelCount; ch++ ) {

			nn::snd::InitializeWaveBuffer(&_streamWaveBufferInfo[i][ch]);
			_streamWaveBufferInfo[i][ch].bufferAddress = _pStreamWaveBuffer[i];
			_streamWaveBufferInfo[i][ch].sampleLength  = sampleLength;
			_streamWaveBufferInfo[i][ch].loopFlag  = false;
			if (i == 0) {
				_streamWaveBufferInfo[i][ch].pAdpcmContext = &_streamAdpcmInfo[ch].context;
			}
			else if( _streamPosition.isLooped() ) {
				_streamWaveBufferInfo[i][ch].pAdpcmContext = &_streamAdpcmInfo[ch].loopContext;
			}
			else {
				_streamWaveBufferInfo[i][ch].pAdpcmContext = NULL;
			}

			_pStreamVoice[ch]->AppendWaveBuffer(&_streamWaveBufferInfo[i][ch]);		
		}
	}
}

void SoundPlayer::preLoadStream(const char* pFileName)
{
	// 再生バッファ番号の初期化
	_streamCurrentBufferId = 0;

	// ファイル読み込み
	_fileReader.Finalize(); // がはは、学生が無茶やっても、これで大丈夫
	_fileReader.Initialize(pFileName);

	// ストリーム再生音が途切れないように、ファイル読み込み優先度を高く設定しておく
	_fileReader.SetPriority(nn::fs::PRIORITY_APP_REALTIME);

	// とりあえず 1KB 読む
	void* pBcwavHeader = _pDeviceHeap->Allocate(0x400, 32);
	_fileReader.Read(pBcwavHeader, 0x400);

	// 波形情報の取得
	_streamWaveInfo = nn::snd::Bcwav::GetWaveInfo(pBcwavHeader);

	// ストリーミング位置のリセット
	_streamPosition.reset(_streamWaveInfo.loopStartFrame, _streamWaveInfo.loopEndFrame);

	// チャンネル数の確認
	_streamChannelCount = nn::snd::Bcwav::GetChannelCount(pBcwavHeader);

	// ADPCM情報の取得
	if( _streamWaveInfo.encoding == nn::snd::Bcwav::ENCODING_DSP_ADPCM ) {
		for( s32 ch = 0; ch < _streamChannelCount; ch++ ) {
			_streamAdpcmInfo[ch] = *nn::snd::Bcwav::GetDspAdpcmInfo(pBcwavHeader, ch);
		}
	}

	// ファイル先頭からデータまでのオフセットを取得
	const void* pBcwavBody = nn::snd::Bcwav::GetWave(pBcwavHeader, nn::snd::Bcwav::CHANNEL_INDEX_L);
	_streamFileHeaderSize = reinterpret_cast<size_t>(pBcwavBody) - reinterpret_cast<size_t>(pBcwavHeader);

	// とりあえずで読み込んだ部分を破棄
	if( pBcwavHeader ) {
		_pDeviceHeap->Free(pBcwavHeader);
	}

	// ストリーミングの初期化
	_streamReadInitialize();
}

void SoundPlayer::playStream(bool bLoop)
{
	_bLoop = bLoop; // ループフラグ

	nn::os::CriticalSection::ScopedLock lock(_criticalSection);

	for( s32 ch = 0; ch < _streamChannelCount; ch++ ) {
		_pStreamVoice[ch]->SetState(nn::snd::Voice::STATE_PLAY);
	}
}

void SoundPlayer::stopStream(void)
{
	nn::os::CriticalSection::ScopedLock lock(_criticalSection);

	for( s32 ch = 0; ch < _streamChannelCount; ch++ ) {
		_pStreamVoice[ch]->SetState(nn::snd::Voice::STATE_STOP);
	}

	// ストリーミング位置のリセット
	_streamPosition.reset();

	// ストリーミングの初期化
	_streamReadInitialize();
}

void SoundPlayer::pauseStream(void)
{
	nn::os::CriticalSection::ScopedLock lock(_criticalSection);

	for( s32 ch = 0; ch < _streamChannelCount; ch++ ) {
		_pStreamVoice[ch]->SetState(_pStreamVoice[ch]->GetState() == nn::snd::Voice::STATE_PAUSE ? nn::snd::Voice::STATE_PLAY : nn::snd::Voice::STATE_PAUSE);
	}
}

void SoundPlayer::setStreamVolume(float volume)
{
	if( volume > 1.0f ) volume = 1.0f;
	if( volume < 0.0f ) volume = 0.0f;

	_streamVolume = volume;

	nn::os::CriticalSection::ScopedLock lock(_criticalSection);

	for( s32 ch = 0; ch < _streamChannelCount; ch++ ) {
		_pStreamVoice[ch]->SetVolume(volume);
	}
}

float SoundPlayer::getStreamVolume(void)
{
	return _streamVolume;
}

void SoundPlayer::_soundThreadFunc(uptr arg)
{
	SoundPlayer* p = reinterpret_cast<SoundPlayer*>(arg);

	while( _bSoundThreadFlag ) {

		nn::snd::WaitForDspSync();  // DSP からのデータ受信を待つ。

		// バッファが再生済みなら新たに音をロードして登録
		// ※左チャンネルだけ監視すれば大丈夫だよね？
		if( p->_pStreamVoice[STREAM_CHANNEL_INDEX_LEFT] != NULL &&
			p->_pStreamVoice[STREAM_CHANNEL_INDEX_LEFT]->GetState() == nn::snd::Voice::STATE_PLAY &&
			p->_streamWaveBufferInfo[p->_streamCurrentBufferId][STREAM_CHANNEL_INDEX_LEFT].status == nn::snd::WaveBuffer::STATUS_DONE ) 
		{
			// ファイル読み込みスレッドに読み込みお願い信号を発信
			s32 id = p->_streamCurrentBufferId;
			p->_queue.Enqueue(reinterpret_cast<uptr>(p->_pStreamWaveBuffer[id]));

			// 読み込み位置の更新
			s32 sampleLength = p->_streamPosition.update(_STREAM_WAVE_BUFFER_SIZE, p->_streamChannelCount);

			for( s32 ch = 0; ch < p->_streamChannelCount; ch++ ) {

				nn::snd::WaveBuffer* pBuf = &p->_streamWaveBufferInfo[id][ch];

				// バッファの再追加
				nn::snd::InitializeWaveBuffer(pBuf);
				pBuf->bufferAddress = p->_pStreamWaveBuffer[id];
				pBuf->sampleLength  = sampleLength;
				pBuf->loopFlag  = false;
				if( p->_streamPosition.isLooped() ) {
					pBuf->pAdpcmContext = &p->_streamAdpcmInfo[ch].loopContext;
				}
				else {
					pBuf->pAdpcmContext = NULL;
				}
				p->_pStreamVoice[ch]->AppendWaveBuffer(pBuf);
			}

			p->_streamCurrentBufferId = (p->_streamCurrentBufferId + 1) % _STREAM_WAVE_BUFFER_NUM;
		}

		nn::os::CriticalSection::ScopedLock lock(p->_criticalSection);
		nn::snd::SendParameterToDsp();  // パラメータを DSP に送信。
	}
}

// ファイル読み込みスレッド用関数
void SoundPlayer::_freadThreadFunc(uptr arg)
{
	SoundPlayer* p = reinterpret_cast<SoundPlayer*>(arg);

	s32 offset = 0;

	while( _bSoundThreadFlag ) {

		// 5msecs おきにキューを確認
		nn::os::Thread::Sleep(nn::fnd::TimeSpan::FromMilliSeconds(5));

		uptr address;
		while( p->_queue.TryDequeue(&address) ) {

			p->_streamFileReadState = STREAM_FILE_READ_STATE_LOAD;

			// ファイル読み込み
			void* pBuffer = reinterpret_cast<void*>(address);
			size_t size = _STREAM_WAVE_BUFFER_SIZE;
			size_t sizeRead = p->_fileReader.Read(pBuffer, size);
			offset += sizeRead;
			nn::snd::FlushDataCache(reinterpret_cast<uptr>(pBuffer), size);

			// ファイル末尾まで読んでいれば、繰り返し位置へジャンプ
			if( offset >= nn::snd::Bcwav::FrameToByte(p->_streamWaveInfo.encoding, p->_streamWaveInfo.loopEndFrame) ) {
				offset = nn::snd::Bcwav::FrameToByte(p->_streamWaveInfo.encoding, p->_streamWaveInfo.loopStartFrame);
				p->_fileReader.Seek(p->_streamFileHeaderSize + offset, nn::fs::POSITION_BASE_BEGIN);
				if( !p->_bLoop ) {
					p->stopStream();
					break;
				}
			}
		}

		p->_streamFileReadState = STREAM_FILE_READ_STATE_NONE;
	}
}

int SoundPlayer::loadBcwav(const char* pFileName)
{
    nn::fs::FileReader fileReader = nn::fs::FileReader(pFileName);
    size_t fileSize = fileReader.GetSize();
	NN_ASSERTMSG(fileSize, "bcwavファイルが読み込めませんでした => %s\n", pFileName);
    
	OnMemoryWaveBuffer buffer;
	buffer.pBcwavBuffer = reinterpret_cast<u8*>(_pDeviceHeap->Allocate(fileSize, 32));
	NN_ASSERTMSG(buffer.pBcwavBuffer, "%d分のサウンド用メモリが確保できませんでした～\n", fileSize);

    size_t readSize = fileReader.Read(buffer.pBcwavBuffer, fileSize);
    NN_LOG("[%s] => loadSize(%d) / fileSize(%d)\n", pFileName, readSize, fileSize);
	NN_ASSERT(fileSize == readSize);
    
	fileReader.Finalize();
	
	nn::snd::FlushDataCache(reinterpret_cast<uptr>(buffer.pBcwavBuffer), readSize);
	
	_onMemoryBuffers.push_back(buffer);

	return _onMemoryBuffers.size() - 1;
}

int SoundPlayer::play(int bcwavNum, bool bLoop)
{
	for( int i = 0; i < _ON_MEMORY_VOICE_MAX; i++ ) {

		if( _pOnMemoryVoice[i] && _pOnMemoryVoice[i]->IsPlaying() ) {
			continue;
		}
		else if( _pOnMemoryVoice[i] ) {
			nn::snd::FreeVoice(_pOnMemoryVoice[i]);
			_pOnMemoryVoice[i] = NULL;
		}

		if( !_pOnMemoryVoice[i] ) {
			_pOnMemoryVoice[i] = nn::snd::AllocVoice(128, NULL, NULL);
		}

		_pOnMemoryVoice[i]->SetupBcwav(reinterpret_cast<uptr>(_onMemoryBuffers[bcwavNum].pBcwavBuffer),
			&_onMemoryBuffers[bcwavNum].info[0], &_onMemoryBuffers[bcwavNum].info[1]);

		if( bLoop ) {
			// 無理やりループにする
			for( int j = 0; j < 2; j++ ) {
				_onMemoryBuffers[bcwavNum].info[j].loopFlag = true;
			}
		}

		// 音量とか
		nn::snd::MixParam mix;
		mix.mainBus[nn::snd::CHANNEL_INDEX_FRONT_LEFT ] = 0.7f;
		mix.mainBus[nn::snd::CHANNEL_INDEX_FRONT_RIGHT] = 0.7f;
		_pOnMemoryVoice[i]->SetMixParam(mix);

		// 再生
		_pOnMemoryVoice[i]->SetState(nn::snd::Voice::STATE_PLAY);

		return i;
	}

	return -1;
}

void SoundPlayer::stop(int voiceNum)
{
	if( voiceNum < 0 ) return;

	_pOnMemoryVoice[voiceNum]->SetState(nn::snd::Voice::STATE_STOP);
}

void SoundPlayer::unloadAllBcwav(void)
{
	std::vector<OnMemoryWaveBuffer>::iterator it = _onMemoryBuffers.begin();
	for( ; it != _onMemoryBuffers.end(); ++it) {
		_pDeviceHeap->Free(it->pBcwavBuffer);
	}

	_onMemoryBuffers.clear();
}
