#include "SoundPlayer.h"

#include <nn/snd.h>
#include <algorithm>

nn::os::CriticalSection SoundPlayer::_criticalSection;   // �N���e�B�J���Z�N�V����
s32 SoundPlayer::_streamChannelCount = 2;				 // �X�e���I2ch
nn::fs::FileReader SoundPlayer::_fileReader;			 // �t�@�C���ǂݍ��݃N���X
nn::fnd::ExpHeap* SoundPlayer::_pDeviceHeap = NULL;		 // �f�o�C�X�q�[�v�Ǘ��N���X

// �L���[�o�b�t�@�[
uptr SoundPlayer::_queueBuffer[SoundPlayer::_QUEUE_BUFFER_SIZE] = { NULL };

bool SoundPlayer::_bSoundThreadFlag = true;

nn::os::Thread SoundPlayer::_soundThread; // �T�E���h�X���b�h

// Voice
nn::snd::Voice* SoundPlayer::_pStreamVoice[SoundPlayer::STREAM_CHANNEL_INDEX_MAX] = { NULL };

bool SoundPlayer::_bFreadThreadFlag = true;

nn::os::Thread SoundPlayer::_fileReadThread; // �t�@�C�����[�h�X���b�h

SoundPlayer::_StreamPosition SoundPlayer::_streamPosition; // �X�g���[���|�W�V�����Ǘ��N���X

nn::snd::Bcwav::WaveInfo SoundPlayer::_streamWaveInfo; // �X�g���[���������

// �X�g���[��ADPCM���
nn::snd::Bcwav::DspAdpcmInfo SoundPlayer::_streamAdpcmInfo[SoundPlayer::STREAM_CHANNEL_INDEX_MAX];

// �X�g���[�������o�b�t�@
void* SoundPlayer::_pStreamWaveBuffer[SoundPlayer::_STREAM_WAVE_BUFFER_NUM] = { NULL };

// �X�g���[���t�@�C���w�b�_�̃T�C�Y
std::size_t SoundPlayer::_streamFileHeaderSize = 0;

// �X�g���[�������o�b�t�@���
nn::snd::WaveBuffer SoundPlayer::_streamWaveBufferInfo[SoundPlayer::_STREAM_WAVE_BUFFER_NUM][SoundPlayer::STREAM_CHANNEL_INDEX_MAX];

// �X�g���[���J�����g�o�b�t�@�ԍ�
s32 SoundPlayer::_streamCurrentBufferId = 0;

// �L���[
nn::os::BlockingQueue SoundPlayer::_queue;

// ���t�@�����X�J�E���^
s32 SoundPlayer::_refCount = 0;

// �X�g���[���t�@�C���ǂݍ��ݏ��
SoundPlayer::StreamFileReadState SoundPlayer::_streamFileReadState = SoundPlayer::STREAM_FILE_READ_STATE_NONE;

// �X�g���[������(0�`1)
f32 SoundPlayer::_streamVolume = 1.0f;

// �X�g���[�����[�v�t���O
bool SoundPlayer::_bLoop = false;

// �I���������pVoice
nn::snd::Voice* SoundPlayer::_pOnMemoryVoice[SoundPlayer::_ON_MEMORY_VOICE_MAX];

// �I���������p�g�`�f�[�^�Ǘ����X�g
std::vector<SoundPlayer::OnMemoryWaveBuffer> SoundPlayer::_onMemoryBuffers;


