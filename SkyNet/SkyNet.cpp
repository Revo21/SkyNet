// SkyNet.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

using namespace std;
// ~New
using namespace AwesomiumCppApi;
// ~End New

#define SIZEOF_JMP_REL  5
#define SIZEOF_MOVEAX_JMPEAX 7

typedef HRESULT (WINAPI* oEndScene) (LPDIRECT3DDEVICE9 pDevice);

static oEndScene originalEndScene;
static bool ui_hooked = false ,ui_active;

#define __defaultfontargs 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0

// ~New
#define SLEEP_MS 50

int windowWidth = 0;
int windowHeight = 0;
Awesomium *htmlGUI;

IDirect3DTexture9*			texture = NULL;
LPD3DXSPRITE				sprite = NULL;
IDirect3DStateBlock9*		pStateBlock = NULL;
D3DXVECTOR3					pos;
D3DCOLOR					fillcolor = D3DCOLOR_ARGB( 255, 255, 255, 255 );
// ~End New

// ~New Helpermethods
DWORD WINAPI MyInput(LPVOID)
{
	while (TRUE)
	{
		if(GetKeyPressed(VK_F2))
		{
			ui_active = !ui_active;	
		}		
		Sleep(100);			
	}
	return 0;
}

void DrawMouse(LPDIRECT3DDEVICE9 pDevice, LPD3DXSPRITE sprite)
{
	//TODO insert new cursor texture HERE ;)	
	if(TexCursor == NULL)D3DXCreateTextureFromFileInMemory(pDevice, &_Cursor, sizeof(_Cursor), &TexCursor);
	GetCursorPos(&PosCursor);
	
	if (ui_active)
	{
		sprite->Begin(D3DXSPRITE_ALPHABLEND);
		sprite->Draw(TexCursor, NULL,NULL,&D3DXVECTOR3( PosCursor.x, PosCursor.y, 0.0f ),D3DCOLOR_ARGB( 255, 255, 255, 255 ));
		sprite->End();				
	}
}

HRESULT initializeRequiredResources(LPDIRECT3DDEVICE9 pDevice, LPDIRECT3DTEXTURE9 *texture, int *width, int *height, LPD3DXSPRITE *sprite, D3DXVECTOR3 *position)
{
		IDirect3DSurface9* surface;
        pDevice->GetBackBuffer(0,0, D3DBACKBUFFER_TYPE::D3DBACKBUFFER_TYPE_FORCE_DWORD, &surface);

		// resolution of backbuffer = game resolution
		D3DSURFACE_DESC desc;
		surface->GetDesc(&desc);
		
		*width = desc.Width;
		*height = desc.Height;	

		add_log(convertInt(*width));
		add_log(convertInt(*height));	
		
		if(!SUCCEEDED(D3DXCreateTexture(pDevice, *width, *height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, texture))) //pDevice->CreateTexture(*width, *height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, texture,NULL)))// D3DPOOL_DEFAULT
		{
			add_log("D3DXCreateTexture() failed to create texture");
			return E_FAIL;
		}	
				
		add_log("D3D9Texture created");	
		
		// Position of texture on screen
		if (SUCCEEDED(D3DXCreateSprite(pDevice,sprite)))
		{
			position->x=0.0f;
			position->y=0.0f;
			position->z=0.0f;
		}		
		else
		{
			add_log("D3DXCreateSprite() failed to create sprite");
			return E_FAIL;
		}		

		add_log("required resources successfully initialized");	
		return S_OK;
}
// ~End New

