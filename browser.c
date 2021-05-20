#include<stdio.h>
#include<stdbool.h>
#include<wchar.h>

#include<windows.h>
#include<wininet.h>

//#pragma comment (lib, "wininet.lib")
void HttpRequest(char* url_string){
    URL_COMPONENTS url_components;
    ZeroMemory(&url_components, sizeof(URL_COMPONENTS));
    url_components.dwStructSize = sizeof(URL_COMPONENTS);
    TCHAR szHostName[4096];
	TCHAR szUrlPath[4096];
	url_components.lpszHostName	= szHostName;
	url_components.lpszUrlPath	= szUrlPath;
	url_components.dwHostNameLength	= 4096;
	url_components.dwUrlPathLength	= 4096;

    if(!InternetCrackUrl(url_string, strlen(url_string), ICU_DECODE, &url_components)){
        printf("failed to analyze url\n");
    }
    /*printf("%s\n",url_components.lpszHostName);
    printf("%d\n",url_components.nPort);
    printf("%s\n",url_components.lpszUrlPath);*/
    HINTERNET hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    HINTERNET hConnect = InternetConnect(hInternet, url_components.lpszHostName, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    HINTERNET hRequest = HttpOpenRequest(hConnect, "GET", url_components.lpszUrlPath, "HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, 0);
    char header[10];//"Content-Type: application/x-www-form-urlencoded; charset=utf-8";
    char optional[10];//"a=1234&b=5678";
    //BOOL r = HttpSendRequest(hRequest, header, strlen(header), (LPVOID)data, strlen(data));
    if(!HttpSendRequest(hRequest, NULL, 0, NULL, 0)){
        printf("failed to send a request\n");
        printf("Error Code : %d\n",GetLastError());
        printf(url_string);
    }

    DWORD query_info_buf = {0};
    DWORD query_info_buf_length = sizeof(query_info_buf);
    /*HttpQueryInfo(
        hRequest,
        HTTP_QUERY_STATUS_CODE | HTTP_QUERY_STATUS_TEXT | HTTP_QUERY_FLAG_NUMBER | 
        HTTP_QUERY_CONTENT_TYPE | HTTP_QUERY_COOKIE | HTTP_QUERY_DATE,
        &query_info_buf, &query_info_buf_length, NULL
    );*/
    HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &query_info_buf, &query_info_buf_length, 0);
    printf("%d\n",query_info_buf);
    //HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_TEXT, &query_info_buf, &query_info_buf_length, 0);
    //printf("%s\n",query_info_buf);
    char response_body_buf[1024] = {0};
    DWORD ReadLength;
    while(InternetReadFile(hRequest, response_body_buf, sizeof(response_body_buf), &ReadLength)){
        //LPWSTR pszWideChar = (LPWSTR)malloc(1025 * sizeof(WCHAR));
        //MultiByteToWideChar(CP_UTF8, 0, response_body_buf, -1, pszWideChar, 1025);
        //printf("%s", response_body_buf);
        //printf("%s", pszWideChar);
        break;
    };
    InternetCloseHandle(hInternet);
}

void HTML_parser(HINTERNET hRequest){
}
int main(void){
    //char url[] = "https://www.google.com";
    char url[] = "https://ja.wikipedia.org/wiki/Uniform_Resource_Locator";
    HttpRequest(url);
}