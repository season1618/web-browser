#include<cstdio>
#include<iostream>
#include<cstdbool>
#include<vector>
#include<cwchar>
#include<string>
using namespace std;
#include<windows.h>
#include<wininet.h>

//#pragma comment (lib, "wininet.lib")
HINTERNET HttpRequest(char* url_string){
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
    return hRequest;
}

struct tag{
    string tag_name;
    vector<string> info;
    vector<int> tag_id;
    tag(){}
    tag(string tag_name){
        this->tag_name = tag_name;
    }
    tag(string tag_name, vector<string> info){
        this->tag_name = tag_name;
        this->info = info;
    }
};
vector<tag> elements(1);
vector<vector<int>> element_child(1);

tag tag_parser(HINTERNET hRequest){
    char tag_buf[1] = {0};
    DWORD ReadLength;
    string tag_name = ""; bool tag_flag = true;
    vector<string> info;
    string key = "", value = ""; bool key_flag = true, value_flag = false;
    while(InternetReadFile(hRequest, tag_buf, sizeof(tag_buf), &ReadLength)){
        if(tag_flag){
            if(tag_buf[0] != ' ') tag_name += tag_buf[0];
            else tag_flag = false;
        }else{
            if(key_flag){
                if(tag_buf[0] != '=') key += tag_buf[0];
                else{ key_flag = false; value_flag = true; }
            }else if(value_flag){
                if(tag_buf[0] != ' ' && tag_buf[0] != '>') value += tag_buf[0];
                else{
                    value_flag = false;
                    value = value.substr(1, value.size() - 2);
                    info.push_back(key);
                    info.push_back(value);
                    key = ""; value = "";
                }
            }
        }
        if(tag_buf[0] == '>') break;
    }
    return tag(tag_name, info);
}
void HTML_parser(HINTERNET hRequest, int parent_id){
    char html_buf[1] = {0};
    DWORD ReadLength;
    int count = 1000;
    while(InternetReadFile(hRequest, html_buf, sizeof(html_buf), &ReadLength)){
        //LPWSTR pszWideChar = (LPWSTR)malloc(1025 * sizeof(WCHAR));
        //MultiByteToWideChar(CP_UTF8, 0, response_body_buf, -1, pszWideChar, 1025);
        printf("%s", html_buf);
        if(html_buf[0] == '<'){
            int child_id = elements.size();
            elements.push_back(tag());
            elements[child_id] = tag_parser(hRequest);
            element_child.push_back(vector<int>());
            element_child[parent_id].push_back(child_id);
            if(elements[child_id].tag_name[0] != '/'){
                HTML_parser(hRequest, child_id);
            }else return;
        }else{
            if(elements.back().tag_name == "text"){
                elements.back().info.push_back(html_buf);
            }else{
                int child_id = elements.size();
                elements.push_back(tag());
                elements[child_id] = tag("text", vector<string>({string({html_buf[0]})}));
                element_child.push_back(vector<int>());
                element_child[parent_id].push_back(child_id);
            }
        }
        if(count--) break;
    };
    InternetCloseHandle(hRequest);
}
int main(void){
    //char url[] = "https://www.google.com";
    char url[] = "https://ja.wikipedia.org/wiki/Uniform_Resource_Locator";
    HINTERNET hRequest = HttpRequest(url);
    HTML_parser(hRequest, 0);
}