SoundPlayer::SoundPlayer(nn::fnd::ExpHeap* pDeviceHeap)
{
	if( _refCount++ ) return;

	_pDeviceHeap = pDeviceHeap;

	// dsp, snd �̏�����
	NN_UTIL_PANIC_IF_FAILED(nn::dsp::Initialize());
	NN_UTIL_PANIC_IF_FAILED(nn::dsp::LoadDefaultComponent());
	NN_UTIL_PANIC_IF_FAILED(nn::snd::Initialize());

	// �N���e�B�J���Z�N�V�����̏�����
	_criticalSection.Initialize();

	// �T�E���h�o�͂��X�e���I��
	nn::snd::SetSoundOutputMode(nn::snd::OUTPUT_MODE_STEREO);

	// �}�X�^�[�{�����[����ݒ�
	nn::snd::SetMasterVolume(1.0);

	/* -------------------------------- */

	// �f�[�^�o�b�t�@�̏�������уf�[�^����
	for (s32 i = 0; i < _STREAM_WAVE_BUFFER_NUM; i++) {   
		_pStreamWaveBuffer[i] = _pDeviceHeap->Allocate(_STREAM_WAVE_BUFFER_SIZE, 32);
		NN_ASSERT(_pStreamWaveBuffer[i]);
	}

	// �T�E���h�X���b�h�̏�����
	_bSoundThreadFlag = true;
	_soundThread.StartUsingAutoStack(
		_soundThreadFunc,
		reinterpret_cast<uptr>(this),
		_SOUND_THREAD_STACK_SIZE,
		_SOUND_THREAD_PRIORITY
		);

	// �t�@�C���ǂݍ��݃X���b�h�̏�����
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

	// �f�[�^�o�b�t�@�̔j��
	for( s32 i = _STREAM_WAVE_BUFFER_NUM - 1; i >= 0; i-- ) {   
		_pDeviceHeap->Free(_pStreamWaveBuffer[i]);
		_pStreamWaveBuffer[i] = NULL;
	}

	// dsp, snd �̏I������
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
	// ���[�v�������ǂ����𔻒�
	_isLooped = ( _position == _loopTailFrame );
	if( _isLooped ) {
		_position = _loopHeadFrame;
	}

	// �v�����ꂽ�ړ���(�o�C�g)���T���v�����ɕϊ�
	s32 nSamplesRequired = nn::snd::GetSampleLength(size, nn::snd::SAMPLE_FORMAT_ADPCM, channelCount);

	s32 ret; // �ړ��T���v����

	// �v�����ꂽ�T���v�����܂�܂�ړ��ł��邩���؂���
	if( nSamplesRequired < _loopTailFrame - _position ) {
		ret = nSamplesRequired; // �ړ��ł���̂ŗv�����ꂽ�T���v�������ړ���
	}
	else {
		// �I���t���[���ɓ��B���Ă��܂��̂ŗv�����ꂽ���͈ړ��ł��Ȃ�
		// ����Č��݂̈ʒu����I���t���[���܂ł̕����ړ��T���v�����Ƃ���
		ret = _loopTailFrame - _position;
	}

	_position += ret; // �ړ�

	return ret; // �ړ������T���v������Ԃ�
}

