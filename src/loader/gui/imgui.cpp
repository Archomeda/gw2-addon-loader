// The code in this file is mostly from ImGui's example

#include "imgui.h"
#include <imgui.h>

namespace loader {
    namespace gui {
        namespace imgui {

            static INT64 imGuiFrequency;
            static INT64 imGuiTime;
            static IDirect3DDevice9* d3dDevice;
            static HWND imGuiWnd;
            static IDirect3DVertexBuffer9* imGuiVertexBuffer;
            static IDirect3DIndexBuffer9* imGuiIndexBuffer;
            static IDirect3DTexture9* imGuiFontTexture;
            static int imGuiVertexBufferSize = 5000;
            static int imGuiIndexBufferSize = 10000;

            struct ImGuiVertex {
                float pos[3];
                D3DCOLOR col;
                float uv[2];
            };
#define D3DFVF_IMGUIVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

            bool CreateFonts() {
                // Build texture atlas
                ImGuiIO& io = ImGui::GetIO();
                unsigned char* pixels;
                int width;
                int height;
                int bytesPerPixel;
                io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytesPerPixel);

                // Upload texture to graphics system
                imGuiFontTexture = nullptr;
                if (d3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &imGuiFontTexture, NULL) < 0) {
                    return false;
                }
                D3DLOCKED_RECT rect;
                if (imGuiFontTexture->LockRect(0, &rect, NULL, 0) != D3D_OK) {
                    return false;
                }
                for (int y = 0; y < height; ++y) {
                    memcpy(static_cast<unsigned char*>(rect.pBits) + rect.Pitch * y, pixels + (width * bytesPerPixel) * y, (width * bytesPerPixel));
                }
                imGuiFontTexture->UnlockRect(0);

                // Store our identifier
                io.Fonts->TexID = static_cast<void *>(imGuiFontTexture);

                return true;
            }


