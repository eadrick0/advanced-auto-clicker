#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#include "imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include <d3d11.h>
#include <windows.h>
#include <tchar.h>
#include <thread>
#include <atomic>
#include <string>

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

std::atomic<bool> g_isClicking = false;
std::atomic<int> g_clickDelay = 1000;
std::atomic<bool> g_rightClick = false;
POINT g_clickPosition = {0, 0};
std::atomic<bool> g_useCurrentPos = true;
std::thread* g_clickerThread = nullptr;

// Global hotkey ID
const int START_STOP_HOTKEY_ID = 1;
std::atomic<int> g_startKey = VK_F6;
bool g_ctrlKey = false;
bool g_altKey = false;
bool g_shiftKey = false;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::string GetKeyName(int key) {
    static char keyName[256];
    UINT scanCode = MapVirtualKey(key, MAPVK_VK_TO_VSC);
    GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName));
    return std::string(keyName);
}

void RegisterHotkey(HWND hwnd) {
    // Önce varolan kısayolu kaldır
    UnregisterHotKey(hwnd, START_STOP_HOTKEY_ID);
    
    // Yeni kısayolu kaydet
    int modifiers = 0;
    if (g_ctrlKey) modifiers |= MOD_CONTROL;
    if (g_altKey) modifiers |= MOD_ALT;
    if (g_shiftKey) modifiers |= MOD_SHIFT;
    
    RegisterHotKey(hwnd, START_STOP_HOTKEY_ID, modifiers, g_startKey);
}

void ApplyCustomStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowBorderSize = 1.0f;
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.95f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.26f, 0.26f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.36f, 0.36f, 0.36f, 0.54f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.35f, 0.41f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.37f, 0.46f, 0.54f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
}

void ClickerFunction() {
    while (g_isClicking) {
        if (g_useCurrentPos) {
            GetCursorPos(&g_clickPosition);
        }
        
        INPUT input[2] = {};
        input[0].type = INPUT_MOUSE;
        input[0].mi.dwFlags = g_rightClick ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
        input[1].type = INPUT_MOUSE;
        input[1].mi.dwFlags = g_rightClick ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_LEFTUP;

        if (!g_useCurrentPos) {
            SetCursorPos(g_clickPosition.x, g_clickPosition.y);
        }
        
        SendInput(2, input, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(g_clickDelay));
    }
}

void ToggleClicker() {
    g_isClicking = !g_isClicking;
    if (g_isClicking) {
        if (g_clickerThread) {
            g_clickerThread->join();
            delete g_clickerThread;
        }
        g_clickerThread = new std::thread(ClickerFunction);
    } else if (g_clickerThread) {
        g_clickerThread->join();
        delete g_clickerThread;
        g_clickerThread = nullptr;
    }
}

int main(int, char**) {
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Auto Clicker", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Advanced Auto Clicker", WS_OVERLAPPEDWINDOW, 100, 100, 500, 400, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Başlangıç kısayol tuşunu kaydet
    RegisterHotkey(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ApplyCustomStyle();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool waitingForKey = false;
    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
            else if (msg.message == WM_HOTKEY && msg.wParam == START_STOP_HOTKEY_ID) {
                ToggleClicker();
            }
        }
        if (done)
            break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Auto Clicker Controls", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::PushStyleColor(ImGuiCol_Button, g_isClicking ? ImVec4(0.7f, 0.2f, 0.2f, 1.0f) : ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        if (ImGui::Button(g_isClicking ? "Stop" : "Start", ImVec2(ImGui::GetWindowWidth() - 20, 40))) {
            ToggleClicker();
        }
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (waitingForKey) {
            ImGui::TextColored(ImVec4(1,1,0,1), "Press the new shortcut key...");
            for (int i = 1; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    g_startKey = i;
                    waitingForKey = false;
                    RegisterHotkey(hwnd);
                    break;
                }
            }
        }

        // Kısayol tuşu ayarları
        if (ImGui::TreeNode("Shortcut Key Settings")) {
            bool changed = false;
            
            if (ImGui::Button(("Shortcut: " + GetKeyName(g_startKey)).c_str())) {
                waitingForKey = true;
            }

            changed |= ImGui::Checkbox("CTRL", &g_ctrlKey);
            ImGui::SameLine();
            changed |= ImGui::Checkbox("ALT", &g_altKey);
            ImGui::SameLine();
            changed |= ImGui::Checkbox("SHIFT", &g_shiftKey);

            if (changed) {
                RegisterHotkey(hwnd);
            }

            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        int delay = g_clickDelay;
        if (ImGui::SliderInt("Click Latency (ms)", &delay, 1, 5000)) {
            g_clickDelay = delay;
        }

        bool rightClick = g_rightClick;
        if (ImGui::Checkbox("Right Click", &rightClick)) {
            g_rightClick = rightClick;
        }

        bool useCurrentPos = g_useCurrentPos;
        if (ImGui::Checkbox("Use Current Cursor Position", &useCurrentPos)) {
            g_useCurrentPos = useCurrentPos;
        }

        if (!g_useCurrentPos) {
            ImGui::Text("Click Location: X=%d Y=%d", g_clickPosition.x, g_clickPosition.y);
            if (ImGui::Button("Get Current Location", ImVec2(ImGui::GetWindowWidth() - 20, 30))) {
                GetCursorPos(&g_clickPosition);
            }
        }

        ImGui::End();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    if (g_isClicking && g_clickerThread) {
        g_isClicking = false;
        g_clickerThread->join();
        delete g_clickerThread;
    }

    // Kısayol tuşunu kaldır
    UnregisterHotKey(hwnd, START_STOP_HOTKEY_ID);

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}