static HRESULT WINAPI OurEndScene(DXDevice DirectX)
{
	if(ui_hooked)
	{		
		// ~New (actual rendering code)
		htmlGUI->updateWebcore();	

		// TODO: Apply performance teaks HERE ! ---- !
		// there could be a bug with the dirtyRect recognition... have to look into it again

		if(htmlGUI->webviewIsDirty())
		{				
			const RECT pRect = htmlGUI->getDirtyRect();
			RECT dirtyRect = htmlGUI->getDirtyRect();

			htmlGUI->renderWebview();

			D3DLOCKED_RECT lockedRect;
			texture->LockRect(0, &lockedRect, NULL, 0);

			BYTE *bytePointerLR=(BYTE*)lockedRect.pBits;		
			const BYTE *bytePointerRB= htmlGUI->getBufferFromRenderbuffer();

			for (DWORD y=dirtyRect.top;y<dirtyRect.bottom;y++) 
			{
				for (DWORD x=dirtyRect.left;x<dirtyRect.right;x++)
				{
					memcpy( &bytePointerLR[lockedRect.Pitch * y + 4 * x], &bytePointerRB[windowWidth * 4 * y + 4 * x], 4 );			
				}
			}

			//memcpy( lockedRect.pBits, htmlGUI->getBufferFromRenderbuffer(), htmlGUI->getRenderbufferSize());		

			texture->UnlockRect(0);		
		}	
				
		sprite->Begin( D3DXSPRITE_ALPHABLEND );	
		sprite->Draw(texture,NULL,NULL,&pos,fillcolor);
		sprite->End();	
		
		if(ui_active)
		{		
			// TODO: disable ingame GUI
			Game::SetInChargen(true, true, true);
			ExecuteConsoleCommand("DisablePlayerControls 1 1 1 1 1 1 1 1 0",Game::GetPlayer());	

			// injecting mouse movement 						
			GetCursorPos(&PosCursor);	
			htmlGUI->injectMouseMove(PosCursor.x, PosCursor.y);	

			DrawMouse(CurrentDevice, sprite);
		}
		else
		{	
			// TODO: enable ingame GUI
			Game::SetInChargen(false, false, true);
			ExecuteConsoleCommand("epc",Game::GetPlayer());			
		}
		// ~End New
	}
	else
	{		
		CurrentDevice = DirectX;
		DirectX->GetDisplayMode(0, &DisplayMode);

		// ~New (Initializing)
		if(SUCCEEDED(initializeRequiredResources(DirectX, &texture, &windowWidth, &windowHeight, &sprite, &pos)))
		{					
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE) MyInput, NULL, NULL, NULL);

			// TODO: reduce Sleep() usage
			htmlGUI = new Awesomium();
			Sleep(200);
			htmlGUI->initializeTransparentWebview(windowWidth, windowHeight);
			Sleep(200);
			htmlGUI->loadFile("/html/index.html");

			Sleep(200);

			// TODO: callback objects for every GUI window
			htmlGUI->setupCallback("object_index", "callback_index");	
			htmlGUI->setupCallback("chat", "message");

			// call JavaScript-Functions with arguments like this:
			std::string testArr[4] = {"Whiterun","123","123","123"};
			htmlGUI->callJavascriptFunction("changeLocation", htmlGUI->stringArrToJsArr(testArr, (sizeof(testArr)/sizeof(testArr[0]))));

			ui_hooked = true;
		}
		else
		{			
			add_log("initializeRequiredResources() failed");
			ui_hooked = false;
		}		
		// ~End New
	}

	return originalEndScene(DirectX);
}

