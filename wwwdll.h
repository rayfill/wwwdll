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
	__stdcall void* ThreadCreate();
	
	/**
	 * �X���b�h�R���e�L�X�g�̎��s
	 * @param context ���s����HTTP�R���e�L�X�g
	 * @param hWnd �R�[���o�b�N����Window�̃n���h��
	 * @param message �R�[���o�b�N����E�B���h�E���b�Z�[�WID
	 * @note �R�[���o�b�N�����E�B���h�E���b�Z�[�W��wParam�Ɋ֐��̎��s����
	 * (1�������A0�����s)�AlParam�ɃX���b�h�R���e�L�X�g��������B
	 */
	__stdcall void ThreadStart(void* context,
							   void* httpContext,
							   HWND hWnd,
							   const unsigned int message);

	/**
	 *  �X���b�h�̏I���҂�
	 * @param threadContext �X���b�h�R���e�L�X�g�̃n���h��
	 * @return ���݂ł�0�Œ�
	 * (�]�T������������s���������̖߂�l��Ԃ��悤�ɕύX���܂�)
	 */
	__stdcall long ThreadJoin(void* threadContext);

	/**
	 * �X���b�h�R���e�L�X�g�̔j��
	 * @param threadContext �X���b�h�R���e�L�X�g�̃n���h��
	 */
	__stdcall void ThreadClose(void* threadContext);


	/**
	 * Web�R���e���c�擾�R���e�L�X�g�̍쐬
	 * @param url �R���e���c�̂���URL
	 * @param cookie �R���e���c�擾���ɕt�^����Cookie�B�Ȃ��ꍇ��NULL���w�肷��
	 * @param userAgent �R���e���c�擾���Ɏg��UserAgent�B�f�t�H���g�̏ꍇ��NULL
	 * @param timeout ���ʐM�^�C���A�E�g���ԁB
	 * @return ��Ƃ��s��HTTP�R���e�L�X�g�n���h���B
	 */
	__stdcall void* HTTPCreateContext(const char* url,
									  const char* cookie,
									  const char* userAgent,
									  const long timeout);

	__stdcall void* HTTPGetContentsSync(const char* url,
										const char* cookie,
										const char* userAgent,
										const long timeout,
										int* result);

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
	__stdcall long HTTPGetLastModified(void* HttpContext,
									   char* buffer, const int length);

	/**
	 * �擾����Web�R���e���c�̃��X�|���X�R�[�h
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @return HTTP response code
	 */
	__stdcall long HTTPGetResponseCode(void* HttpContext);

	/**
	 * �擾����Web�R���e���c��CRC32�̎擾
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @return CRC32�l
	 */
	__stdcall long HTTPGetCRC32(void* HttpContext);

	/**
	 * �������CRC32�̎擾
	 * @param buffer CRC32�l���v�Z���镶����
	 * @return CRC32�l
	 */
	__stdcall long HTTPGetCRC32FromString(const char* buffer);

	/**
	 * �擾����Web�R���e���c��Filter�K�p��CRC32�̎擾
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @param FilterManagerContext �t�B���^�}�l�[�W���R���e�L�X�g
	 * @return CRC32�l
	 */
	__stdcall long HTTPGetFilteredCRC32(void* httpContext,
										void* filterManagerContext);

	/**
	 * �擾����Web�R���e���c���w��t�@�C���֕ۑ�����B
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @param filename �ۑ�����t�@�C����
	 * @return ������1�A���s��0��������
	 */
	__stdcall long HTTPContentsSave(void* HttpContext, const char* filename);

	/**
	 * �擾����Web�R���e���c�̒������擾����
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @return Web�R���e���c�̒���
	 */
	__stdcall long HTTPGetContentsLength(void* HttpContext);

	/**
	 * �擾����Web�R���e���c���擾����
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 * @param buffer �R���e���c���󂯎��o�b�t�@
	 * @oaram length �o�b�t�@�̒���
	 * @return Web�R���e���c�̒���
	 */
	__stdcall long HTTPGetResource(void* HttpContext,
								   char* buffer,
								   const int length);

	/**
	 * �擾����Web�R���e���c���J������
	 * @param HttpContext HTTPGetContents()�̖߂�l
	 */
	__stdcall void HTTPClose(void* HttpContext);

	/**
	 * ���K�\���I�u�W�F�N�g���쐬����
	 * @param pattern ���K�\���p�^�[��
	 * @return ���K�\���I�u�W�F�N�g�R���e�L�X�g�BNULL�̏ꍇ�Apattern���s��
	 */
	__stdcall void* RegexCompile(const char* pattern);

	/**
	 * ���K�\���}�b�`���s��
	 * @param regexContext ���K�\���R���e�L�X�g�BRegexCompile()�̖߂�l
	 * @param HttpContext HTTP�擾�R���e�L�X�g�BHTTPGetContents()�̖߂�l 
	 * @param buffer �}�b�`���ʂ��󂯎��o�b�t�@ 
	 * @param length �󂯎��o�b�t�@�̒���
	 * @param ignoreCase �}�b�`���ɑ啶���������𖳎�����ꍇ�A��[��
	 */
	__stdcall long RegexMatcher(void* regexContext, void* thereadHandle,
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
	__stdcall long RegexMatchFromString(void* regexContext,
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
	__stdcall long RegexMatchedString(void* regexContext,
									  const char* sourceString,
									  const int groupNumber,
									  char* buffer,
									  const int length);


	/**
	 * ���K�\���}�b�`����}�b�`������u����������������
	 * @param regexContext ���K�\���R���e�L�X�g�BRegexCompile()�̖߂�l
	 * @param sourceString ���K�\���}�b�`�Ŏg���������Ώۂ̕�����
	 * @param groupNumber ���K�\���p�^�[���ŗ^�����O���[�v'(', ')'�̃y�A�ԍ��B
	 * �S�̃}�b�`��0�ԁA�p�^�[���̍�������Ή�����J�����ʂ��Ƃ�1, 2, 3,�E�E�E
	 * �Ƃ����悤�ɐU����B
	 * @param replaceString �u���Ɏg��������
	 * @param buffer �u�����ʂ��󂯎��o�b�t�@ 
	 * @param length �󂯎��o�b�t�@�̒���
	 * @return �u����̕�����̒����BgroupNumber���͈͊O�̏ꍇ�A-1�B
	 */
	__stdcall long RegexMatchedReplace(void* regexContext,
									   const char* sourceString,
									   const int groupNumber,
									   const char* replaceString,
									   char* buffer,
									   const int length);

	/**
	 * ���K�\���R���e�L�X�g�̔j��
	 * @param regexContext ���K�\���R���e�L�X�g�BRegexCompile()�̖߂�l
	 */
	__stdcall void RegexTerminate(void* regexContext);


	/**
	 * �t�B���^�}�l�[�W���̍쐬
	 * @return �t�B���^�}�l�[�W���R���e�L�X�g�̃n���h��
	 */
	__stdcall void* FilterManagerCreate();

	/**
	 * �K���t�B���^�̎擾
	 * @param managerContext �t�B���^�}�l�[�W���R���e�L�X�g�̃n���h���B
	 * @param url �t�B���^��K�p����URL
	 * @return �t�B���^�n���h���B
	 */
	__stdcall void* FilterGetFilters(void* managerContext, const char* url);

	/**
	 * �K���t�B���^�̔j��
	 * @param filterHandle FiltergetFilters()�Ŏ擾�����t�B���^�n���h��
	 */
	__stdcall void FilterRemoveFilters(void* filterHandle);

	/**
	 * �t�B���^�̓K�p
	 * @param filterHandle FiltergetFilters()�Ŏ擾�����t�B���^�n���h��
	 * @param contents �R���e���c������BHTTPGetResource()�Ŏ擾�������B
	 * @return �t�B���^��K�p������̃T�C�Y�B
	 * @note �ϊ���̕������contents�ɏ㏑������܂��B
	 */
	__stdcall long FilterApply(void* filterHandle, char* contents);

	/**
	 * �t�B���^�}�l�[�W���̔j��
	 * @param managerContext �t�B���^�}�l�[�W���R���e�L�X�g�̃n���h���B
	 * FilterManagerCreate()�̖߂�l
	 */
	__stdcall void FilterManagerTerminate(void* managerContext);

	
	/*
	 * DLL�����R���e�L�X�g�̍쐬
	 * @return DLL�����R���e�L�X�g
	 */
	__stdcall void* WWWInit();

	/**
	 * DLL�����R���e�L�X�g�̔j��
	 * @param contextHandle DLL�����R���e�L�X�g�BWWWInit()�̖߂�l
	 */
	__stdcall void WWWTerminate(void* contextHandle);


#ifdef __cplusplus
}
#endif 

#endif
