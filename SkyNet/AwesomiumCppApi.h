#include <iostream>
#include <memory>
#include <Awesomium/awesomium_capi.h>

namespace AwesomiumCppApi
{

	class Awesomium
	{
	private:		
		awe_webview* webView;	
		const awe_renderbuffer* rBuffer;
		int testVar;
		

	public:	
		Awesomium();
		void initializeTransparentWebview(int width, int height);
		void loadFile(char* path);

		void updateWebcore();
		bool webviewIsDirty();
		void renderWebview();

		RECT getDirtyRect();

		unsigned char* getBuffer();
		const unsigned char* getBufferFromRenderbuffer();
		int getRenderbufferSize();

		void focusWebview(bool mode);

		void injectKeyDown(byte keyCode);
		void injectKeyUp(byte keyCode);
		void injectKeyboardInput(byte keyCode);

		void injectMouseDown();
		void injectMouseUp();
		void injectMouseClick();
		void injectMouseMove(int x, int y);

		awe_jsarray* stringArrToJsArr(std::string in[], static int size);
		void callJavascriptFunction(char* functionname, awe_jsarray* args);
		
		void setupCallback(char *jsObjectName, char* callbackFunctionName);

		awe_webview* getWebView();
	};

}