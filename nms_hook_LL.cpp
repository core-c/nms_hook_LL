#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#define COLOR_DEFAULT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_BOLD (COLOR_DEFAULT | FOREGROUND_INTENSITY)
#define COLOR_ERROR (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define COLOR_OK (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_TITLE (BACKGROUND_RED | BACKGROUND_GREEN)
#define COLOR_TIMINGS (FOREGROUND_RED | FOREGROUND_GREEN)


// The variables for the timing values can be overwritten with data read from a file.
// That file is a simple .txt file with a line, with 3 timing-values, for each delay.
// There are 3 delays, so there are 3 lines in the file.
// Each line contains 3 numbers (durations in microseconds).
// The first line contains the delay for the wire glitch. The default values are: 1000 62400 78000.
// The second line contains the delay for the cache glitch. The default values are: 46800 78000 78000.
// The third line contains the delay for the adjacency glitch. The default values are: 1000 62400 78000.
//
// '#define' or '#undef' to select a different compiled version.
#define VALUES_FROM_FILE

#if defined(VALUES_FROM_FILE)
#include <fstream>
#endif



// The message types for inter-app communication
#define MSG_DO_CACHEGLITCH 1
#define MSG_DO_WIREGLITCH 2
#define MSG_DO_ADJACENCYGLITCH 3

// The timing value variables. Units are microseconds.
int delay_WireGlitch[3]         = {  750, 50000, 50000 };
int delay_CacheGlitch[3]        = {  500, 50000, 50000 };
int delay_AdjacencyGlitch[3]    = { 1000, 50000, 50000 };


HHOOK hMouseHook;           // handle to the hooked procedure
DWORD NMSthreadId;          // threadId of NMS
DWORD InstallerThreadId;    // threadId of the installer thread
int64_t qpcFrequency;       // Performance Frequency



// print a colored text to the console.
// We print the linebreak(s) in the default color to prevent wrong coloring while the console is scrolling.
void printColored(WORD color, const char* txt, const char* linebreaks) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    printf(txt);
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT); // restore
    printf(linebreaks);
}

// print an error message and wait for a key...
int errorOut(int errorCode, const char* txt, const char* linebreaks) {
    printColored(COLOR_ERROR, txt, linebreaks);
    printf("\n\n");
    system("pause");
    return errorCode;
}

void printTimings() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, COLOR_TIMINGS);
    printf("      Wire Glitch: %d %d %d  \n", delay_WireGlitch[0], delay_WireGlitch[1], delay_WireGlitch[2]);
    printf("     Cache Glitch: %d %d %d  \n", delay_CacheGlitch[0], delay_CacheGlitch[1], delay_CacheGlitch[2]);
    printf(" Adjacency Glitch: %d %d %d  ", delay_AdjacencyGlitch[0], delay_AdjacencyGlitch[1], delay_AdjacencyGlitch[2]);
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT); // restore
    printf("\n\n");
}

// Sleep() does not wait for milliseconds at all.
// I measured 0.015 seconds for what was supposed to be a 1ms delay.
// And Sleep() delay also varies. It is not consistent => useless for our purpose.
// So, therefore this blocking delay function.
void delayUS(int us) {
    const int64_t overhead = 7; // 7 ticks overhead for checking/exiting the function
    int64_t qpcStart, qpcCurrent, qpcEnd, ticks;
    double nSeconds = us * 0.000001f;
    if (QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&qpcStart))) {
        ticks = static_cast<int64_t>(nSeconds * qpcFrequency) - overhead;
        qpcEnd = qpcStart + ticks;
        while (QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&qpcCurrent))) {
            if (qpcCurrent >= qpcEnd) break;
        }
    }
}

// restict timing values in the range [0..100000] microseconds.
int argToTiming(const char* arg) {
    int result = max(0, min(atoi(arg), 100000));
    return result;
}

// Send a key down/up for a given scancode
bool SendInput_Key(UINT scancode, DWORD up = 0) {
    INPUT inputs[1] = {};
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE | up;
    inputs[0].ki.wScan = scancode;
    return (SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT)) == ARRAYSIZE(inputs));
}

