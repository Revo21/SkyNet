#include "AwesomiumCppApi.h"

void clear_logA()
{
	HANDLE filehandle;	
	filehandle = CreateFile("DX_Injection_Log.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);	
	CloseHandle(filehandle);
}
void add_logA(char* string)
{
	HANDLE filehandle;
	DWORD dwReadBytes;
	char buffer[2048];
	filehandle = CreateFile("DX_Injection_Log.txt", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
	SetFilePointer(filehandle, 0, 0, FILE_END);
	sprintf_s(buffer, 1024, "Log: %s\r\n", string);
	WriteFile(filehandle, buffer, strlen(buffer), &dwReadBytes, 0);
	CloseHandle(filehandle);
}
char* convertIntA(int number)
{	
	char buffer [30];
	itoa (number,buffer,10);
	return buffer;
}

namespace AwesomiumCppApi
{
	
#define BUFFER_LEN_CALLBACKS 100


	Awesomium::Awesomium()
	{		
		awe_webcore_initialize_default();							
	}

	void Awesomium::initializeTransparentWebview(int width, int height)
	{
		webView = awe_webcore_create_webview(width, height, false);
		awe_webview_set_transparent(webView, true);
		add_logA("Awesomium: Webview created");		
	}

	void Awesomium::loadFile(char* path)
	{
		awe_string* file_str = awe_string_create_from_ascii(path, strlen(path));
		awe_webview_load_file(webView,file_str, awe_string_empty());
		awe_string_destroy(file_str);
		while(awe_webview_is_loading_page(webView))
		{			
			Sleep(200);			
			awe_webcore_update();
			add_logA("Awesomium: awe_webcore_update() executed");
		}
		add_logA("Awesomium: File loaded");
	}

	void Awesomium::updateWebcore()
	{							
		awe_webcore_update();		
	}
	
	bool Awesomium::webviewIsDirty()
	{				
		return awe_webview_is_dirty(webView);
	}

	void Awesomium::renderWebview()
	{
		rBuffer = awe_webview_render(webView);		
	}

	RECT Awesomium::getDirtyRect()
	{
		awe_rect temp = awe_webview_get_dirty_bounds(webView);
		RECT a = {temp.x,temp.y,temp.x+temp.width,temp.y+temp.height};		
		return a;
	}

	unsigned char* Awesomium::getBuffer()
	{
		unsigned char *Buffer;
		Buffer = new unsigned char[getRenderbufferSize()];			
		awe_renderbuffer_copy_to(rBuffer, Buffer, awe_renderbuffer_get_width(rBuffer)*4, 4, true, false);		
		return Buffer;
	}

	const unsigned char* Awesomium::getBufferFromRenderbuffer()
	{				
		return awe_renderbuffer_get_buffer(rBuffer);
	}

	int Awesomium::getRenderbufferSize()
	{		
		return awe_renderbuffer_get_rowspan(rBuffer) * awe_renderbuffer_get_height(rBuffer);
	}

	void Awesomium::focusWebview(bool mode)
	{
		(mode? awe_webview_focus : awe_webview_unfocus) (webView);		
	}

	awe_webview* Awesomium::getWebView(){return webView;}

	void Awesomium::injectKeyDown(byte keyCode)
	{
		awe_webview_focus( webView );
		awe_webview_inject_keyboard_event_win( webView, WM_KEYDOWN, (WPARAM)keyCode, 0x000e0001);
	}
	
	void Awesomium::injectKeyUp(byte keyCode)
	{
		awe_webview_focus( webView );
		awe_webview_inject_keyboard_event_win( webView, WM_KEYUP, (WPARAM)keyCode, 0x000e0001);
	}

	void Awesomium::injectKeyboardInput(byte keyCode)
	{
		awe_webview_focus( webView );

		switch(keyCode)
		{
			case 0x08: // Backspace				
				awe_webview_inject_keyboard_event_win( webView, WM_KEYDOWN, (WPARAM)VK_BACK, 0x000e0001);
				awe_webview_inject_keyboard_event_win( webView, WM_KEYUP, (WPARAM)VK_BACK, 0x000e0001);
				break;
			default: 
				awe_webview_inject_keyboard_event_win( webView, WM_CHAR, (WPARAM)keyCode, 0x000e0001);
		}
	}

	void Awesomium::injectMouseDown()
	{
		awe_webview_focus( webView );
		awe_webview_inject_mouse_down(webView, AWE_MB_LEFT);			
	}

	void Awesomium::injectMouseUp()
	{
		awe_webview_focus( webView );
		awe_webview_inject_mouse_up(webView, AWE_MB_LEFT);
	}

	void Awesomium::injectMouseClick()
	{
		awe_webview_focus( webView );		
		awe_webview_inject_mouse_down(webView, AWE_MB_LEFT);		
		awe_webview_inject_mouse_up(webView, AWE_MB_LEFT);				
	}

	void Awesomium::injectMouseMove(int x, int y)
	{
		awe_webview_focus( webView );		
		awe_webview_inject_mouse_move(webView, x, y);		
	}

	awe_jsarray* Awesomium::stringArrToJsArr(std::string in[], static int size)
	{				
		// TODO: use size to initialize pArguments
		const awe_jsvalue *pArguments[10];
		awe_string* temp;

		//std::string *input = new std::string[size]; 
		for (int i = 0; i < size && i < 10; i++) 
		{
			temp = awe_string_create_from_ascii(in[i].c_str(),strlen(in[i].c_str()));
			pArguments[i]=awe_jsvalue_create_string_value(temp);
		}

		awe_string_destroy(temp);	
		
		return awe_jsarray_create(pArguments, size);
	}

	void Awesomium::callJavascriptFunction(char* functionname, awe_jsarray* args)
	{
		add_logA("Awesomium: callJavascriptFunction() called");
		awe_string* functionname_str = awe_string_create_from_ascii(functionname, strlen(functionname));
		awe_webview_call_javascript_function(webView, awe_string_empty(), functionname_str, args, awe_string_empty());
		awe_string_destroy(functionname_str);		
	}



	void callback_UI(awe_webview *view, const awe_string  *object_name, const awe_string *callback_name, const awe_jsarray *args)
	{		
		char buff_object_name[BUFFER_LEN_CALLBACKS] = {0};
		char buff_callback_name[BUFFER_LEN_CALLBACKS] = {0};
		char buff_args[BUFFER_LEN_CALLBACKS] = {0};		
		const awe_jsvalue *temp = awe_jsarray_get_element(args,0);

		// Get the strings
		awe_string_to_utf8(object_name, buff_object_name, BUFFER_LEN_CALLBACKS);
		awe_string_to_utf8(callback_name, buff_callback_name, BUFFER_LEN_CALLBACKS);
		awe_string_to_utf8(awe_jsvalue_to_string(temp), buff_args, BUFFER_LEN_CALLBACKS);

		std::string s_obj_name(buff_object_name);
		std::string s_call_name(buff_callback_name);
		std::string s_args(buff_args);

		// Callback example: Javascript triggered this function -> chat.message("Test message") -> the output is the following:
		add_logA("Callback executed");
		add_logA((char*)s_obj_name.c_str());      // s_obj_name == name of the callback object ( chat )
		add_logA((char*)s_call_name.c_str());     // s_call_name == the function used by the callback object ( message )		
		add_logA((char*)s_args.c_str());		  // s_args == arguments given to the callback method ( Test message ) 
			
		// do whatever you like with the arguments ( s_args ) but check which GUI window triggered the callback by comparing s_obj_name with the object names defined while initializing the GUI
	}

	void Awesomium::setupCallback(char *jsObjectName, char* callbackFunctionName)
	{
		awe_string* awes_name = awe_string_create_from_ascii(jsObjectName, strlen(jsObjectName));	
		awe_string* awes_callback = awe_string_create_from_ascii(callbackFunctionName,strlen(callbackFunctionName));

		awe_webview_create_object(webView, awes_name);			
		awe_webview_set_object_callback(webView, awes_name, awes_callback);
		
		awe_string_destroy(awes_name);
		awe_string_destroy(awes_callback);
		
		awe_webview_set_callback_js_callback(webView, callback_UI);
	}
}

