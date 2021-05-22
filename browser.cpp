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
    string data = "";
    while(InternetReadFile(hRequest, tag_buf, sizeof(tag_buf), &ReadLength)){
        if(tag_buf[0] == '>'){
            info.push_back(data);
            break;
        }
        if(tag_flag){
            if(tag_buf[0] != ' ') tag_name += tag_buf[0];
            else tag_flag = false;
        }else{
            if(tag_buf[0] != '=' && tag_buf[0] != ' ') data += tag_buf[0];
            else{
                info.push_back(data);
                data = "";
            }
        }
    }
    /*cout<<tag_name;
    for(string data:info) cout<<" "<<data;
    cout<<endl;*/
    return tag(tag_name, info);
}
void HTML_parser(HINTERNET hRequest, int parent_id){
    char html_buf[1] = {0};
    DWORD ReadLength;
    int count = 1000;
    while(InternetReadFile(hRequest, html_buf, sizeof(html_buf), &ReadLength)){
        //LPWSTR pszWideChar = (LPWSTR)malloc(1025 * sizeof(WCHAR));
        //MultiByteToWideChar(CP_UTF8, 0, response_body_buf, -1, pszWideChar, 1025);
        //printf("%s",html_buf);
        if(html_buf[0] == '<'){
            tag t = tag_parser(hRequest);
            if(t.tag_name[0] != '/'){
                int child_id = elements.size();
                elements.push_back(tag());
                elements[child_id] = t;
                element_child.push_back(vector<int>());
                element_child[parent_id].push_back(child_id);
                if(t.tag_name == "!DOCTYPE") continue;
                else if(t.tag_name == "meta") continue;
                else HTML_parser(hRequest, child_id);
            }else return;
        }else{
            if(elements.back().tag_name == "text"){
                elements.back().info[0] += html_buf[0];
            }else{
                int child_id = elements.size();
                elements.push_back(tag());
                elements[child_id] = tag("text", vector<string>({string({html_buf[0]})}));
                element_child.push_back(vector<int>());
                element_child[parent_id].push_back(child_id);
            }
        }
        if(!count--) break;
    };
    InternetCloseHandle(hRequest);
}
void HTML_parser_test(int parent_id, int depth){
    for(int child_id:element_child[parent_id]){
        for(int i = 0; i < depth; i++) cout<<"    ";
        cout<<elements[child_id].tag_name;
        for(string data:elements[child_id].info) cout<<" "<<data;
        cout<<endl;
        HTML_parser_test(child_id, depth + 1);
    }
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    /*HWND hwnd;
	WNDCLASS winc;

	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = DefWindowProc;
	winc.cbClsExtra	= winc.cbWndExtra = 0;
	winc.hInstance = hInstance;
	winc.hIcon = LoadIcon(NULL , IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL , IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = TEXT("KITTY");

	if (!RegisterClass(&winc)) return 0;

	hwnd = CreateWindow(
			TEXT("KITTY"), TEXT("Kitty on your lap"),
			WS_OVERLAPPEDWINDOW,
			100, 100, 200, 200, NULL, NULL,
			hInstance, NULL
	);

	if (hwnd == NULL) return 0;

	ShowWindow(hwnd , SW_SHOW);*/
    MessageBox(NULL, "", "", MB_OK);
    return 0;
}
/*int main(void){
    //char url[] = "https://www.google.com";
    char url[] = "https://ja.wikipedia.org/wiki/Uniform_Resource_Locator";
    HINTERNET hRequest = HttpRequest(url);
    HTML_parser(hRequest, 0);
    HTML_parser_test(0, 0);
}*/