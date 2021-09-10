#include<cstdio>
#include<iostream>
#include<cstdbool>
#include<vector>
#include<deque>
#include<set>
#include<cwchar>
#include<string>
using namespace std;
#include<windows.h>
#include<wininet.h>

//#pragma comment (lib, "wininet.lib")

// HTTP communication
void HttpRequest(char* url_string, string& data){
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

    DWORD query_attributes_buf = {0};
    DWORD query_attributes_buf_length = sizeof(query_attributes_buf);
    /*HttpQueryattributes(
        hRequest,
        HTTP_QUERY_STATUS_CODE | HTTP_QUERY_STATUS_TEXT | HTTP_QUERY_FLAG_NUMBER | 
        HTTP_QUERY_CONTENT_TYPE | HTTP_QUERY_COOKIE | HTTP_QUERY_DATE,
        &query_attributes_buf, &query_attributes_buf_length, NULL
    );*/
    /*HttpQueryattributes(
        hRequest,
        HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
        &query_attributes_buf, &query_attributes_buf_length, 0
    );*/
    //printf("%d\n",query_attributes_buf);
    //HttpQueryattributes(hRequest, HTTP_QUERY_STATUS_TEXT, &query_attributes_buf, &query_attributes_buf_length, 0);
    //printf("%s\n",query_attributes_buf);
    DWORD BufferSize = 1024;
    DWORD ReadLength;
    char ReadBuffer[BufferSize];
    while(true){
        for(int i = 0; i < BufferSize; i++) ReadBuffer[i] = '\0';
        if(!InternetReadFile(hRequest, ReadBuffer, BufferSize, &ReadLength)) break;
        //ReadBuffer[BufferSize] = '\0';
        for(char c:ReadBuffer){
            if(c == '\0') break;
            data.push_back(c);
        }
    }
}


