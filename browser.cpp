#include<cstdio>
#include<iostream>
#include<cstdbool>
#include<vector>
#include<deque>
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
}


// parser
struct tag{
    string name;
    vector<string> info;
    tag(){}
    tag(string name){
        this->name = name;
    }
    tag(string name, vector<string> info){
        this->name = name;
        this->info = info;
    }
};

vector<tag> elements(1);
vector<vector<int>> element_tree(1);

tag tag_parser(HINTERNET hRequest){
    char tag_buf[1] = {0};
    DWORD ReadLength;
    string name = ""; bool tag_flag = true;
    vector<string> info;
    string data = "";
    while(InternetReadFile(hRequest, tag_buf, sizeof(tag_buf), &ReadLength)){
        if(tag_buf[0] == '>'){
            info.push_back(data);
            break;
        }
        if(tag_flag){
            if(tag_buf[0] != ' ') name += tag_buf[0];
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
    /*cout<<name;
    for(string data:info) cout<<" "<<data;
    //cout<<endl;*/
    return tag(name, info);
}
void css_parser(HINTERNET hRequest){
    char html_buf[1] = {0};
    DWORD ReadLength = 1;
    deque<char> dc(8, '0'); string s = "</style>";
    while(true){
        InternetReadFile(hRequest, html_buf, sizeof(html_buf), &ReadLength);
        if(ReadLength == 0) break;
        
        dc.pop_front();
        dc.push_back(html_buf[0]);
        bool flag = true;
        for(int i = 0; i < s.size(); i++){
            if(dc[i] != s[i]) flag = false;
        }
        if(flag) break;
    }
}
void javascript_parser(HINTERNET hRequest){
    char html_buf[1] = {0};
    DWORD ReadLength = 1;
    deque<char> dc(9, '0'); string s = "</script>";
    while(true){
        InternetReadFile(hRequest, html_buf, sizeof(html_buf), &ReadLength);
        if(ReadLength == 0) break;

        dc.pop_front();
        dc.push_back(html_buf[0]);
        bool flag = true;
        for(int i = 0; i < s.size(); i++){
            if(dc[i] != s[i]) flag = false;
        }
        if(flag) break;
    }
}
void HTML_parser(HINTERNET hRequest, int parent_id){
    char html_buf[1] = {0};
    DWORD ReadLength = 1;
    int count = 1000;
    while(true){
        InternetReadFile(hRequest, html_buf, sizeof(html_buf), &ReadLength);
        /*char utf8[1000] = {0};
        DWORD ReadLength = 1000;
        InternetReadFile(hRequest, utf8, sizeof(utf8), &ReadLength);
        WCHAR utf16[1000] = {};
        MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, sizeof(utf16) / sizeof(TCHAR));
        char sjis[1000] = {};
        WideCharToMultiByte(932, 0, utf16, -1, sjis, sizeof(sjis), NULL, NULL);
        printf(TEXT("%s\n"), sjis);
        for(int i = 0; i < 1000; i++){
            printf("%x ",utf8[i]);
        }cout<<endl;
        for(int i = 0; i < 1000; i++){
            printf("%x ",utf16[i]);
        }cout<<endl;
        for(int i = 0; i < 1000; i++){
            printf("%x ",sjis[i]);
        }cout<<endl;*/
        if(ReadLength == 0) break;
        //LPWSTR pszWideChar = (LPWSTR)malloc(1025 * sizeof(WCHAR));
        //MultiByteToWideChar(CP_UTF8, 0, response_body_buf, -1, pszWideChar, 1025);
        if(html_buf[0] == '<'){
            tag t = tag_parser(hRequest);
            if(t.name == "style"){
                css_parser(hRequest);
            }
            else if(t.name == "script"){
                javascript_parser(hRequest);
            }
            else if(t.name[0] != '/'){
                int child_id = elements.size();
                elements.push_back(tag());
                elements[child_id] = t;
                element_tree.push_back(vector<int>());
                element_tree[parent_id].push_back(child_id);
                if(t.name == "!DOCTYPE") continue;
                else if(t.name == "meta"){
                    continue;
                }else if(t.name == "link"){
                    continue;
                }
                else HTML_parser(hRequest, child_id);
            }else return;
        }else{
            if(elements.back().name == "text"){
                elements.back().info[0] += html_buf[0];
            }else{
                int child_id = elements.size();
                elements.push_back(tag());
                elements[child_id] = tag("text", vector<string>({string({html_buf[0]})}));
                element_tree.push_back(vector<int>());
                element_tree[parent_id].push_back(child_id);
            }
        }
        if(!count--) break;
    };
    //InternetCloseHandle(hRequest);
}
void HTML_parser_test(int parent_id, int depth){
    for(int child_id:element_tree[parent_id]){
        for(int i = 0; i < depth; i++) cout<<"    ";
        cout<<elements[child_id].name;
        //for(string data:elements[child_id].info) cout<<" "<<data;
        cout<<endl;
        HTML_parser_test(child_id, depth + 1);
    }
}


// HTML Rendering Engine
struct character{
    int height = 0;
    int width = 0;
    string font;
    bool bold = false;
    COLORREF color = RGB(0, 0, 0);
    int indent = 0;
    bool indention = false;
    character(){}
};
struct position{
    int height, width;
    int window_height, window_width;
    int scrollbar;
    position(int width, int height){
        this->height = height;
        this->width = width;
    }
};
void HTML_Rendering(HWND hWnd, HDC hdc, int parent_id, character pro, position *pos_p){
    tag parent_tag = elements[parent_id];
    string s = parent_tag.name;

    if(s == "text"){
        HFONT hFont = CreateFont(
            pro.height, 0, // size
            0, 0,
            (pro.bold ? FW_BOLD : FW_NORMAL), // bold
            FALSE, // italic
            FALSE, FALSE,
            SHIFTJIS_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial"
        );
        SelectObject(hdc, hFont);
        SetTextColor(hdc, pro.color);
        int text_length = parent_tag.info[0].size();
        char text_utf8[text_length + 1] = {};
        WCHAR text_utf16[text_length + 1] = {}; char text_sjis[text_length + 1] = {};
        for(int i = 0; i < text_length; i++){
            text_utf8[i] = parent_tag.info[0][i];
        }
        MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, text_utf16, sizeof(text_utf16) / sizeof(TCHAR));
        WideCharToMultiByte(932, 0, text_utf16, -1, text_sjis, sizeof(text_sjis), NULL, NULL);
        //printf(TEXT("%s\n"), text_sjis);
        for(int i = 0; i < text_length;){
            int code = 0;
            int line_length = (i == 0 ? pro.indent : 0);
            for(int j = 0; i + j < text_length; j++){
                (code *= 256) += text_sjis[i+j] % 256;
                if(text_sjis[i+j] < 0x100 || text_sjis[i+j+1] < 0x100){
                    line_length += pro.width;
                    if(line_length + pro.width > pos_p->window_width || i + j == text_length - 1){
                        if(pos_p->scrollbar <= pos_p->height){
                            TextOut(hdc, (i == 0 ? pro.indent : 0), pos_p->height - pos_p->scrollbar, text_sjis + i, j + 1);
                        }
                        pos_p->height += pro.height;
                        i += j + 1;
                        break;
                    }
                    code = 0;
                }
            }
        }
        return;
        /*cout<<s; printf(" %s\n",text_sjis);
        for(int i = 0; i < child_tag.info[0].size(); i++){
            printf("%c %x ",text_sjis[i], (long)text_sjis[i]);
        }cout<<endl;*/
        //TextOut(hdc, 0, 20, text, 100);
        //cout<<text<<endl;
        //cout<<pos_p->height<<" "<<pos_p->width<<endl;
        /*for(int i = 0; i < text.size(); i++){
            if(pos_p->width + pro.size > window_width){
                pos_p->height += pro.size;
                pos_p->width = 0;
            }
            char ch[1]; ch[0] = text[i];
            TextOut(hdc, pos_p->width, pos_p->height, ch, 1);
            //SelectObject(hdc, hFont);
            pos_p->width += pro.size;
        }
        if(pro.indention){
            pos_p->height += pro.size;
            pos_p->width = 0;
        }*/
    }

    else if(s == "title"){
        string window_title_str = elements[element_tree[parent_id][0]].info[0];
        char window_title[window_title_str.size() + 1] = {};
        for(int i = 0; i < window_title_str.size(); i++){
            window_title[i] = window_title_str[i];
        }
        SetWindowText(hWnd, window_title);
        return;
    }

    else if(s == "h1"){
        pro.height = 48;
        pro.width = 24;
        pro.indention = true;
    }
    else if(s == "h2"){
        pro.height = 36;
        pro.width = 18;
        pro.indention = true;
    }
    else if(s == "h3"){
        pro.height = 24;
        pro.width = 12;
        pro.indention = true;
    }
    else if(s == "h4"){
        pro.height = 16;
        pro.width = 8;
        pro.indention = true;
    }
    else if(s == "h5"){
        pro.height = 16;
        pro.width = 8;
        pro.indention = true;
    }
    else if(s == "h6"){
        pro.height = 16;
        pro.width = 8;
        pro.indention = true;
    }


    else if(s == "a"){
        pro.height = 16;
        pro.width = 8;
        pro.color = RGB(0, 0, 0xFF);
    }
    else if(s == "b"){
        pro.height = 16;
        pro.width = 8;
        pro.bold = true;
    }
    else if(s == "p"){
        pro.height = 16;
        pro.width = 8;
        pro.indent = 4;
    }
    else if(s == "li"){
        pro.height = 16;
        pro.width = 8;
        pro.indent = 4;
        TextOut(hdc, 0, pos_p->height, "  ・", 7);
        //elements[element_tree[parent_id][0]].info[0].insert(0, "  ・");
    }
    for(int child_id:element_tree[parent_id]){    
        HTML_Rendering(hWnd, hdc, child_id, pro, pos_p);
    }
}


// window processing
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    static SCROLLINFO scr;
    switch (uMsg){
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_CREATE:
            scr.cbSize = sizeof(SCROLLINFO);
            scr.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
            scr.nMin = 0; scr.nMax = 600;
            scr.nPage = 1;

            SetScrollInfo(hWnd , SB_VERT , &scr , TRUE);
            return 0;
        case WM_VSCROLL:
            switch(LOWORD(wParam)){
                case SB_TOP:
                    scr.nPos = scr.nMin;
                    break;
                case SB_BOTTOM:
                    scr.nPos = scr.nMax;
                    break;
                case SB_LINEUP:
                    if(scr.nPos > 0) scr.nPos--;
                    break;
                case SB_LINEDOWN:
                    if(scr.nPos < scr.nMax - 1) scr.nPos++;
                    break;
                case SB_PAGEUP:
                    scr.nPos -= scr.nPage;
                    break;
                case SB_PAGEDOWN:
                    scr.nPos += scr.nPage;
                    break;
                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    scr.nPos = HIWORD(wParam);
                    break;
            }
            SetScrollInfo(hWnd , SB_VERT , &scr , TRUE);

            InvalidateRect(hWnd , NULL , TRUE);
            UpdateWindow(hWnd);
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
                character pro;
                position pos(0, 20);
                HTML_Rendering(hWnd, hdc, 0, pro, &pos);
                ReleaseDC(hWnd, hdc);
            }
            return 0;
        case WM_PAINT:
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            character pro;
            position pos(0, 20);
            RECT rect;
            GetWindowRect(hWnd, &rect);
            pos.window_height = rect.bottom - rect.top;
            pos.window_width = rect.right - rect.left;
            pos.scrollbar = scr.nPos;
            HTML_Rendering(hWnd, hdc, 0, pro, &pos);
            EndPaint(hWnd, &ps);

            scr.nMax = pos.height;
            SetScrollInfo(hWnd , SB_VERT , &scr, TRUE);
            return 0;
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
        WS_OVERLAPPEDWINDOW | WS_VSCROLL,
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

	if(hWnd == NULL) return 0;

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