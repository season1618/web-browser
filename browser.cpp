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

// HTTP communication
void HttpRequest(HINTERNET *hInternet_p, HINTERNET *hConnect_p, HINTERNET *hRequest_p, char* url_string){
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
    *hInternet_p = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    *hConnect_p = InternetConnect(*hInternet_p, url_components.lpszHostName, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    *hRequest_p = HttpOpenRequest(*hConnect_p, "GET", url_components.lpszUrlPath, "HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, 0);
    //HINTERNET hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    //HINTERNET hConnect = InternetConnect(hInternet, url_components.lpszHostName, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    //HINTERNET hRequest = HttpOpenRequest(hConnect, "GET", url_components.lpszUrlPath, "HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, 0);
    char header[10];//"Content-Type: application/x-www-form-urlencoded; charset=utf-8";
    char optional[10];//"a=1234&b=5678";
    //BOOL r = HttpSendRequest(hRequest, header, strlen(header), (LPVOID)data, strlen(data));
    if(!HttpSendRequest(*hRequest_p, NULL, 0, NULL, 0)){
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
    /*HttpQueryInfo(
        hRequest,
        HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
        &query_info_buf, &query_info_buf_length, 0
    );*/
    //printf("%d\n",query_info_buf);
    //HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_TEXT, &query_info_buf, &query_info_buf_length, 0);
    //printf("%s\n",query_info_buf);
    //return hRequest;
}


// HTML parser
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
                if(data[0] == '\"' && data.back() == '\"') data = data.substr(1, data.size()-2);
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
    DWORD ReadLength = 1;
    int count = 3000;
    while(true){
        InternetReadFile(hRequest, html_buf, sizeof(html_buf), &ReadLength);
        if(ReadLength == 0) break;
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
    //InternetCloseHandle(hRequest);
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


// HTML Rendering Engine
struct character{
    bool draw_flag;
    int size;
    int font;
    int color;
    character(bool draw_flag){
        this->draw_flag = draw_flag;
    }
};
void HTML_Rendering(HWND hWnd, HDC hdc, int parent_id, character pro){
    for(int child_id:element_child[parent_id]){
        tag elm = elements[child_id];
        string s = elm.tag_name;
        if(s == "text"){
            if(pro.draw_flag){
                char str[elm.info[0].size()];
                for(int i = 0; i < elm.info[0].size(); i++){
                    str[i] = elm.info[0][i];
                }
                TextOut(hdc, 0, 20, str, strlen(str));
            }
        }

        else if(s == "title"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }
        else if(s == "h1"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }
        else if(s == "h2"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }
        else if(s == "h3"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }
        else if(s == "h4"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }
        else if(s == "h5"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }
        else if(s == "h6"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }

        else if(s == "a"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }
        else if(s == "br"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }
        else if(s == "link"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }
        else if(s == "p"){
            HTML_Rendering(hWnd, hdc, child_id, character(true));
        }
        
        else{
            HTML_Rendering(hWnd, hdc, child_id, character(false));
        }
    }
}


// window processing
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch (uMsg){
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            if(HIWORD(wParam) == BN_CLICKED){
                // take url
                HWND hEdt = FindWindowEx(hWnd, 0, "EDIT", NULL);
                LPTSTR url = (LPTSTR)calloc((GetWindowTextLength(hEdt) + 1), sizeof(TCHAR));
                GetWindowText(hEdt, url, (GetWindowTextLength(hEdt) + 1));
                //printf("%s\n",url);

                // communication
                HINTERNET hInternet, hConnect, hRequest;
                HttpRequest(&hInternet, &hConnect, &hRequest, url);
                // https://ja.wikipedia.org/wiki/Uniform_Resource_Locator
                HTML_parser(hRequest, 0);
                InternetCloseHandle(hInternet);
                //HTML_parser_test(0, 0);
                HDC hdc = GetDC(hWnd);
                HTML_Rendering(hWnd, hdc, 0, character(true));
                ReleaseDC(hWnd, hdc);
            }
            return 0;
        /*case WM_PAINT:
            TCHAR str[] = TEXT("hello world");
            //HDC hdc = BeginPaint(hWnd, &ps);
            HDC hdc = GetDC(hWnd);
            TextOut(hdc, 100, 100, str, lstrlen(str));
            //EndPaint(hWnd, &ps);
            return 0;*/
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    // arrange components
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL , IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL , IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "test";

	if (!RegisterClass(&wc)) return 0;

	HWND hWnd = CreateWindow(
        "test", "Title",
        WS_OVERLAPPEDWINDOW,
        100, 100, 800, 600, NULL, NULL,
        hInstance, NULL
	);
    HWND hEdt = CreateWindowEx(
        0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		0, 0, 400, 20,
        hWnd, (HMENU)101, hInstance, NULL
    );
    HWND hBtn = CreateWindowEx(
        0, "BUTTON", "run",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		400, 0, 70, 20,
		hWnd, NULL, hInstance, NULL
    );

	if (hWnd == NULL) return 0;

    ShowWindow(hWnd, nCmdShow);

    TCHAR str[] = TEXT("hello world");
    HDC hdc = GetDC(hWnd);
    TextOut(hdc, 100, 100, str, lstrlen(str));
    ReleaseDC(hWnd, hdc);

    //UpdateWindow(hWnd);
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}