            void ImGuiRenderDrawLists(ImDrawData* drawData) {
                // Avoid rendering when minimized
                ImGuiIO& io = ImGui::GetIO();
                if (io.DisplaySize.x <= 0 || io.DisplaySize.y < 0) {
                    return;
                }

                // Create and grow buffers if needed
                if (!imGuiVertexBuffer || imGuiVertexBufferSize < drawData->TotalVtxCount) {
                    if (imGuiVertexBuffer) {
                        imGuiVertexBuffer->Release();
                        imGuiVertexBuffer = nullptr;
                    }
                    imGuiVertexBufferSize = drawData->TotalVtxCount + 5000;
                    if (d3dDevice->CreateVertexBuffer(imGuiVertexBufferSize * sizeof(ImGuiVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_IMGUIVERTEX, D3DPOOL_DEFAULT, &imGuiVertexBuffer, NULL) < 0) {
                        return;
                    }
                }
                if (!imGuiIndexBuffer || imGuiIndexBufferSize < drawData->TotalIdxCount) {
                    if (imGuiIndexBuffer) {
                        imGuiIndexBuffer->Release();
                        imGuiIndexBuffer = nullptr;
                    }
                    imGuiIndexBufferSize = drawData->TotalIdxCount + 5000;
                    if (d3dDevice->CreateIndexBuffer(imGuiIndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &imGuiIndexBuffer, NULL) < 0) {
                        return;
                    }
                }

                // Back up the DX9 state
                IDirect3DStateBlock9* stateBlock = nullptr;
                if (d3dDevice->CreateStateBlock(D3DSBT_ALL, &stateBlock) < 0) {
                    return;
                }

                // Copy and convert all vertices into a single contiguous buffer
                ImGuiVertex* vertexDest;
                ImDrawIdx* indexDest;
                if (imGuiVertexBuffer->Lock(0, static_cast<UINT>(drawData->TotalVtxCount * sizeof(ImGuiVertex)), reinterpret_cast<void**>(&vertexDest), D3DLOCK_DISCARD) < 0) {
                    return;
                }
                if (imGuiIndexBuffer->Lock(0, static_cast<UINT>(drawData->TotalIdxCount * sizeof(ImDrawIdx)), reinterpret_cast<void**>(&indexDest), D3DLOCK_DISCARD) < 0) {
                    return;
                }
                for (int n = 0; n < drawData->CmdListsCount; ++n) {
                    const ImDrawList* cmdList = drawData->CmdLists[n];
                    const ImDrawVert* vertexSrc = cmdList->VtxBuffer.Data;
                    for (int i = 0; i < cmdList->VtxBuffer.Size; ++i) {
                        vertexDest->pos[0] = vertexSrc->pos.x;
                        vertexDest->pos[1] = vertexSrc->pos.y;
                        vertexDest->pos[2] = 0;
                        vertexDest->col = (vertexSrc->col & 0xFF00FF00) | ((vertexSrc->col & 0xFF0000) >> 16) | ((vertexSrc->col & 0xFF) << 16); // RGBA --> ARGB for DirectX9
                        vertexDest->uv[0] = vertexSrc->uv.x;
                        vertexDest->uv[1] = vertexSrc->uv.y;
                        ++vertexDest;
                        ++vertexSrc;
                    }
                    memcpy(indexDest, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
                    indexDest += cmdList->IdxBuffer.Size;
                }
                imGuiVertexBuffer->Unlock();
                imGuiIndexBuffer->Unlock();
                d3dDevice->SetStreamSource(0, imGuiVertexBuffer, 0, sizeof(ImGuiVertex));
                d3dDevice->SetIndices(imGuiIndexBuffer);
                d3dDevice->SetFVF(D3DFVF_IMGUIVERTEX);

                // Set up viewport
                D3DVIEWPORT9 viewport;
                viewport.X = 0;
                viewport.Y = 0;
                viewport.Width = static_cast<DWORD>(io.DisplaySize.x);
                viewport.Height = static_cast<DWORD>(io.DisplaySize.y);
                viewport.MinZ = 0;
                viewport.MaxZ = 1;
                d3dDevice->SetViewport(&viewport);

                // Set up render state: fixed-pipeline, alpha-blending, no face culling, no depth testing
                d3dDevice->SetPixelShader(NULL);
                d3dDevice->SetVertexShader(NULL);
                d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
                d3dDevice->SetRenderState(D3DRS_LIGHTING, false);
                d3dDevice->SetRenderState(D3DRS_ZENABLE, false);
                d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
                d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
                d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
                d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
                d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
                d3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
                d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

                // Set up orthographic projection matrix
                // Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
                {
                    const float L = 0.5f;
                    const float R = io.DisplaySize.x + 0.5f;
                    const float T = 0.5f;
                    const float B = io.DisplaySize.y + 0.5f;
                    D3DMATRIX identityMatrix = { { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 } };
                    D3DMATRIX projectionMatrix = {
                        2.0f / (R - L), 0.0f, 0.0f, 0.0f,
                        0.0f, 2.0f / (T - B), 0.0f, 0.0f,
                        0.0f, 0.0f, 0.5f, 0.0f,
                        (L + R) / (L - R), (T + B) / (B - T), 0.5f, 1.0f
                    };
                    d3dDevice->SetTransform(D3DTS_WORLD, &identityMatrix);
                    d3dDevice->SetTransform(D3DTS_VIEW, &identityMatrix);
                    d3dDevice->SetTransform(D3DTS_PROJECTION, &projectionMatrix);
                }

                // Render command lists
                int vertexOffset = 0;
                int indexOffset = 0;
                for (int n = 0; n < drawData->CmdListsCount; ++n) {
                    const ImDrawList* cmdList = drawData->CmdLists[n];
                    for (int i = 0; i < cmdList->CmdBuffer.Size; ++i) {
                        const ImDrawCmd* cmd = &cmdList->CmdBuffer[i];
                        if (cmd->UserCallback) {
                            cmd->UserCallback(cmdList, cmd);
                        }
                        else {
                            const RECT r = { static_cast<LONG>(cmd->ClipRect.x), static_cast<LONG>(cmd->ClipRect.y), static_cast<LONG>(cmd->ClipRect.z), static_cast<LONG>(cmd->ClipRect.w) };
                            d3dDevice->SetTexture(0, static_cast<IDirect3DTexture9*>(cmd->TextureId));
                            d3dDevice->SetScissorRect(&r);
                            d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vertexOffset, 0, static_cast<UINT>(cmdList->VtxBuffer.Size), indexOffset, cmd->ElemCount / 3);
                        }
                        indexOffset += cmd->ElemCount;
                    }
                    vertexOffset += cmdList->VtxBuffer.Size;
                }

                // Restore the DX9 state
                stateBlock->Apply();
                stateBlock->Release();
            }

            bool ProcessWndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
                if (ImGui::GetCurrentContext() == NULL) {
                    return false;
                }

                ImGuiIO& io = ImGui::GetIO();
                switch (msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONDBLCLK:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONDBLCLK:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONDBLCLK: {
                    int button = 0;
                    if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) {
                        button = 0;
                    }
                    else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) {
                        button = 1;
                    }
                    else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) {
                        button = 2;
                    }
                    io.MouseDown[button] = true;
                    return false;
                }
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP: {
                    int button = 0;
                    if (msg == WM_LBUTTONUP) {
                        button = 0;
                    }
                    else if (msg == WM_RBUTTONUP) {
                        button = 1;
                    }
                    else if (msg == WM_MBUTTONUP) {
                        button = 2;
                    }
                    io.MouseDown[button] = false;
                    return false;
                }
                case WM_MOUSEWHEEL:
                    io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? 1 : -1;
                    return false;
                case WM_MOUSEMOVE:
                    if (msg == WM_MOUSEMOVE) {
                        io.MousePos.x = static_cast<short>(lParam);
                        io.MousePos.y = static_cast<short>(lParam >> 16);
                    }                    
                    return false;
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN:
                    if (wParam < 256) {
                        io.KeysDown[wParam] = 1;
                    }
                    return false;
                case WM_KEYUP:
                case WM_SYSKEYUP:
                    if (wParam < 256) {
                        io.KeysDown[wParam] = 0;
                    }
                    return false;
                case WM_CHAR:
                    // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
                    if (wParam > 0 && wParam < 0x10000) {
                        io.AddInputCharacter(static_cast<unsigned short>(wParam));
                    }
                    return false;
                }

                return false;
            }


            bool Initialize(HWND hWnd, IDirect3DDevice9* device) {
                imGuiWnd = hWnd;
                d3dDevice = device;

                if (!QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&imGuiFrequency))) {
                    return false;
                }
                if (!QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&imGuiTime))) {
                    return false;
                }

                ImGuiIO& io = ImGui::GetIO();
                io.KeyMap[ImGuiKey_Tab] = VK_TAB;
                io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
                io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
                io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
                io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
                io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
                io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
                io.KeyMap[ImGuiKey_Home] = VK_HOME;
                io.KeyMap[ImGuiKey_End] = VK_END;
                io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
                io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
                io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
                io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
                io.KeyMap[ImGuiKey_A] = 'A';
                io.KeyMap[ImGuiKey_C] = 'C';
                io.KeyMap[ImGuiKey_V] = 'V';
                io.KeyMap[ImGuiKey_X] = 'X';
                io.KeyMap[ImGuiKey_Y] = 'Y';
                io.KeyMap[ImGuiKey_Z] = 'Z';

                io.RenderDrawListsFn = &ImGuiRenderDrawLists;
                io.ImeWindowHandle = hWnd;

                return true;
            }

            void Shutdown() {
                if (imGuiVertexBuffer) {
                    imGuiVertexBuffer->Release();
                    imGuiVertexBuffer = nullptr;
                }
                if (imGuiIndexBuffer) {
                    imGuiIndexBuffer->Release();
                    imGuiIndexBuffer = nullptr;
                }

                ImGui::Shutdown();
                d3dDevice = nullptr;
                imGuiWnd = NULL;
            }

            void NewFrame() {
                if (!imGuiFontTexture) {
                    CreateDeviceObjects();
                }

                ImGuiIO& io = ImGui::GetIO();

                // Set up display size (every frame to accommodate for window resizing)
                RECT rect;
                GetClientRect(imGuiWnd, &rect);
                io.DisplaySize = ImVec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));

                // Set up time step
                INT64 time;
                QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));
                io.DeltaTime = static_cast<float>(time - imGuiTime) / imGuiFrequency;
                imGuiTime = time;

                // Read keybaord modifiers inputs
                io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
                io.KeySuper = false;

                // Start the frame
                ImGui::NewFrame();
            }

            bool UpdateMouseCursor() {
                ImGuiIO& io = ImGui::GetIO();
                if (!io.WantCaptureMouse) {
                    return false;
                }

                ImGuiMouseCursor cursor = io.MouseDrawCursor ? ImGuiMouseCursor_::ImGuiMouseCursor_None : ImGui::GetMouseCursor();
                if (cursor == ImGuiMouseCursor_::ImGuiMouseCursor_None) {
                    // Hide OS mouse cursor if ImGui is drawing it or if it wants no cursor
                    SetCursor(NULL);
                }
                else {
                    // Hardware cursor type
                    LPTSTR winCursor = IDC_ARROW;
                    switch (cursor) {
                    case ImGuiMouseCursor_::ImGuiMouseCursor_Arrow:
                        winCursor = IDC_ARROW;
                        break;
                    case ImGuiMouseCursor_::ImGuiMouseCursor_TextInput:
                        winCursor = IDC_IBEAM;
                        break;
                    case ImGuiMouseCursor_::ImGuiMouseCursor_Move:
                        winCursor = IDC_SIZEALL;
                        break;
                    case ImGuiMouseCursor_::ImGuiMouseCursor_ResizeEW:
                        winCursor = IDC_SIZEWE;
                        break;
                    case ImGuiMouseCursor_::ImGuiMouseCursor_ResizeNS:
                        winCursor = IDC_SIZENS;
                        break;
                    case ImGuiMouseCursor_::ImGuiMouseCursor_ResizeNESW:
                        winCursor = IDC_SIZENESW;
                        break;
                    case ImGuiMouseCursor_::ImGuiMouseCursor_ResizeNWSE:
                        winCursor = IDC_SIZENWSE;
                        break;
                    }
                    SetCursor(LoadCursor(NULL, winCursor));
                }
                return true;
            }

            bool CreateDeviceObjects() {
                if (!d3dDevice) {
                    return false;
                }
                if (!CreateFonts()) {
                    return false;
                }
                return true;
            }

            void InvalidateDeviceObjects() {
                if (!d3dDevice) {
                    return;
                }
                if (imGuiVertexBuffer) {
                    imGuiVertexBuffer->Release();
                    imGuiVertexBuffer = nullptr;
                }
                if (imGuiIndexBuffer) {
                    imGuiIndexBuffer->Release();
                    imGuiIndexBuffer = nullptr;
                }

                // At this point note that we set ImGui::GetIO().Fonts->TexID to be == imGuiFontTexture, so clear both.
                ImGuiIO& io = ImGui::GetIO();
                if (imGuiFontTexture)
                    imGuiFontTexture->Release();
                imGuiFontTexture = nullptr;
                io.Fonts->TexID = nullptr;
            }

        }
    }
}