static DWORD DetourCreateType1(DWORD SrcVA, DWORD DstVA, DWORD Size)
{
	DWORD DetourVA, dwProtect, i;
	if (SrcVA && DstVA && Size >= SIZEOF_JMP_REL)
	{
		DetourVA = (DWORD) VirtualAlloc(NULL, Size + SIZEOF_JMP_REL, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (DetourVA && VirtualProtect((VOID*)SrcVA, Size, PAGE_EXECUTE_READWRITE, &dwProtect))
		{
			for (i=0; i < Size; i++) 
				*(BYTE*)(DetourVA + i) = *(BYTE*)(SrcVA + i);

			*(BYTE*)(DetourVA + Size + 0) = 0xE9;
			*(DWORD*)(DetourVA + Size + 1) = (SrcVA - DetourVA - SIZEOF_JMP_REL);

			*(BYTE*)(SrcVA + 0) = 0xE9;
			*(DWORD*)(SrcVA + 1) = (DstVA - SrcVA - SIZEOF_JMP_REL);

			VirtualProtect((VOID*)SrcVA, Size, dwProtect, &dwProtect);

			VirtualProtect((VOID*)DetourVA, Size + 
				SIZEOF_JMP_REL, PAGE_EXECUTE_READ, &dwProtect);

			return DetourVA;
		}
	}
	return (0);
}

static DWORD DetourCreateType2(DWORD SrcVA, DWORD DstVA, DWORD Size)
{
	DWORD DetourVA, dwProtect, i;
	if (SrcVA && DstVA && Size >= SIZEOF_MOVEAX_JMPEAX)
	{
		DetourVA = (DWORD) VirtualAlloc(
			NULL, Size + SIZEOF_MOVEAX_JMPEAX, 
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (DetourVA && VirtualProtect((VOID*)SrcVA, Size, PAGE_EXECUTE_READWRITE, &dwProtect))
		{
			for (i=0; i < Size; i++) 
				*(BYTE*)(DetourVA + i) = *(BYTE*)(SrcVA + i);

			*(BYTE*)(DetourVA + Size + 0) = 0xB8;
			*(DWORD*)(DetourVA + Size + 1) = (SrcVA + Size);
			*(WORD*)(DetourVA + Size + 5) = 0xE0FF;

			*(BYTE*)(SrcVA + 0) = 0xB8;
			*(DWORD*)(SrcVA + 1) = (DstVA);
			*(WORD*)(SrcVA + 5) = 0xE0FF;

			VirtualProtect((VOID*)SrcVA, Size, dwProtect, &dwProtect);

			VirtualProtect((VOID*)DetourVA, Size + SIZEOF_MOVEAX_JMPEAX, PAGE_EXECUTE_READ, &dwProtect);

			return DetourVA;
		}
	}
	return 0;
}

static DWORD* FindDevice(DWORD Base) 
{
	for(long i= 0,n = 0; i < 0x128000; i++)
	{
		if(*(BYTE *)(Base+i+0x00)==0xC7)n++;
		if(*(BYTE *)(Base+i+0x01)==0x06)n++;
		if(*(BYTE *)(Base+i+0x06)==0x89)n++;
		if(*(BYTE *)(Base+i+0x07)==0x86)n++;
		if(*(BYTE *)(Base+i+0x0C)==0x89)n++;
		if(*(BYTE *)(Base+i+0x0D)==0x86)n++;

		if(n == 6) 
			return (DWORD*)(Base + i + 2);n = 0;
	}
	return(0);
}

static bool bCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for(;*szMask;++szMask,++pData,++bMask)
		if(*szMask=='x' && *pData!=*bMask) 
			return false;

	return (*szMask) == NULL;
}

static DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char* szMask)
{ 
	for(DWORD i=0; i < dwLen; i++)
		if(bCompare((BYTE*)(dwAddress+i),bMask,szMask))
			return (DWORD)(dwAddress+i);
	return 0;
}

static DWORD GetAddressPtr(int Index, HMODULE DirectX) 
{
	DWORD* VTableStart = 0;  
	DWORD dwDevicePointer = FindPattern((DWORD)DirectX, 0x1280000, (PBYTE)"\xC7\x06\x00\x00\x00\x00\x89\x86\x00\x00\x00\x00\x89\x86", "xx????xx????xx");
	memcpy(&VTableStart, (void*)(dwDevicePointer+2), 4);
	return VTableStart[Index];
} 
static HINSTANCE ResolveHInstance()
{
	D3DDEVICE_CREATION_PARAMETERS params;
	CurrentDevice->GetCreationParameters(&params);
	return (HINSTANCE)GetWindowLong(window = params.hFocusWindow, GWL_HINSTANCE);
}

