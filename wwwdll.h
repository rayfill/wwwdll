#ifndef HTMLMODIFYCHECKER_HPP_
#define HTMLMODIFYCHECKER_HPP_

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * �X���b�h�R���e�L�X�g�̍쐬
	 * @return �X���b�h�R���e�L�X�g�n���h��
	 */
	void* __stdcall ThreadCreate();
	
	/**
	 * �X���b�h�R���e�L�X�g�̎��s
	 * @param context ���s����HTTP�R���e�L�X�g
	 * @param hWnd �R�[���o�b�N����Window�̃n���h��
	 * @param message �R�[���o�b�N����E�B���h�E���b�Z�[�WID
	 * @note �R�[���o�b�N�����E�B���h�E���b�Z�[�W��wParam�Ɋ֐��̎��s����
	 * (1�������A0�����s)�AlParam�ɃX���b�h�R���e�L�X�g��������B
	 */
	void __stdcall ThreadStart(void* context,
							   void* httpContext,
							   HWND hWnd,
							   const unsigned int message);

	/**
	 *  �X���b�h�̏I���҂�
	 * @param threadContext �X���b�h�R���e�L�X�g�̃n���h��
	 * @return ���݂ł�0�Œ�
	 * (�]�T������������s���������̖߂�l��Ԃ��悤�ɕύX���܂�)
	 */
	long __stdcall ThreadJoin(void* threadContext);

	/**
	 * �X���b�h�R���e�L�X�g�̔j��
	 * @param threadContext �X���b�h�R���e�L�X�g�̃n���h��
	 */
	void __stdcall ThreadClose(void* threadContext);


	/**
	 * Web�R���e���c�擾�R���e�L�X�g�̍쐬
	 * @param url �R���e���c�̂���URL
	 * @param cookie �R���e���c�擾���ɕt�^����Cookie�B�Ȃ��ꍇ��NULL���w�肷��
	 * @param userAgent �R���e���c�擾���Ɏg��UserAgent�B�f�t�H���g�̏ꍇ��NULL
	 * @param timeout ���ʐM�^�C���A�E�g���ԁB
	 * @return ��Ƃ��s��HTTP�R���e�L�X�g�n���h���B
	 */
	void* __stdcall HTTPCreateContext(const char* url,
									  const char* cookie,
									  const char* userAgent,
									  const long timeout);

	void* __stdcall HTTPGetContentsSync(const char* url,
										const char* cookie,
										const char* userAgent,
										const long timeout,
										int* result);

	/**
	 * �擾����Web�R���e���c��URL�̎擾
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @param buffer URL���󂯎��o�b�t�@
	 * @oaram length �o�b�t�@�̒���
	 * @return �������݂ɕK�v�ȃo�b�t�@���A�������͏������񂾒���
	 * @note �܂� buffer��NULL���w�肵�Ăяo���A�K�v�ȕ����񒷂��擾���A
	 * �o�b�t�@���m�ۂ�����Ɋm�ۂ����o�b�t�@��buffer�Ɏw�肵�ČĂяo�����Ƃ�
	 * �擾�ł���B
	 */
	int __stdcall HTTPGetURL(void* httpContext,
							 char* buffer,
							 const int length);

	/**
	 * �擾����Web�R���e���c�̍X�V�����̎擾
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @param buffer �X�V�������󂯎��o�b�t�@
	 * @oaram length �o�b�t�@�̒���
	 * @return �������݂ɕK�v�ȃo�b�t�@���A�������͏������񂾒���
	 * @note �܂� buffer��NULL���w�肵�Ăяo���A�K�v�ȕ����񒷂��擾���A
	 * �o�b�t�@���m�ۂ�����Ɋm�ۂ����o�b�t�@��buffer�Ɏw�肵�ČĂяo�����Ƃ�
	 * �擾�ł���B
	 */
	long __stdcall HTTPGetLastModified(void* HttpContext,
									   char* buffer, const int length);

	/**
	 * �擾����Web�R���e���c�̃��X�|���X�R�[�h
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @return HTTP response code
	 */
	long __stdcall HTTPGetResponseCode(void* HttpContext);

	/**
	 * �擾����Web�R���e���c��CRC32�̎擾
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @return CRC32�l
	 */
	long __stdcall HTTPGetCRC32(void* HttpContext);

	/**
	 * �������CRC32�̎擾
	 * @param buffer CRC32�l���v�Z���镶����
	 * @return CRC32�l
	 */
	long __stdcall HTTPGetCRC32FromString(const char* buffer);

	/**
	 * �擾����Web�R���e���c��Filter�K�p��CRC32�̎擾
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @param FilterManagerContext �t�B���^�}�l�[�W���R���e�L�X�g
	 * @return CRC32�l
	 */
	long __stdcall HTTPGetFilteredCRC32(void* httpContext,
										void* filterManagerContext);

	/**
	 * �擾����Web�R���e���c���w��t�@�C���֕ۑ�����B
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @param filename �ۑ�����t�@�C����
	 * @return ������1�A���s��0��������
	 */
	long __stdcall HTTPContentsSave(void* HttpContext, const char* filename);

	/**
	 * �擾����Web�R���e���c�̒������擾����
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @return Web�R���e���c�̒���
	 */
	long __stdcall HTTPGetContentsLength(void* HttpContext);

	/**
	 * �擾����Web�R���e���c���擾����
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @param buffer �R���e���c���󂯎��o�b�t�@
	 * @oaram length �o�b�t�@�̒���
	 * @return Web�R���e���c�̒���
	 */
	long __stdcall HTTPGetResource(void* HttpContext,
								   char* buffer,
								   const int length);

	/**
	 * �擾����Web�R���e���c���J������
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 */
	void __stdcall HTTPClose(void* HttpContext);

	/**
	 * ���K�\���I�u�W�F�N�g���쐬����
	 * @param pattern ���K�\���p�^�[��
	 * @return ���K�\���I�u�W�F�N�g�R���e�L�X�g�BNULL�̏ꍇ�Apattern���s��
	 */
	void* __stdcall RegexCompile(const char* pattern);

	/**
	 * ���K�\���}�b�`���s��
	 * @param regexContext ���K�\���R���e�L�X�g�BRegexCompile()�̖߂�l
	 * @param HttpContext HTTP�擾�R���e�L�X�g�BHTTPGetContents()�̖߂�l 
	 * @param buffer �}�b�`���ʂ��󂯎��o�b�t�@ 
	 * @param length �󂯎��o�b�t�@�̒���
	 * @param ignoreCase �}�b�`���ɑ啶���������𖳎�����ꍇ�A��[��
	 */
	long __stdcall RegexMatcher(void* regexContext, void* thereadHandle,
								char* buffer, const int length,
								const int ignoreCase);

	/**
	 * ���K�\���}�b�`���s��
	 * @param regexContext ���K�\���R���e�L�X�g�BRegexCompile()�̖߂�l
	 * @param targetString �����Ώۂ̕�����
	 * @param buffer �}�b�`���ʂ��󂯎��o�b�t�@ 
	 * @param length �󂯎��o�b�t�@�̒���
	 * @param ignoreCase �}�b�`���ɑ啶���������𖳎�����ꍇ�A��[��
	 */
	long __stdcall RegexMatchFromString(void* regexContext,
										const char* targetString,
										char* buffer,
										const int length,
										const int ignoreCase);

	/**
	 * ���K�\���}�b�`����}�b�`���������o��
	 * @param regexContext ���K�\���R���e�L�X�g�BRegexCompile()�̖߂�l
	 * @param sourceString ���K�\���}�b�`�Ŏg���������Ώۂ̕�����
	 * @param groupNumber ���K�\���p�^�[���ŗ^�����O���[�v'(', ')'�̃y�A�ԍ��B
	 * �S�̃}�b�`��0�ԁA�p�^�[���̍�������Ή�����J�����ʂ��Ƃ�1, 2, 3,�E�E�E
	 * �Ƃ����悤�ɐU����B
	 * @param buffer �}�b�`���ʂ��󂯎��o�b�t�@ 
	 * @param length �󂯎��o�b�t�@�̒���
	 * @return �K�����������̒����BgroupNumber���͈͊O�̏ꍇ�A-1�B
	 */
	long __stdcall RegexMatchedString(void* regexContext,
									  const char* sourceString,
									  const int groupNumber,
									  char* buffer,
									  const int length);

	/**
	 * ���K�\���R���e�L�X�g�̔j��
	 * @param regexContext ���K�\���R���e�L�X�g�BRegexCompile()�̖߂�l
	 */
	void __stdcall RegexTerminate(void* regexContext);


	/**
	 * �t�B���^�}�l�[�W���̍쐬
	 * @return �t�B���^�}�l�[�W���R���e�L�X�g�̃n���h��
	 */
	void* __stdcall FilterManagerCreate();

	/**
	 * �K���t�B���^�̎擾
	 * @param managerContext �t�B���^�}�l�[�W���R���e�L�X�g�̃n���h���B
	 * @param url �t�B���^��K�p����URL
	 * @return �t�B���^�n���h���B
	 */
	void* __stdcall FilterGetFilters(void* managerContext, const char* url);

	/**
	 * �K���t�B���^�̔j��
	 * @param filterHandle FiltergetFilters()�Ŏ擾�����t�B���^�n���h��
	 */
	void __stdcall FilterRemoveFilters(void* filterHandle);

	/**
	 * �t�B���^�̓K�p
	 * @param filterHandle FiltergetFilters()�Ŏ擾�����t�B���^�n���h��
	 * @param contents �R���e���c������BHTTPGetResource()�Ŏ擾�������B
	 * @return �t�B���^��K�p������̃T�C�Y�B
	 * @note �ϊ���̕������contents�ɏ㏑������܂��B
	 */
	long __stdcall FilterApply(void* filterHandle, char* contents);

	/**
	 * �t�B���^�}�l�[�W���̔j��
	 * @param managerContext �t�B���^�}�l�[�W���R���e�L�X�g�̃n���h���B
	 * FilterManagerCreate()�̖߂�l
	 */
	void __stdcall FilterManagerTerminate(void* managerContext);

	
	/*
	 * DLL�����R���e�L�X�g�̍쐬
	 * @return DLL�����R���e�L�X�g
	 */
	void* __stdcall WWWInit();

	/**
	 * DLL�����R���e�L�X�g�̔j��
	 * @param contextHandle DLL�����R���e�L�X�g�BWWWInit()�̖߂�l
	 */
	void __stdcall WWWTerminate(void* contextHandle);


#ifdef __cplusplus
}
#endif 

#endif
