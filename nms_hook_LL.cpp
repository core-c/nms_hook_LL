#include <windows.h>
#include <stdio.h>


#define COLOR_DEFAULT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_ERROR (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define COLOR_OK (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_TITLE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)

// Timing values can be read from file.
// '#define' or '#undef' to select a different compiled version.
#undef VALUES_FROM_FILE
//
#if defined(VALUES_FROM_FILE)
#include <fstream>
#endif


/*
// reference: sleep_for(), sleep_until(), ...
#include <chrono>
#include <thread>
{
    using namespace std::this_thread; // sleep_for, sleep_until
    using namespace std::chrono; // nanoseconds, system_clock, seconds
    sleep_for(milliseconds(delayMS_WireGlitch));
}
*/


// These are the default values.
#define DELAYMS_WIREGLITCH 0;
#define DELAYMS_CACHEGLITCH 3;

// The variables values can be overwritten from data read from a file
int delay_WireGlitch = DELAYMS_WIREGLITCH;
int delay_CacheGlitch = DELAYMS_CACHEGLITCH;


HHOOK hMouseHook;           // handle to the hooked procedure
DWORD NMSthreadId;          // threadId of NMS
DWORD InstallerThreadId;    // threadId of the installer thread




// print a colored text to the console
void printColored(const char* txt, WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    printf(txt);
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT); // restore
}

int errorOut(int errorCode, const char* txt) {
    printColored(txt, COLOR_ERROR);
    printf("\n\n");
    system("pause");
    return errorCode;
}


int WireGlitch() {
    UINT nSent;
    INPUT inputs[1] = {};
    UINT scanCodeQ = MapVirtualKeyA('Q', MAPVK_VK_TO_VSC);

    // send Q down
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;
    inputs[0].ki.wScan = scanCodeQ;
    nSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (nSent != ARRAYSIZE(inputs)) {
        // error
        return 1;
    }

    if (delay_WireGlitch > 0) Sleep(delay_WireGlitch);

    // send LMB down
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    nSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (nSent != ARRAYSIZE(inputs)) {
        // error
        return 3;
    }

    Sleep(4);

    // send Q up
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    inputs[0].ki.wScan = scanCodeQ;
    nSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (nSent != ARRAYSIZE(inputs)) {
        // error
        return 2;
    }

    Sleep(5);

    // send LMB up
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    nSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (nSent != ARRAYSIZE(inputs)) {
        // error
        return 4;
    }

    return 0;
}


int CacheGlitch() {
    UINT nSent;
    INPUT inputs[1] = {};
    UINT scanCodeQ = MapVirtualKeyA('Q', MAPVK_VK_TO_VSC);
    UINT scanCodeC = MapVirtualKeyA('C', MAPVK_VK_TO_VSC);

    // send C down
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wScan = scanCodeC;
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;
    nSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (nSent != ARRAYSIZE(inputs)) {
        // error
        return 1;
    }

    if (delay_CacheGlitch > 0) Sleep(delay_CacheGlitch);

    // send Q down
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wScan = scanCodeQ;
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;
    nSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (nSent != ARRAYSIZE(inputs)) {
        // error
        return 1;
    }

    Sleep(5);

    // send Q up
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wScan = scanCodeQ; //0
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    nSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (nSent != ARRAYSIZE(inputs)) {
        // error
        return 1;
    }

    Sleep(5);

    // send C up
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wScan = scanCodeC; //0
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    nSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (nSent != ARRAYSIZE(inputs)) {
        // error
        return 2;
    }

    return 0;
}


// The hooked mouse message handler
LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    WORD mouseXButton;
    PMSLLHOOKSTRUCT pStruct = reinterpret_cast<PMSLLHOOKSTRUCT>(lParam);

    if (nCode == HC_ACTION) {
        if (pStruct) {
            switch (wParam) { // message
            case WM_XBUTTONUP:
                mouseXButton = HIWORD(pStruct->mouseData);
                switch (mouseXButton) {
                case XBUTTON1:
                    // Let the installer-thread execute the glitching.
                    // Just send it a message to start doing so..
                    PostThreadMessage(InstallerThreadId, WM_APP, 0, 1);
                    break;
                case XBUTTON2:
                    PostThreadMessage(InstallerThreadId, WM_APP, 0, 2);
                    break;
                }

                break;
            }
        }
    }

    return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}