void SoundPlayer::_streamVoiceInitialize(void)
{
	// �X�g���[���T�E���h�p Voice �̏�����(�X�e���I2ch)
	for( s32 ch = 0; ch < _streamChannelCount; ch++ ) {

		// (���\��voice������...����...����ł����̂�...�ՂɂȂ�...�������)

		if( _pStreamVoice[ch] ) nn::snd::FreeVoice(_pStreamVoice[ch]); // �����Ă���E��

		_pStreamVoice[ch] = nn::snd::AllocVoice(128, NULL, NULL); // ���
		NN_ASSERT(_pStreamVoice[ch]);

		nn::snd::MixParam mix;
		mix.mainBus[nn::snd::CHANNEL_INDEX_FRONT_LEFT ] = ch == 0 ? 0.707f : 0.0f; // ���C���{�����[�� (L)
		mix.mainBus[nn::snd::CHANNEL_INDEX_FRONT_RIGHT] = ( _streamChannelCount == 1 || ch == 1 ) ? 0.707f : 0.0f; // ���C���{�����[�� (R)
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
	// voice�̏�����
	_streamVoiceInitialize();

	// �t�@�C���ʒu�̏�����
	_fileReader.Seek(_streamFileHeaderSize, nn::fs::POSITION_BASE_BEGIN);


	for( s32 i = 0; i < _STREAM_WAVE_BUFFER_NUM; i++ ) {

		// �t�@�C���ǂݍ��݈˗�
		_queue.Enqueue(reinterpret_cast<uptr>(_pStreamWaveBuffer[i]));

		// �ǂݍ��݈ʒu�̍X�V
		s32 sampleLength = _streamPosition.update(_STREAM_WAVE_BUFFER_SIZE, _streamChannelCount);

		// �o�b�t�@�̏�����
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
	// �Đ��o�b�t�@�ԍ��̏�����
	_streamCurrentBufferId = 0;

	// �t�@�C���ǂݍ���
	_fileReader.Finalize(); // ���͂́A�w������������Ă��A����ő��v
	_fileReader.Initialize(pFileName);

	// �X�g���[���Đ������r�؂�Ȃ��悤�ɁA�t�@�C���ǂݍ��ݗD��x�������ݒ肵�Ă���
	_fileReader.SetPriority(nn::fs::PRIORITY_APP_REALTIME);

	// �Ƃ肠���� 1KB �ǂ�
	void* pBcwavHeader = _pDeviceHeap->Allocate(0x400, 32);
	_fileReader.Read(pBcwavHeader, 0x400);

	// �g�`���̎擾
	_streamWaveInfo = nn::snd::Bcwav::GetWaveInfo(pBcwavHeader);

	// �X�g���[�~���O�ʒu�̃��Z�b�g
	_streamPosition.reset(_streamWaveInfo.loopStartFrame, _streamWaveInfo.loopEndFrame);

	// �`�����l�����̊m�F
	_streamChannelCount = nn::snd::Bcwav::GetChannelCount(pBcwavHeader);

	// ADPCM���̎擾
	if( _streamWaveInfo.encoding == nn::snd::Bcwav::ENCODING_DSP_ADPCM ) {
		for( s32 ch = 0; ch < _streamChannelCount; ch++ ) {
			_streamAdpcmInfo[ch] = *nn::snd::Bcwav::GetDspAdpcmInfo(pBcwavHeader, ch);
		}
	}

	// �t�@�C���擪����f�[�^�܂ł̃I�t�Z�b�g���擾
	const void* pBcwavBody = nn::snd::Bcwav::GetWave(pBcwavHeader, nn::snd::Bcwav::CHANNEL_INDEX_L);
	_streamFileHeaderSize = reinterpret_cast<size_t>(pBcwavBody) - reinterpret_cast<size_t>(pBcwavHeader);

	// �Ƃ肠�����œǂݍ��񂾕�����j��
	if( pBcwavHeader ) {
		_pDeviceHeap->Free(pBcwavHeader);
	}

	// �X�g���[�~���O�̏�����
	_streamReadInitialize();
}

void SoundPlayer::playStream(bool bLoop)
{
	_bLoop = bLoop; // ���[�v�t���O

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

	// �X�g���[�~���O�ʒu�̃��Z�b�g
	_streamPosition.reset();

	// �X�g���[�~���O�̏�����
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

		nn::snd::WaitForDspSync();  // DSP ����̃f�[�^��M��҂B

		// �o�b�t�@���Đ��ς݂Ȃ�V���ɉ������[�h���ēo�^
		// �����`�����l�������Ď�����Α��v����ˁH
		if( p->_pStreamVoice[STREAM_CHANNEL_INDEX_LEFT] != NULL &&
			p->_pStreamVoice[STREAM_CHANNEL_INDEX_LEFT]->GetState() == nn::snd::Voice::STATE_PLAY &&
			p->_streamWaveBufferInfo[p->_streamCurrentBufferId][STREAM_CHANNEL_INDEX_LEFT].status == nn::snd::WaveBuffer::STATUS_DONE ) 
		{
			// �t�@�C���ǂݍ��݃X���b�h�ɓǂݍ��݂��肢�M���𔭐M
			s32 id = p->_streamCurrentBufferId;
			p->_queue.Enqueue(reinterpret_cast<uptr>(p->_pStreamWaveBuffer[id]));

			// �ǂݍ��݈ʒu�̍X�V
			s32 sampleLength = p->_streamPosition.update(_STREAM_WAVE_BUFFER_SIZE, p->_streamChannelCount);

			for( s32 ch = 0; ch < p->_streamChannelCount; ch++ ) {

				nn::snd::WaveBuffer* pBuf = &p->_streamWaveBufferInfo[id][ch];

				// �o�b�t�@�̍Ēǉ�
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
		nn::snd::SendParameterToDsp();  // �p�����[�^�� DSP �ɑ��M�B
	}
}

// �t�@�C���ǂݍ��݃X���b�h�p�֐�
void SoundPlayer::_freadThreadFunc(uptr arg)
{
	SoundPlayer* p = reinterpret_cast<SoundPlayer*>(arg);

	s32 offset = 0;

	while( _bSoundThreadFlag ) {

		// 5msecs �����ɃL���[���m�F
		nn::os::Thread::Sleep(nn::fnd::TimeSpan::FromMilliSeconds(5));

		uptr address;
		while( p->_queue.TryDequeue(&address) ) {

			p->_streamFileReadState = STREAM_FILE_READ_STATE_LOAD;

			// �t�@�C���ǂݍ���
			void* pBuffer = reinterpret_cast<void*>(address);
			size_t size = _STREAM_WAVE_BUFFER_SIZE;
			size_t sizeRead = p->_fileReader.Read(pBuffer, size);
			offset += sizeRead;
			nn::snd::FlushDataCache(reinterpret_cast<uptr>(pBuffer), size);

			// �t�@�C�������܂œǂ�ł���΁A�J��Ԃ��ʒu�փW�����v
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
	NN_ASSERTMSG(fileSize, "bcwav�t�@�C�����ǂݍ��߂܂���ł��� => %s\n", pFileName);
    
	OnMemoryWaveBuffer buffer;
	buffer.pBcwavBuffer = reinterpret_cast<u8*>(_pDeviceHeap->Allocate(fileSize, 32));
	NN_ASSERTMSG(buffer.pBcwavBuffer, "%d���̃T�E���h�p���������m�ۂł��܂���ł����`\n", fileSize);

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
			// ������胋�[�v�ɂ���
			for( int j = 0; j < 2; j++ ) {
				_onMemoryBuffers[bcwavNum].info[j].loopFlag = true;
			}
		}

		// ���ʂƂ�
		nn::snd::MixParam mix;
		mix.mainBus[nn::snd::CHANNEL_INDEX_FRONT_LEFT ] = 0.7f;
		mix.mainBus[nn::snd::CHANNEL_INDEX_FRONT_RIGHT] = 0.7f;
		_pOnMemoryVoice[i]->SetMixParam(mix);

		// �Đ�
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