// send a mouse-button press/release
bool SendInput_Mouse(DWORD button) {
    INPUT inputs[1] = {};
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = button;
    return (SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT)) == ARRAYSIZE(inputs));
}


// This is the actual wire glitch: 'Q' + LMB
bool DoWireGlitch() {
    UINT scanCodeQ = MapVirtualKeyA('Q', MAPVK_VK_TO_VSC);

    if (!SendInput_Key(scanCodeQ)) return FALSE;
    if (delay_WireGlitch[0] > 0) delayUS(delay_WireGlitch[0]);
    if (!SendInput_Mouse(MOUSEEVENTF_LEFTDOWN)) return FALSE;
    if (delay_WireGlitch[1] > 0) delayUS(delay_WireGlitch[1]);
    if (!SendInput_Key(scanCodeQ, KEYEVENTF_KEYUP)) return FALSE;
    if (delay_WireGlitch[2] > 0) delayUS(delay_WireGlitch[2]);
    if (!SendInput_Mouse(MOUSEEVENTF_LEFTUP)) return FALSE;

    return TRUE;
}

// This is the actual cache glitch: 'Q' + 'C'
bool DoCacheGlitch() {
    UINT scanCodeQ = MapVirtualKeyA('Q', MAPVK_VK_TO_VSC);
    UINT scanCodeC = MapVirtualKeyA('C', MAPVK_VK_TO_VSC);

    if (!SendInput_Key(scanCodeC)) return FALSE;
    if (delay_CacheGlitch[0] > 0) delayUS(delay_CacheGlitch[0]);
    if (!SendInput_Key(scanCodeQ)) return FALSE;
    if (delay_CacheGlitch[1] > 0) delayUS(delay_CacheGlitch[1]);
    if (!SendInput_Key(scanCodeQ, KEYEVENTF_KEYUP)) return FALSE;
    if (delay_CacheGlitch[2] > 0) delayUS(delay_CacheGlitch[2]);
    if (!SendInput_Key(scanCodeC, KEYEVENTF_KEYUP)) return FALSE;

    return TRUE;
}

// This is the actual adjacency glitch: 'E' + LMB
bool DoAdjacencyGlitch() {
    UINT scanCodeE = MapVirtualKeyA('E', MAPVK_VK_TO_VSC);

    if (!SendInput_Key(scanCodeE)) return FALSE;
    if (delay_AdjacencyGlitch[0] > 0) delayUS(delay_AdjacencyGlitch[0]);
    if (!SendInput_Mouse(MOUSEEVENTF_LEFTDOWN)) return FALSE;
    if (delay_AdjacencyGlitch[1] > 0) delayUS(delay_AdjacencyGlitch[1]);
    if (!SendInput_Key(scanCodeE, KEYEVENTF_KEYUP)) return FALSE;
    if (delay_AdjacencyGlitch[2] > 0) delayUS(delay_AdjacencyGlitch[2]);
    if (!SendInput_Mouse(MOUSEEVENTF_LEFTUP)) return FALSE;

    return TRUE;
}


// The hooked mouse message handler
LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    WORD mouseXButton;
    bool isCtrlPressed;
    PMSLLHOOKSTRUCT pStruct = reinterpret_cast<PMSLLHOOKSTRUCT>(lParam);

    if (nCode == HC_ACTION) {
        if (pStruct) {
            switch (wParam) { // message
            case WM_XBUTTONUP:
                isCtrlPressed = (GetKeyState(VK_LCONTROL) & 0x8000);
                mouseXButton = HIWORD(pStruct->mouseData);
                switch (mouseXButton) {
                case XBUTTON1:
                    // If the ctrl-key is not pressed, we do a cache-glitch.
                    // If the ctrl-key is pressed, we do an adjacency-glitch.
                    if (isCtrlPressed) {
                        // Let the installer-thread execute the glitching.
                        // Just send it a message to start doing so..
                        // This keeps the handler itself responsive/fast.
                        PostThreadMessage(InstallerThreadId, WM_APP, 0, MSG_DO_ADJACENCYGLITCH);
                    }
                    else {
                        PostThreadMessage(InstallerThreadId, WM_APP, 0, MSG_DO_CACHEGLITCH);
                    }
                    break;
                case XBUTTON2:
                    PostThreadMessage(InstallerThreadId, WM_APP, 0, MSG_DO_WIREGLITCH);
                    break;
                }

                break;
            }
        }
    }

    return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}


