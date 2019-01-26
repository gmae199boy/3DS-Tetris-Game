/*---------------------------------------------------------------------------*
  File: SoundPlayer.h

   �T�E���h�Đ�
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
	// �X�g���[�~���O�̓ǂݍ��݃|�W�V�������Ǘ�����N���X
	class _StreamPosition
	{
	public:
		_StreamPosition();

		// �|�W�V�����Ȃǂ̏������Z�b�g����
		//
		// loopHeadFrame : ���[�v�擪�ʒu
		// loopTailFrame : ���[�v�I�[�ʒu
		//
		void reset(u32 loopHeadFrame, u32 loopTailFrame);

		// �|�W�V�����Ȃǂ̏������Z�b�g����
		//
		void reset(void);

		// �|�W�V�����̍X�V
		//
		// size : �v������ړ���(�o�C�g)
		//
		// �߂�l : ���ۂɈړ������ړ���(�T���v��)
		s32 update(s32 size, s32 channelCount);

		// �v���p�e�B
		const s32& position(void) const { return _position; }
		const bool& isLooped(void) const { return _isLooped; }

	private:
		u32 _loopHeadFrame; // ���[�v�J�n�t���[��
		u32 _loopTailFrame;	// ���[�v�I���t���[��
		s32 _position;		// �ǂݍ��݃|�W�V����(�T���v��)
		bool _isLooped;		// ���[�v�t���O
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
	void pauseStream(void); // �|�[�Y����������Đ������
	void setStreamVolume(float volume);
	float getStreamVolume(void);

	// �t�@�C���ǂݍ��݃X���b�h�̏��
	enum StreamFileReadState
	{
		STREAM_FILE_READ_STATE_NONE,
		STREAM_FILE_READ_STATE_LOAD,
		STREAM_FILE_READ_STATE_MAX
	};

	const StreamFileReadState& getStreamFileReadState(void) const { return _streamFileReadState; }


	// �߂�l�Fbcwav�����ԍ�
	int loadBcwav(const char* pFileName);

	void unloadAllBcwav(void);

	// �߂�l�Fvoice�Ǘ��ԍ� ���[�v�����~�߂�Ƃ��Ɏg�� -1��������Đ��ł��Ȃ������Ƃ�������
	int play(int bcwavNum, bool bLoop = false);

	void stop(int voiceNum);


private:
	void _streamVoiceInitialize(void); // �X�g���[�~���O�T�E���h�pvoice�̏�����
	void _streamReadInitialize(void); // �X�g���[�~���O�̏�����

private:
	static int _refCount;

	//
	// �X�g���[���Đ��p
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // �X�g���[���Đ��ɗp����o�b�t�@���ƃo�b�t�@�T�C�Y
    static const int _STREAM_WAVE_BUFFER_NUM = 4;
    static const size_t _STREAM_WAVE_BUFFER_SIZE = 2048;

	enum StreamChannelIndex
	{
		STREAM_CHANNEL_INDEX_LEFT,
		STREAM_CHANNEL_INDEX_RIGHT,
		STREAM_CHANNEL_INDEX_MAX
	};

	// �f�o�C�X�q�[�v�Ǘ��I�u�W�F�N�g�ւ̃|�C���^
	static nn::fnd::ExpHeap* _pDeviceHeap;

	// �t�@�C�����[�_�[
	static nn::fs::FileReader _fileReader;

	// �g�`�f�[�^�o�b�t�@
	static nn::snd::WaveBuffer _streamWaveBufferInfo[_STREAM_WAVE_BUFFER_NUM][STREAM_CHANNEL_INDEX_MAX];
	static void* _pStreamWaveBuffer[_STREAM_WAVE_BUFFER_NUM];

	// �`�����l����
	static s32 _streamChannelCount;

	// ���ݍĐ����̃o�b�t�@�ԍ�
	static s32 _streamCurrentBufferId;

	// �g�`���
	static nn::snd::Bcwav::WaveInfo _streamWaveInfo;

	// ADPCM���
	static nn::snd::Bcwav::DspAdpcmInfo _streamAdpcmInfo[STREAM_CHANNEL_INDEX_MAX];

	// �t�@�C���w�b�_�[�T�C�Y
	static size_t _streamFileHeaderSize;

	static _StreamPosition _streamPosition; // �X�g���[�~���O�ʒu�Ǘ�

	// �X�g���[���pVoice
	static nn::snd::Voice* _pStreamVoice[STREAM_CHANNEL_INDEX_MAX];

	// �X�g���[���ǂݍ��ݏ�
	static StreamFileReadState _streamFileReadState;

	// �X�g���[���Đ��{�����[��
	static f32 _streamVolume;

	// �X�g���[���Đ����[�v�t���O
	static bool _bLoop;

	//
	// �I���������i�����V���b�g�E�W���O���j�p
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// �����Đ���
	static const int _ON_MEMORY_VOICE_MAX = 24; // �}�j���A���茳�ɂȂ�����킩��ˁ[

	// �I���������pVoice
	static nn::snd::Voice* _pOnMemoryVoice[_ON_MEMORY_VOICE_MAX];

	// �I���������p�g�`�f�[�^�o�b�t�@�Ǘ��\����
	struct OnMemoryWaveBuffer
	{
		nn::snd::WaveBuffer info[2]; // ���[�v�p�炵��
		void* pBcwavBuffer;
	};

	// �I���������p�g�`�f�[�^�Ǘ����X�g
	static std::vector<OnMemoryWaveBuffer> _onMemoryBuffers;

	//
	// �T�E���h�X���b�h�֘A
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static nn::os::CriticalSection _criticalSection; // �N���e�B�J���Z�N�V����

	// �T�E���h�X���b�h / �t�@�C�����[�h�X���b�h�Ԃ̒ʐM�p�L���[
    static nn::os::BlockingQueue _queue;
    static const size_t _QUEUE_BUFFER_SIZE = _STREAM_WAVE_BUFFER_NUM;
    static uptr _queueBuffer[_QUEUE_BUFFER_SIZE];

	static nn::os::Thread _fileReadThread;  // �t�@�C���ǂݍ��݃X���b�h

	static const int _FILE_READ_THREAD_PRIORITY = 4;		 // �t�@�C���ǂݍ��݃X���b�h�̃v���C�I���e�B�i���߁j
	static const size_t _FILE_READ_THREAD_STACK_SIZE = 2048; // �t�@�C�����[�h�X���b�h�X�^�b�N�T�C�Y(2KB)

	static bool _bFreadThreadFlag;

	// �t�@�C���ǂݍ��݃X���b�h�p�֐�
	static void _freadThreadFunc(uptr arg);

	static const int _SOUND_THREAD_PRIORITY = 2;		 // �T�E���h�X���b�h�̃v���C�I���e�B�i���߁j
	static const size_t _SOUND_THREAD_STACK_SIZE = 2048; // �T�E���h�X���b�h�X�^�b�N�T�C�Y(2KB)

	static bool _bSoundThreadFlag;

	static nn::os::Thread _soundThread;  // �T�E���h�X���b�h
	
	// �T�E���h�X���b�g�p�֐�
	static void _soundThreadFunc(uptr arg);
};

#endif // _SOUND_PLAYER_H_