// parser
struct Element {
    string name = "";
    vector<string> attributes;
    vector<int> child_element;
    Element(){}
    Element(string name){
        this->name = name;
    }
    Element(string name, vector<string> attributes){
        this->name = name;
        this->attributes = attributes;
    }
};
set<string> empty_element_name = { "!DOCTYPE", "area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "param", "source", "track", "wbr"};
string data = "";
vector<Element> document(1);

Element tag_parser(string& data, int& i){
    bool is_name = true;
    Element elm;
    string attr = "";
    for(; data[i] != '>'; i++){
        char c = data[i];
        if(is_name){
            if(c != ' ') elm.name += c;
            else is_name = false;
        }else{
            if(c != '=' && c != ' ') attr += c;
            else{
                if(attr[0] == '\"' && attr.back() == '\"') attr = attr.substr(1, attr.size()-2);
                elm.attributes.push_back(attr);
                attr = "";
            }
        }
    }
    elm.attributes.push_back(attr);
    /*cout<<name;
    for(string data:attributes) cout<<" "<<data;
    //cout<<endl;*/
    return elm;
}
void css_parser(string& data, int& i){
    deque<char> dc(8, '0'); string s = "</style>";
    for(; i < data.size(); i++){
        char c = data[i];
        dc.pop_front();
        dc.push_back(c);
        bool flag = true;
        for(int j = 0; j < s.size(); j++){
            if(dc[j] != s[j]) flag = false;
        }
        if(flag) break;
    }
}
void js_parser(string& data, int& i){
    deque<char> dc(9, '0'); string s = "</script>";
    for(; i < data.size(); i++){
        char c = data[i];
        dc.pop_front();
        dc.push_back(c);
        bool flag = true;
        for(int j = 0; j < s.size(); j++){
            if(dc[j] != s[j]) flag = false;
        }
        if(flag) break;
    }
}
char read_utf8(string& data, int& i){
    if(data[i] & 128 == 0){
        return data[i];
    }else if(data[i] & (128 + 64 + 32) == 128 + 64){
        i++;
        return '?';
    }else if(data[i] & (128 + 64 + 32 + 16) == 128 + 64 + 32){
        i += 2;
        return '?';
    }else if(data[i] & (128 + 64 + 32 + 16 + 8) == 128 + 64 + 32 + 16){
        i += 3;
        return '?';
    }else{
        return '?';
    }
}
void html_parser(string& data, int& i, int id){
    int count = 1000;
    for(; i < data.size(); i++){
        char c = read_utf8(data, i);
        if(c == '<'){ // element
            Element elm = tag_parser(data, i);
            if(elm.name == "style"){
                css_parser(data, i);
            }
            else if(elm.name == "script"){
                js_parser(data, i);
            }
            else if(elm.name[0] != '/'){ // opening tag
                int child_id = document.size();
                document.push_back(Element());
                document[child_id] = elm;
                document[id].child_element.push_back(child_id);
                if(!empty_element_name.count(elm.name)) html_parser(data, i, child_id);
            }else return;
        }
        else{ // text
            if(document.back().name == "text"){
                document.back().attributes[0] += c;
            }else{
                int child_id = document.size();
                document.push_back(Element());
                document[child_id] = Element("text", vector<string>({string({c})}));
                document[id].child_element.push_back(child_id);
            }
        }
        if(!count--) break;
    }
}
/*void html_parser_test(int parent_id, int depth){
    for(int child_id:document[parent_id].child_element){
        for(int i = 0; i < depth; i++) cout<<"    ";
        cout<<document[child_id].name;
        //for(string data:elements[child_id].attributes) cout<<" "<<data;
        cout<<endl;
        html_parser_test(child_id, depth + 1);
    }
}*/


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
    int y, x;
    int window_height, window_width;
    int scrollbar;
    position(int y, int x){
        this->y = y;
        this->x = x;
    }
};
void render(HWND hWnd, HDC hdc, position& pos, int id = 0, character pro = character()){
    Element elm = document[id];
    string s = elm.name;

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
        int text_length = elm.attributes[0].size();
        char text_utf8[text_length + 1] = {};
        WCHAR text_utf16[text_length + 1] = {}; char text_sjis[text_length + 1] = {};
        for(int i = 0; i < text_length; i++){
            text_utf8[i] = elm.attributes[0][i];
        }
        MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, text_utf16, sizeof(text_utf16) / sizeof(TCHAR));
        WideCharToMultiByte(932, 0, text_utf16, -1, text_sjis, sizeof(text_sjis), NULL, NULL);
        //printf(TEXT("%s\n"), text_sjis);
        RECT rect;
        rect.left = pos.x;
        rect.top = pos.y;
        rect.right = pos.x + pos.window_width;
        rect.bottom = pos.y + pos.window_height;
        DrawText(hdc, text_sjis, sizeof(text_sjis), &rect, DT_WORDBREAK);
        /*for(int i = 0; i < text_length;){
            int code = 0;
            int line_length = (i == 0 ? pro.indent : 0);
            for(int j = 0; i + j < text_length; j++){
                (code *= 256) += text_sjis[i+j] % 256;
                if(text_sjis[i+j] < 0x100 || text_sjis[i+j+1] < 0x100){
                    line_length += pro.width;
                    if(line_length + pro.width > pos.window_width || i + j == text_length - 1){
                        if(pos.scrollbar <= pos.y){
                            TextOut(hdc, (i == 0 ? pro.indent : 0), pos.y - pos.scrollbar, text_sjis + i, j + 1);
                        }
                        pos.y += pro.height;
                        i += j + 1;
                        break;
                    }
                    code = 0;
                }
            }
        }*/
        return;
        /*cout<<s; printf(" %s\n",text_sjis);
        for(int i = 0; i < child_tag.attributes[0].size(); i++){
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
        string window_title_str = document[elm.child_element[0]].attributes[0];
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
        TextOut(hdc, 0, pos.y, "  ・", 7);
        //elements[element_tree[parent_id][0]].attributes[0].insert(0, "  ・");
    }
    for(int child_id:elm.child_element){
        render(hWnd, hdc, pos, child_id, pro);
    }
}


// window processing
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    static SCROLLINFO scr;
    switch(uMsg){
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
                HttpRequest(url, data);
                // https://ja.wikipedia.org/wiki/Uniform_Resource_Locator
                
                int i = 0;
                html_parser(data, i, 0);
                //html_parser_test(0, 0);
                HDC hdc = GetDC(hWnd);
                position pos(20, 0);
                render(hWnd, hdc, pos);
                ReleaseDC(hWnd, hdc);
            }
            return 0;
        case WM_PAINT:
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rect;
            GetWindowRect(hWnd, &rect);
            position pos(20, 0);
            pos.window_height = rect.bottom - rect.top;
            pos.window_width = rect.right - rect.left;
            pos.scrollbar = scr.nPos;
            render(hWnd, hdc, pos);
            EndPaint(hWnd, &ps);

            scr.nMax = pos.y;
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

    TCHAR str[] = TEXT("窓");
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