// The installer thread that hooks the mouse handler.
// It also listens to messages from the mouse-handler to start executing the glitching.
DWORD WINAPI installHook(LPVOID lpParm)
{
    WORD color;
    MSG message;
    HWND rHwnd;              // window handle of NMS
    DWORD pid = 0;           // processId of NMS
    HINSTANCE hInstance = GetModuleHandle(NULL);

    printf("Reading Performance Frequency.. ");
    if (QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&qpcFrequency))) {
        printf("%lld Hz\n", qpcFrequency);
    }
    else {
        return errorOut(1, "Error: The Performance Frequency can not be read", "\n");
    }

    printf("Searching for 'No Man's Sky'.. ");
    rHwnd = FindWindow(NULL, L"No Man's Sky");
    if (!rHwnd) return errorOut(2, "Error: The game must be running", "\n");
    printf("hWnd = %p\n", rHwnd);


    printf("Getting the NMS thread.. ");
    NMSthreadId = GetWindowThreadProcessId(rHwnd, &pid);
    if (!NMSthreadId) return errorOut(3, "Error: Can not get the threadId", "\n");
    printf("threadId = %d\n", NMSthreadId);


    printf("Hooking.. ");
    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseProc, hInstance, 0);
    if (!hMouseHook) return errorOut(4, "Error: Can not create the hook", "\n");
    printf("successful\n");


    printColored(COLOR_OK, "Happy glitchbuilding..", "\n");


    // Keep pumping messages
    while (GetMessage(&message, NULL, 0, 0)) {
        // a message from the callback routine ?
        if (message.message == WM_APP) {
            switch (message.lParam) {
            case MSG_DO_WIREGLITCH:
                color = (DoWireGlitch()) ? COLOR_DEFAULT : COLOR_ERROR;
                printColored(color, "Wire Glitch", "\n");
                break;
           case MSG_DO_CACHEGLITCH:
                color = (DoCacheGlitch()) ? COLOR_DEFAULT : COLOR_ERROR;
                printColored(color, "Cache Glitch", "\n");
                break;
            case MSG_DO_ADJACENCYGLITCH:
                color = (DoAdjacencyGlitch()) ? COLOR_DEFAULT : COLOR_ERROR;
                printColored(color, "Adjacency Glitch", "\n");
                break;
            }
    }
        // process the message
        else {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }


    printf("Unhooking mouse.. ");
    if (UnhookWindowsHookEx(hMouseHook) == 0) return errorOut(5, "Failed", "\n");
    printf("Done\n");

    return 0;
}


#if defined(VALUES_FROM_FILE)
void readTimings() {
    bool result = FALSE;
    // set to default values
    int fWireGlitch[3] = { delay_WireGlitch[0], delay_WireGlitch[1], delay_WireGlitch[2] };
    int fCacheGlitch[3] = { delay_CacheGlitch[0], delay_CacheGlitch[1], delay_CacheGlitch[2] };
    int fAdjacencyGlitch[3] = { delay_AdjacencyGlitch[0], delay_AdjacencyGlitch[1], delay_AdjacencyGlitch[2] };

    // try to read the values from file
    std::ifstream valuesFile("timings.txt");
    if (valuesFile.is_open()) {
        if (valuesFile >> fWireGlitch[0] >> fWireGlitch[1] >> fWireGlitch[2])
            if (valuesFile >> fCacheGlitch[0] >> fCacheGlitch[1] >> fCacheGlitch[2])
                if (valuesFile >> fAdjacencyGlitch[0] >> fAdjacencyGlitch[1] >> fAdjacencyGlitch[2]) {
                    // if all reading succeeded, set the application variables
                    std::copy(fWireGlitch, fWireGlitch + 3, delay_WireGlitch);
                    std::copy(fCacheGlitch, fCacheGlitch + 3, delay_CacheGlitch);
                    std::copy(fAdjacencyGlitch, fAdjacencyGlitch + 3, delay_AdjacencyGlitch);
                    result = TRUE;
                }
    }
    valuesFile.close();
    if (result) printColored(COLOR_OK, "Loaded", "");
           else printColored(COLOR_ERROR, "Failed to load", "");
}

