#include "highlight_cursor.h"
#include "../resource.h"

using namespace std;

namespace addon::qol {

    void* arrowImage = nullptr;
    IDirect3DVertexBuffer9* vertexBuffer = nullptr;
    const int vertexCount = 4;
    IDirect3DTexture9* arrowTexture = nullptr;
    const RECT arrowRect = { 0, 0, 256, 256 };

    chrono::steady_clock::time_point loopStart;
    const auto loopInDuration = 0.5e9ns; // 0.5s
    const auto loopOutDuration = 0.5e9ns; // 0.5s

    struct Vertex {
        float x, y, z;
        D3DCOLOR color;
        float tu, tv;
    };
    const DWORD VertexShaderFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

    void* LoadEmbeddedResource(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType, UINT* pSize) {
        HRSRC resource = FindResource(hModule, lpName, lpType);
        if (!resource) {
            return nullptr;
        }
        *pSize = SizeofResource(hModule, resource);
        if (!*pSize) {
            return nullptr;
        }
        HGLOBAL resourceData = LoadResource(hModule, resource);
        if (!resourceData) {
            return nullptr;
        }
        void* binaryData = LockResource(resourceData);
        if (!binaryData) {
            return nullptr;
        }
        return binaryData;
    }

    void SetArrowVertexBuffer(Vertex* const vertex, int alpha) {
        const float left = static_cast<float>(arrowRect.left);
        const float top = static_cast<float>(arrowRect.top);
        const float right = static_cast<float>(arrowRect.right);
        const float bottom = static_cast<float>(arrowRect.bottom);
        const D3DCOLOR color = D3DCOLOR_ARGB(alpha, 255, 255, 0);

        vertex[0].x = left;
        vertex[0].y = top;
        vertex[0].z = 0;
        vertex[0].color = color;
        vertex[0].tu = 0;
        vertex[0].tv = 0;
        vertex[1].x = right;
        vertex[1].y = top;
        vertex[1].z = 0;
        vertex[1].color = color;
        vertex[1].tu = 1;
        vertex[1].tv = 0;
        vertex[2].x = left;
        vertex[2].y = bottom;
        vertex[2].z = 0;
        vertex[2].color = color;
        vertex[2].tu = 0;
        vertex[2].tv = 1;
        vertex[3].x = right;
        vertex[3].y = bottom;
        vertex[3].z = 0;
        vertex[3].color = color;
        vertex[3].tu = 1;
        vertex[3].tv = 1;
    }


    void DisableHighlightCursor(IDirect3DDevice9* const pDev) {
        if (vertexBuffer) {
            vertexBuffer->Release();
            vertexBuffer = nullptr;
        }
        if (arrowTexture) {
            arrowTexture->Release();
            arrowTexture = nullptr;
        }
        if (arrowImage) {
            arrowImage = nullptr;
        }
    }

    void EnableHighlightCursor(HMODULE hModule, IDirect3DDevice9* const pDev) {
        if (!arrowImage) {
            UINT arrowSize;
            arrowImage = LoadEmbeddedResource(hModule, MAKEINTRESOURCE(IDB_PNGARROW), L"PNG", &arrowSize);
            if (D3DXCreateTextureFromFileInMemory(pDev, arrowImage, arrowSize, &arrowTexture)) {
                arrowTexture = nullptr;
                arrowImage = nullptr;
                return;
            }
        }
        if (!vertexBuffer) {
            if (pDev->CreateVertexBuffer(vertexCount * sizeof(Vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, VertexShaderFvf, D3DPOOL_DEFAULT, &vertexBuffer, NULL) < 0) {
                vertexBuffer = nullptr;
                return;
            }
        }
    }

    void RenderHighlightCursor(HWND hWnd, IDirect3DDevice9* const pDev) {
        // Determine the loop progression variables
        const auto loopDuration = chrono::steady_clock::now() - loopStart;
        if (loopDuration > loopInDuration + loopOutDuration) {
            // We are outside the loop, quit early
            return;
        }
        float animationOpacity = 1;
        float animationPos = 0;
        if (loopDuration < loopInDuration) {
            animationOpacity = static_cast<float>(loopDuration.count()) / static_cast<float>(loopInDuration.count());
            animationPos = 0.2f - (animationOpacity * 0.2f);
        }
        else if (loopDuration - loopInDuration < loopOutDuration) {
            animationOpacity = 1 - static_cast<float>((loopDuration - loopInDuration).count()) / static_cast<float>(loopOutDuration.count());
        }

        POINT cursorPos;
        if (!GetCursorPos(&cursorPos)) {
            return;
        }
        RECT clientRect;
        if (!GetClientRect(hWnd, &clientRect)) {
            return;
        }

        pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        pDev->SetRenderState(D3DRS_LIGHTING, false);
        pDev->SetRenderState(D3DRS_ZENABLE, false);
        pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
        pDev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
        pDev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        pDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        pDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        pDev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
        pDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        pDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        pDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

        Vertex* vertexDest;
        if (vertexBuffer->Lock(0, static_cast<UINT>(vertexCount * sizeof(Vertex)), reinterpret_cast<void**>(&vertexDest), D3DLOCK_DISCARD) < 0) {
            return;
        }
        SetArrowVertexBuffer(vertexDest, static_cast<int>(animationOpacity * 255));
        vertexBuffer->Unlock();

        pDev->SetFVF(VertexShaderFvf);
        pDev->SetStreamSource(0, vertexBuffer, 0, sizeof(Vertex));
        pDev->SetTexture(0, arrowTexture);

        D3DXMATRIX identityMatrix;
        D3DXMatrixIdentity(&identityMatrix);
        pDev->SetTransform(D3DTS_VIEW, &identityMatrix);

        D3DXMATRIX projectionMatrix;
        D3DXMatrixOrthoOffCenterLH(&projectionMatrix, clientRect.left + 0.5f, clientRect.right + 0.5f, clientRect.bottom + 0.5f, clientRect.top + 0.5f, 0, 1);
        pDev->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

        D3DXMATRIX preTranslationMatrix;
        D3DXMatrixTranslation(&preTranslationMatrix, -(arrowRect.right - arrowRect.left) * 0.5f, (arrowRect.bottom - arrowRect.top) * (0.05f + animationPos), 0);

        for (int i = 0; i < 4; ++i) {
            // Draw 4 arrows, each with a different angle
            D3DXMATRIX transformationMatrix;
            D3DXVECTOR2 scaling(0.25f, 0.25f);
            D3DXVECTOR2 translation(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y));
            D3DXMatrixTransformation2D(&transformationMatrix, NULL, 0, &scaling, NULL, D3DXToRadian(90 * i), &translation);

            pDev->SetTransform(D3DTS_WORLD, &(preTranslationMatrix * transformationMatrix));
            pDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        }
    }

    void TriggerHighlightCursor() {
        const auto loopDuration = chrono::steady_clock::now() - loopStart;
        if (loopDuration < loopInDuration + loopOutDuration) {
            // We are inside the loop, no need to reset
            return;
        }
        loopStart = chrono::steady_clock::now();
    }

}