DWORD WINAPI HookProcess()
{
	char RealD3D9[MAX_PATH];
	GetSystemDirectory(RealD3D9,MAX_PATH);
	strcat(RealD3D9,"\\d3d9.dll");

	HMODULE directX;
	do
	{
		directX = GetModuleHandle(RealD3D9);
		Sleep(100);
	}
	while(!directX);
	DWORD dwEndScene = GetAddressPtr(42, directX);
	do
	{
		Sleep(100);
		if(!originalEndScene && memcmp((void*)dwEndScene, (void*)"\x8B\xFF", 2) == 0)
			originalEndScene = (oEndScene)DetourCreateType2(dwEndScene, (DWORD)OurEndScene, SIZEOF_MOVEAX_JMPEAX);
	}
	while(!ui_hooked || !originalEndScene);
	DirectInput8Create(ResolveHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, NULL);
	dinput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	keyboard->SetDataFormat(&c_dfDIKeyboard);
	keyboard->SetCooperativeLevel(window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	keyboard->Acquire();

	// ~New
	dinput->CreateDevice(GUID_SysMouse, &mouse, NULL);
	mouse->SetDataFormat(&c_dfDIMouse);
	mouse->SetCooperativeLevel(window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE );
	mouse->Acquire();
	// ~End New

	SkyNet_Ready = true;
	return 0;
}

// -----------------------------------------  Methods New

WNDPROC oldWndProc;
LRESULT CALLBACK OurMessageLoop(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void HookWindowMessageLoop(LPCSTR windowName)
{	
	HWND hWnd;	

	do {
		hWnd = GetForegroundWindow();
	} while( hWnd != FindWindow( NULL, windowName ) );
	
	oldWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG)OurMessageLoop);
}
LRESULT CALLBACK OurMessageLoop(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_KEYDOWN:			
		case WM_KEYUP:
		case WM_CHAR:
			if( ui_hooked && ui_active )
			{		
				// TODO: AwesomiumCppApi call -> injectKeyboardEventWin
				htmlGUI->focusWebview(true);
				awe_webview_inject_keyboard_event_win(htmlGUI->getWebView(),message,wParam, lParam);		
				return 1;
			}
			break;			
		default:
			 break;		
	}

	return CallWindowProc(oldWndProc, hWnd, message, wParam, lParam);
}


void main()
{
	PrintNote("DXInjection started");
	clear_log();
		
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE) HookProcess, NULL, NULL, NULL);
	HookWindowMessageLoop("Skyrim");	

	/*while(!RC_D3D)
		Wait(100);*/	

	DIMOUSESTATE currentMouseState, lastMouseState; 
	while(!SkyNet_Ready) Wait(50);
	bool set = false;	

	while(true)
	{
		if(set)
		{
			mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&currentMouseState);

			if( currentMouseState.lZ != 0 && lastMouseState.lZ != currentMouseState.lZ )
			{
				lastMouseState.lZ = currentMouseState.lZ;
				if(ui_hooked && ui_active)
				{
					// TODO: AwesomiumCppApi call -> injectMouseWheel
					htmlGUI->focusWebview(true);
					awe_webview_inject_mouse_wheel(htmlGUI->getWebView(), currentMouseState.lZ, 0);					
				}
			} 
			if( currentMouseState.rgbButtons[0]&0x80 && (lastMouseState.rgbButtons[0]&0x80) != (currentMouseState.rgbButtons[0]&0x80) )
			{
				lastMouseState.rgbButtons[0] = currentMouseState.rgbButtons[0]; 
				if(ui_hooked && ui_active)
				{					
					htmlGUI->injectMouseDown();					
				}
			}
			else if((lastMouseState.rgbButtons[0]&0x80) != (currentMouseState.rgbButtons[0]&0x80))
			{
				lastMouseState.rgbButtons[0] = currentMouseState.rgbButtons[0]; 
				if(ui_hooked && ui_active)
				{					
					htmlGUI->injectMouseUp();							
				}
			}						
		}
		else
		{
			set = true;			
			mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&lastMouseState);
		}
		Wait(0);
	}	
}