void writeTimings() {
    bool result = FALSE;
    std::ofstream valuesFile("timings.txt");
    if (valuesFile.is_open()) {
        if (valuesFile << delay_WireGlitch[0] << " " << delay_WireGlitch[1] << " " << delay_WireGlitch[2] << "\n")
            if (valuesFile << delay_CacheGlitch[0] << " " << delay_CacheGlitch[1] << " " << delay_CacheGlitch[2] << "\n")
                if (valuesFile << delay_AdjacencyGlitch[0] << " " << delay_AdjacencyGlitch[1] << " " << delay_AdjacencyGlitch[2] << "\n")
                    result = TRUE;
    }
    valuesFile.close();
    if (result) printColored(COLOR_OK, "Saved", "");
           else printColored(COLOR_ERROR, "Failed to save", "");
}
#endif


int main(int argc, char** argv)
{
    HANDLE hThread;

    printColored(COLOR_TITLE, "=== 'No Man's Sky' Glitch Build Tool ===", "\n\n");

    switch (argc) {
    case 10: // CLI 10 arguments? (specify all timing values)
    case 11: // or 11 arguments?  trailing 's' (save values)
        printf("Using all timing values read from arguments.. ");
        delay_WireGlitch[0] = argToTiming(argv[1]);
        delay_WireGlitch[1] = argToTiming(argv[2]);
        delay_WireGlitch[2] = argToTiming(argv[3]);
        delay_CacheGlitch[0] = argToTiming(argv[4]);
        delay_CacheGlitch[1] = argToTiming(argv[5]);
        delay_CacheGlitch[2] = argToTiming(argv[6]);
        delay_AdjacencyGlitch[0] = argToTiming(argv[7]);
        delay_AdjacencyGlitch[1] = argToTiming(argv[8]);
        delay_AdjacencyGlitch[2] = argToTiming(argv[9]);
        // save the values to file?
        if (argc == 11 && strcmp(argv[10], "s") == 0) {
            writeTimings();
        }
        printf("\n");
        printTimings();
        break;
    case 4: // CLI 3 arguments? (specify only main timing values)
    case 5: // or 4 arguments?  trailing 's' (save values)
        printf("Using main timing values read from arguments.. ");
        delay_WireGlitch[0] = argToTiming(argv[1]);
        delay_CacheGlitch[0] = argToTiming(argv[2]);
        delay_AdjacencyGlitch[0] = argToTiming(argv[3]);
        // save the values to file?
        if (argc == 5 && strcmp(argv[4], "s") == 0) {
            writeTimings();
        }
        printf("\n");
        printTimings();
        break;
    case 2: // Show only the current timings ?
        if (strcmp(argv[1], "t") == 0) {
            printf("Current timing values:\n");
            printTimings();
            return 0; // exit the application
        }
        break;
    default:
#if defined(VALUES_FROM_FILE)
        // There are no arguments provided on the command line.
        // Try reading values from file.
        printf("Using timing values read from file.. ");
        readTimings();
        printf("\n");
        printTimings();
#endif
        break;
    }

    // Start a new thread that will install the hook,
    // and stays listening to messages from the hook procedure for when to glitchbuild.
    hThread = CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(installHook), reinterpret_cast<LPVOID>(argv[0]), NULL, &InstallerThreadId);
    if (hThread)
        return WaitForSingleObject(hThread, INFINITE);
    else
        return 1;
}