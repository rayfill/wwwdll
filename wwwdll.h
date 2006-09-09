#ifndef HTMLMODIFYCHECKER_HPP_
#define HTMLMODIFYCHECKER_HPP_

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * スレッドコンテキストの作成
	 * @return スレッドコンテキストハンドル
	 */
	__stdcall void* ThreadCreate();
	
	/**
	 * スレッドコンテキストの実行
	 * @param context 実行するHTTPコンテキスト
	 * @param hWnd コールバックするWindowのハンドル
	 * @param message コールバックするウィンドウメッセージID
	 * @note コールバックされるウィンドウメッセージのwParamに関数の実行結果
	 * (1が完了、0が失敗)、lParamにスレッドコンテキストがかえる。
	 */
	__stdcall void ThreadStart(void* context,
							   void* httpContext,
							   HWND hWnd,
							   const unsigned int message);

	/**
	 *  スレッドの終了待ち
	 * @param threadContext スレッドコンテキストのハンドル
	 * @return 現在では0固定
	 * (余裕があったら実行した処理の戻り値を返すように変更します)
	 */
	__stdcall long ThreadJoin(void* threadContext);

	/**
	 * スレッドコンテキストの破棄
	 * @param threadContext スレッドコンテキストのハンドル
	 */
	__stdcall void ThreadClose(void* threadContext);


	/**
	 * Webコンテンツ取得コンテキストの作成
	 * @param url コンテンツのあるURL
	 * @param cookie コンテンツ取得時に付与するCookie。ない場合はNULLを指定する
	 * @param userAgent コンテンツ取得時に使うUserAgent。デフォルトの場合はNULL
	 * @param timeout 無通信タイムアウト時間。
	 * @return 作業を行うHTTPコンテキストハンドル。
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
	 * 取得したWebコンテンツの更新日時の取得
	 * @param HttpContext HTTPGetContents()の戻り値
	 * @param buffer 更新日時を受け取るバッファ
	 * @oaram length バッファの長さ
	 * @return 書き込みに必要なバッファ長、もしくは書き込んだ長さ
	 * @note まず bufferにNULLを指定し呼び出し、必要な文字列長を取得し、
	 * バッファを確保した後に確保したバッファをbufferに指定して呼び出すことで
	 * 取得できる。
	 */
	__stdcall long HTTPGetLastModified(void* HttpContext,
									   char* buffer, const int length);

	/**
	 * 取得したWebコンテンツのレスポンスコード
	 * @param HttpContext HTTPGetContents()の戻り値
	 * @return HTTP response code
	 */
	__stdcall long HTTPGetResponseCode(void* HttpContext);

	/**
	 * 取得したWebコンテンツのCRC32の取得
	 * @param HttpContext HTTPGetContents()の戻り値
	 * @return CRC32値
	 */
	__stdcall long HTTPGetCRC32(void* HttpContext);

	/**
	 * 文字列のCRC32の取得
	 * @param buffer CRC32値を計算する文字列
	 * @return CRC32値
	 */
	__stdcall long HTTPGetCRC32FromString(const char* buffer);

	/**
	 * 取得したWebコンテンツのFilter適用後CRC32の取得
	 * @param HttpContext HTTPGetContents()の戻り値
	 * @param FilterManagerContext フィルタマネージャコンテキスト
	 * @return CRC32値
	 */
	__stdcall long HTTPGetFilteredCRC32(void* httpContext,
										void* filterManagerContext);

	/**
	 * 取得したWebコンテンツを指定ファイルへ保存する。
	 * @param HttpContext HTTPGetContents()の戻り値
	 * @param filename 保存するファイル名
	 * @return 成功時1、失敗時0がかえる
	 */
	__stdcall long HTTPContentsSave(void* HttpContext, const char* filename);

	/**
	 * 取得したWebコンテンツの長さを取得する
	 * @param HttpContext HTTPGetContents()の戻り値
	 * @return Webコンテンツの長さ
	 */
	__stdcall long HTTPGetContentsLength(void* HttpContext);

	/**
	 * 取得したWebコンテンツを取得する
	 * @param HttpContext HTTPGetContents()の戻り値
	 * @param buffer コンテンツを受け取るバッファ
	 * @oaram length バッファの長さ
	 * @return Webコンテンツの長さ
	 */
	__stdcall long HTTPGetResource(void* HttpContext,
								   char* buffer,
								   const int length);

	/**
	 * 取得したWebコンテンツを開放する
	 * @param HttpContext HTTPGetContents()の戻り値
	 */
	__stdcall void HTTPClose(void* HttpContext);

	/**
	 * 正規表現オブジェクトを作成する
	 * @param pattern 正規表現パターン
	 * @return 正規表現オブジェクトコンテキスト。NULLの場合、patternが不正
	 */
	__stdcall void* RegexCompile(const char* pattern);

	/**
	 * 正規表現マッチを行う
	 * @param regexContext 正規表現コンテキスト。RegexCompile()の戻り値
	 * @param HttpContext HTTP取得コンテキスト。HTTPGetContents()の戻り値 
	 * @param buffer マッチ結果を受け取るバッファ 
	 * @param length 受け取るバッファの長さ
	 * @param ignoreCase マッチ時に大文字小文字を無視する場合、非ゼロ
	 */
	__stdcall long RegexMatcher(void* regexContext, void* thereadHandle,
								char* buffer, const int length,
								const int ignoreCase);

	/**
	 * 正規表現マッチを行う
	 * @param regexContext 正規表現コンテキスト。RegexCompile()の戻り値
	 * @param targetString 検索対象の文字列
	 * @param buffer マッチ結果を受け取るバッファ 
	 * @param length 受け取るバッファの長さ
	 * @param ignoreCase マッチ時に大文字小文字を無視する場合、非ゼロ
	 */
	__stdcall long RegexMatchFromString(void* regexContext,
										const char* targetString,
										char* buffer,
										const int length,
										const int ignoreCase);

	/**
	 * 正規表現マッチからマッチ部分を取り出す
	 * @param regexContext 正規表現コンテキスト。RegexCompile()の戻り値
	 * @param sourceString 正規表現マッチで使った検索対象の文字列
	 * @param groupNumber 正規表現パターンで与えたグループ'(', ')'のペア番号。
	 * 全体マッチが0番、パターンの左側から対応する開き括弧ごとに1, 2, 3,・・・
	 * というように振られる。
	 * @param buffer マッチ結果を受け取るバッファ 
	 * @param length 受け取るバッファの長さ
	 * @return 適合した文字の長さ。groupNumberが範囲外の場合、-1。
	 */
	__stdcall long RegexMatchedString(void* regexContext,
									  const char* sourceString,
									  const int groupNumber,
									  char* buffer,
									  const int length);


	/**
	 * 正規表現マッチからマッチ部分を置換した文字列を作る
	 * @param regexContext 正規表現コンテキスト。RegexCompile()の戻り値
	 * @param sourceString 正規表現マッチで使った検索対象の文字列
	 * @param groupNumber 正規表現パターンで与えたグループ'(', ')'のペア番号。
	 * 全体マッチが0番、パターンの左側から対応する開き括弧ごとに1, 2, 3,・・・
	 * というように振られる。
	 * @param replaceString 置換に使う文字列
	 * @param buffer 置換結果を受け取るバッファ 
	 * @param length 受け取るバッファの長さ
	 * @return 置換後の文字列の長さ。groupNumberが範囲外の場合、-1。
	 */
	__stdcall long RegexMatchedReplace(void* regexContext,
									   const char* sourceString,
									   const int groupNumber,
									   const char* replaceString,
									   char* buffer,
									   const int length);

	/**
	 * 正規表現コンテキストの破棄
	 * @param regexContext 正規表現コンテキスト。RegexCompile()の戻り値
	 */
	__stdcall void RegexTerminate(void* regexContext);


	/**
	 * フィルタマネージャの作成
	 * @return フィルタマネージャコンテキストのハンドル
	 */
	__stdcall void* FilterManagerCreate();

	/**
	 * 適合フィルタの取得
	 * @param managerContext フィルタマネージャコンテキストのハンドル。
	 * @param url フィルタを適用するURL
	 * @return フィルタハンドル。
	 */
	__stdcall void* FilterGetFilters(void* managerContext, const char* url);

	/**
	 * 適合フィルタの破棄
	 * @param filterHandle FiltergetFilters()で取得したフィルタハンドル
	 */
	__stdcall void FilterRemoveFilters(void* filterHandle);

	/**
	 * フィルタの適用
	 * @param filterHandle FiltergetFilters()で取得したフィルタハンドル
	 * @param contents コンテンツ文字列。HTTPGetResource()で取得した物。
	 * @return フィルタを適用した後のサイズ。
	 * @note 変換後の文字列はcontentsに上書きされます。
	 */
	__stdcall long FilterApply(void* filterHandle, char* contents);

	/**
	 * フィルタマネージャの破棄
	 * @param managerContext フィルタマネージャコンテキストのハンドル。
	 * FilterManagerCreate()の戻り値
	 */
	__stdcall void FilterManagerTerminate(void* managerContext);

	
	/*
	 * DLL処理コンテキストの作成
	 * @return DLL処理コンテキスト
	 */
	__stdcall void* WWWInit();

	/**
	 * DLL処理コンテキストの破棄
	 * @param contextHandle DLL処理コンテキスト。WWWInit()の戻り値
	 */
	__stdcall void WWWTerminate(void* contextHandle);


#ifdef __cplusplus
}
#endif 

#endif
