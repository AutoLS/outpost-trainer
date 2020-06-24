#include <AE/AutoEngine.h>
#include "UI.cpp"
#include "process.cpp"

enum game_items
{
	ITEM_WOOD,
	ITEM_STONE,
	ITEM_ENERGY,
	ITEM_MAX,
};

int wmain()
{
	InitLib(LIB_SDL_FULL);
	render Graphics = InitGraphics("Outpost Trainer v1.0", {400, 400}, 
								   SDL_WINDOW_INPUT_FOCUS   |
								   SDL_WINDOW_MOUSE_FOCUS);
	
	rect32 WindowRect = {V2(), Graphics.WinDim};

	game_input Input = {};
	Input.KeyState = SDL_GetKeyboardState(0);
	
	SDL_Event Event;
	uint64 LastCount = SDL_GetPerformanceCounter();
	real32 t = 0;
	
	char* ProcessName = "Outpost.exe";
	
	TTF_Font* ButtonFont = TTF_OpenFont("ast/LibreBaskerville-Regular.ttf", 12);
	TTF_Font* LabelsFont = TTF_OpenFont("ast/LibreBaskerville-Regular.ttf", 20);
	int LabelH = TTF_FontHeight(LabelsFont);
	
	label StatusLabelFailed = CreateLabel(V2(), "Cannot find outpost.exe...", 
										  LabelsFont, 
									     {255, 0, 0, 255}, Graphics.Renderer);
	label StatusLabelSucceed = CreateLabel(V2(), 
										   "Opened outpost.exe for writing...", 
										   LabelsFont, 
										   {0, 255, 0, 255}, Graphics.Renderer);
	
	char* ItemNames[3] = 
	{
		"Wood",
		"Stone",
		"Energy"
	};
	
	uint32 AddressOffset[] = 
	{
		0x494C70,
		0x494440,
		0x492D20
	};
	
	uint32 WoodOffset[] = {0x330, 0x5A0, 0x18, 0x30};
	uint32 StoneOffset[] = {0x90, 0x8C0, 0x18, 0x48};
	uint32 EnergyOffset[] = {0xA70, 0x410, 0x110, 0x1C};
	
	image_data* UITextures = LoadUITextures(Graphics.Renderer);
	button Button[ITEM_MAX] = {};
	label ItemLabels[ITEM_MAX] = {};
	for(int i = 0; i < 3; ++i)
	{
		v2 Pos = V2(100, 50 + (i*70.0f));
		Button[i] = CreateButton(Pos, V2(50, 50), 
								 UITextures[0], UITextures[1],
								 Graphics.Renderer, ButtonFont, "+100");
		ItemLabels[i] = CreateLabel(V2(10, Pos.y+25-LabelH*0.5f), ItemNames[i], 
									LabelsFont, {255, 255, 255, 255},
									Graphics.Renderer);
	}
	
	bool Running = true;
	while(Running)
	{
		uint32 ItemsValue[ITEM_MAX] = {};
		image_data ItemsValueText[ITEM_MAX] = {}; 
		
		process_info ProcessInfo = 
		GetProcessInfo(ProcessName, "mono-2.0-bdwgc.dll");
		
		ResetKeyState(&Input.Mouse, BUTTON_MAX);
		ResetKeyState(&Input.Keyboard, KEY_MAX);
		
		while(SDL_PollEvent(&Event) != 0)
		{
			if(HandleEvents(&Event, &Input, &Graphics))
			{
				Running = false;
			}
		}
		
		SDL_SetRenderDrawColor(Graphics.Renderer, 50, 50, 50, 255);
		SDL_RenderClear(Graphics.Renderer);
		
		while(t > 0)
		{
			real32 dt = Min(t, Graphics.Display.TargetSecPerFrame);
			//Logic
			t -= dt;
		}
		
		HandleButtons(Button, &Input, ArraySize(Button));
		
		if(ProcessInfo.Status == PROCESS_OPENED)
		{
			uintptr_t DynPtrBaseAddr[ITEM_MAX] =
			{
				ProcessInfo.ModuleBaseAddr + AddressOffset[ITEM_WOOD],
				ProcessInfo.ModuleBaseAddr + AddressOffset[ITEM_STONE],
				ProcessInfo.ModuleBaseAddr + AddressOffset[ITEM_ENERGY]
			};
			
			uintptr_t WoodPtr = 
			FindDAMAddr(ProcessInfo.Process, DynPtrBaseAddr[0], WoodOffset, 	
						ArraySize(WoodOffset));
			
			uintptr_t StonePtr = 
			FindDAMAddr(ProcessInfo.Process, DynPtrBaseAddr[1], StoneOffset, 	
						ArraySize(StoneOffset));
						
			uintptr_t EnergyPtr = 
			FindDAMAddr(ProcessInfo.Process, DynPtrBaseAddr[2], EnergyOffset, 	
						ArraySize(EnergyOffset));
						
			//printf("Dynamic base address: 0x%llx\n", DynPtrBaseAddr);
			ReadWriteMemory(&ProcessInfo, &Button[ITEM_WOOD], 
							&Input, KEY_F1,
							WoodPtr, &ItemsValue[ITEM_WOOD], 100);
			ReadWriteMemory(&ProcessInfo, &Button[ITEM_STONE], 
							&Input, KEY_F2,
							StonePtr, &ItemsValue[ITEM_STONE], 100);
			ReadWriteMemory(&ProcessInfo, &Button[ITEM_ENERGY], 
							&Input, KEY_F3,
							EnergyPtr, &ItemsValue[ITEM_ENERGY], 100);
		}
		
		for(int i = 0; i < ITEM_MAX; ++i)
		{
			char Buffer[100] = {};
			sprintf(Buffer, "%d", ItemsValue[i]);
			LoadText(Graphics.Renderer, LabelsFont, &ItemsValueText[i], 
					 Buffer, {255, 0, 0, 255});
					 
			rect32 TextRect = ItemLabels[i].Rect;
			TextRect.Pos += V2(150, 0);
			TextRect.Dim = V2(ItemsValueText[i].Dim);
			
			ItemsValueText[i].Rect = SetRectRounded(TextRect);
		}
		
		CloseHandle(ProcessInfo.Process);
		
		real32 FrameCompleteTime = 
		Win32GetSecondElapsed(LastCount, SDL_GetPerformanceCounter());
		
		if(FrameCompleteTime < Graphics.Display.TargetSecPerFrame)
		{
			int SleepTime = (int)(Graphics.Display.TargetSecPerFrame - 
								  FrameCompleteTime * 1000) - 1;
			if(SleepTime > 0)
			{
				Sleep(SleepTime);
			}
			//assert(Win32GetSecondElapsed(LastCount, SDL_GetPerformanceCounter()) < TargetSecPerFrame);
			while(Win32GetSecondElapsed(LastCount, SDL_GetPerformanceCounter()) <
				  Graphics.Display.TargetSecPerFrame);
		}
		
		uint64 EndCount = SDL_GetPerformanceCounter();
		
		//Render
		ProcessInfo.Status ? RenderLabel(&StatusLabelSucceed, 
										 Graphics.Renderer) 
						   : RenderLabel(&StatusLabelFailed, 
										 Graphics.Renderer);
		RenderButtons(Button, Graphics.Renderer, ArraySize(Button));
		RenderLabels(ItemLabels, Graphics.Renderer, ArraySize(ItemLabels));
		for(int i = 0; i < ITEM_MAX; ++i)
		{
			SDL_RenderCopy(Graphics.Renderer, ItemsValueText[i].Texture,
						   0, &ItemsValueText[i].Rect);
			SDL_DestroyTexture(ItemsValueText[i].Texture);			
		}
		SDL_RenderPresent(Graphics.Renderer);
		
		real32 ElapsedTime = Win32GetSecondElapsed(LastCount, EndCount);
		t = ElapsedTime;

		uint64 CounterElapsed = EndCount - LastCount;
		real32 FPS = (real32)PerformanceFrequency / (real32)CounterElapsed;
		
		//PrintFPS(FPS);
		
		LastCount = EndCount;
	}
	
	return 0;
}