// The installer thread that tries to set the hook.
// It also listens to messages from the mouse-handler to execute the glitching.
DWORD WINAPI installHook(LPVOID lpParm)
{
    MSG message;
    int result;
    HWND rHwnd;              // window handle of NMS
    DWORD pid = 0;           // processId of NMS
    HINSTANCE hInstance = GetModuleHandle(NULL);


    printf("Searching for 'No Man's Sky'.. ");
    rHwnd = FindWindow(NULL, L"No Man's Sky");
    if (!rHwnd) return errorOut(1, "Error: The game must be running.\n");
    printf("hWnd = %p.\n", rHwnd);


    printf("Getting the NMS threadId.. ");
    NMSthreadId = GetWindowThreadProcessId(rHwnd, &pid);
    if (!NMSthreadId) return errorOut(2, "Error: Can not get the threadId.\n");
    printf("%d.\n", NMSthreadId);


    printf("Hooking.. ");
    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseProc, hInstance, 0);
    if (!hMouseHook) return errorOut(3, "Error: Can not create the hook.\n");
    printf("It's all hooked now.\n");


    printColored("Happy glitchbuilding..\n", COLOR_OK);


    // Keep pumping messages
    while (GetMessage(&message, NULL, 0, 0)) {
        // a message from the callback routine ?
        if (message.message == WM_APP) {
            switch (message.lParam) {
            case 1:
                // Wire Glitch ('Q' + left mouse click)
                printf("XBUTTON1");
                result = WireGlitch();
                if (result != 0) {
                    printColored(" Wire Glitch\n", COLOR_ERROR);
                }
                else {
                    printf(" Wire Glitch");
                }
                
                break;
            case 2:
                // Cache Object ('Q' + 'C')
                printf("XBUTTON2 Cache Glitch\n");
                result = CacheGlitch();
                if (result != 0) {
                    printColored(" Cache Glitch\n", COLOR_ERROR);
                }
                else {
                    printf(" Cache Glitch");
                }
                break;
            }
        }
        else {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }


    printf("Unhooking mouse.. ");
    if (UnhookWindowsHookEx(hMouseHook) == 0) return errorOut(4, "Failed\n");
    printf("Done.\n");

    return 0;
}


#if defined(VALUES_FROM_FILE)
void readTimings() {
    // set to default values
    int wireGlitch = DELAYMS_WIREGLITCH;
    int cacheGlitch = DELAYMS_CACHEGLITCH;

    // try to read the values from file
    std::ifstream valuesFile("timings.txt");
    if (valuesFile.is_open()) {
        valuesFile >> wireGlitch;
        valuesFile >> cacheGlitch;
    }
    valuesFile.close();

    // set the application variables
    delay_WireGlitch = wireGlitch;
    delay_CacheGlitch = cacheGlitch;
    printf("     Wire Glitch: %d\n", delay_WireGlitch);
    printf("    Cache Glitch: %d\n", delay_CacheGlitch);
}
#endif


int main(int argc, char** argv)
{
    HANDLE hThread;

    printColored("=== 'No Man's Sky' Glitch Build Tool ===\n\n", COLOR_TITLE);

#if defined(VALUES_FROM_FILE)
    printf("Overwriting timing values read from file.. \n");
    readTimings();
#endif

    // Start a new thread that will install the hook,
    // and stays listening to messages from the hook procedure for when to glitchbuild.
    hThread = CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(installHook), reinterpret_cast<LPVOID>(argv[0]), NULL, &InstallerThreadId);
    if (hThread)
        return WaitForSingleObject(hThread, INFINITE);
    else
        return 1;
}