#include "highlight_cursor.h"
#include "../resource.h"

using namespace std;

namespace addon::qol {

    CursorHighlightType highlightType;

    IDirect3DVertexBuffer9* vertexBuffer = nullptr;
    int vertexCount = 0;
    void* highlightImage = nullptr;
    IDirect3DTexture9* highlightTexture = nullptr;
    const RECT highlightRect = { 0, 0, 256, 256 };
    D3DCOLOR highlightColor = 0xFFFFFF;
    RECT clientRect;
    POINT cursorPos;

    chrono::steady_clock::time_point loopStart;
    chrono::duration<double, std::nano> loopMaxDuration;

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

    void SetHighlightVertexBuffer(Vertex* const vertex, int startIndex, int alpha) {
        const float left = static_cast<float>(highlightRect.left);
        const float top = static_cast<float>(highlightRect.top);
        const float right = static_cast<float>(highlightRect.right);
        const float bottom = static_cast<float>(highlightRect.bottom);
        const D3DCOLOR color = (alpha << 24) | (highlightColor & 0xFFFFFF);

        vertex[startIndex + 0].x = left;
        vertex[startIndex + 0].y = top;
        vertex[startIndex + 0].z = 0;
        vertex[startIndex + 0].color = color;
        vertex[startIndex + 0].tu = 0;
        vertex[startIndex + 0].tv = 0;
        vertex[startIndex + 1].x = right;
        vertex[startIndex + 1].y = top;
        vertex[startIndex + 1].z = 0;
        vertex[startIndex + 1].color = color;
        vertex[startIndex + 1].tu = 1;
        vertex[startIndex + 1].tv = 0;
        vertex[startIndex + 2].x = left;
        vertex[startIndex + 2].y = bottom;
        vertex[startIndex + 2].z = 0;
        vertex[startIndex + 2].color = color;
        vertex[startIndex + 2].tu = 0;
        vertex[startIndex + 2].tv = 1;
        vertex[startIndex + 3].x = right;
        vertex[startIndex + 3].y = bottom;
        vertex[startIndex + 3].z = 0;
        vertex[startIndex + 3].color = color;
        vertex[startIndex + 3].tu = 1;
        vertex[startIndex + 3].tv = 1;
    }


    void RenderArrows(IDirect3DDevice9* const pDev) {
        static const auto loopInDuration = 0.5e9ns; // 0.5s
        static const auto loopOutDuration = 0.5e9ns; // 0.5s

        // Determine the loop progression variables
        const auto loopDuration = chrono::steady_clock::now() - loopStart;
        float animationOpacity = 1;
        float animationPos = 0;
        if (loopDuration < loopInDuration) {
            animationOpacity = static_cast<float>(loopDuration.count()) / static_cast<float>(loopInDuration.count());
            animationPos = 0.2f - (animationOpacity * 0.2f);
        }
        else if (loopDuration - loopInDuration < loopOutDuration) {
            animationOpacity = 1 - static_cast<float>((loopDuration - loopInDuration).count()) / static_cast<float>(loopOutDuration.count());
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
        SetHighlightVertexBuffer(vertexDest, 0, static_cast<int>(animationOpacity * 255));
        vertexBuffer->Unlock();

        pDev->SetFVF(VertexShaderFvf);
        pDev->SetStreamSource(0, vertexBuffer, 0, sizeof(Vertex));
        pDev->SetTexture(0, highlightTexture);

        D3DXMATRIX identityMatrix;
        D3DXMatrixIdentity(&identityMatrix);
        pDev->SetTransform(D3DTS_VIEW, &identityMatrix);

        D3DXMATRIX projectionMatrix;
        D3DXMatrixOrthoOffCenterLH(&projectionMatrix, clientRect.left + 0.5f, clientRect.right + 0.5f, clientRect.bottom + 0.5f, clientRect.top + 0.5f, 0, 1);
        pDev->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

        D3DXMATRIX preTranslationMatrix;
        D3DXMatrixTranslation(&preTranslationMatrix, -(highlightRect.right - highlightRect.left) * 0.5f, (highlightRect.bottom - highlightRect.top) * (0.05f + animationPos), 0);

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

    void RenderCirclesIn(IDirect3DDevice9* const pDev) {
        static const auto loopInDuration = 0.35e9ns; // 0.5s
        static const auto loopOutDuration = 0.35e9ns; // 0.5s
        static const auto circleInterval = 0.2e9ns; // 0.2s
        static const int circleCount = 3;

        // Determine the loop progression variables
        const auto loopDuration = chrono::steady_clock::now() - loopStart;
        float animationOpacity[circleCount];
        float animationScale[circleCount];
        for (int i = 0; i < circleCount; ++i) {
            const auto normalizedLoopDuration = loopDuration - (i * circleInterval);
            if (loopDuration < i * circleInterval) {
                // This one shouldn't be visible yet
                animationOpacity[i] = 0;
                animationScale[i] = 0;
            }
            else if (normalizedLoopDuration < loopInDuration) {
                animationOpacity[i] = static_cast<float>(normalizedLoopDuration.count()) / static_cast<float>(loopInDuration.count());
                animationScale[i] = 1.2f - (animationOpacity[i] * 0.6f);
            }
            else if (normalizedLoopDuration - loopInDuration < loopOutDuration) {
                animationOpacity[i] = 1 - static_cast<float>((normalizedLoopDuration - loopInDuration).count()) / static_cast<float>(loopOutDuration.count());
                animationScale[i] = 0.6f;
            }
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

        pDev->SetFVF(VertexShaderFvf);
        pDev->SetStreamSource(0, vertexBuffer, 0, sizeof(Vertex));
        pDev->SetTexture(0, highlightTexture);

        D3DXMATRIX identityMatrix;
        D3DXMatrixIdentity(&identityMatrix);
        pDev->SetTransform(D3DTS_VIEW, &identityMatrix);

        D3DXMATRIX projectionMatrix;
        D3DXMatrixOrthoOffCenterLH(&projectionMatrix, clientRect.left + 0.5f, clientRect.right + 0.5f, clientRect.bottom + 0.5f, clientRect.top + 0.5f, 0, 1);
        pDev->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

        D3DXMATRIX preTranslationMatrix;
        D3DXMatrixTranslation(&preTranslationMatrix, -(highlightRect.right - highlightRect.left) * 0.5f, -(highlightRect.bottom - highlightRect.top) * 0.5f, 0);

        Vertex* vertexDest;
        if (vertexBuffer->Lock(0, static_cast<UINT>(vertexCount * sizeof(Vertex)), reinterpret_cast<void**>(&vertexDest), D3DLOCK_DISCARD) < 0) {
            return;
        }
        for (int i = 0; i < circleCount; ++i) {
            SetHighlightVertexBuffer(vertexDest, i * 4, static_cast<int>(animationOpacity[i] * 255));
        }
        vertexBuffer->Unlock();

        // Draw the circles
        for (int i = 0; i < circleCount; ++i) {
            D3DXMATRIX transformationMatrix;
            D3DXVECTOR2 scaling(0.25f * animationScale[i], 0.25f * animationScale[i]);
            D3DXVECTOR2 translation(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y));
            D3DXMatrixTransformation2D(&transformationMatrix, NULL, 0, &scaling, NULL, NULL, &translation);

            pDev->SetTransform(D3DTS_WORLD, &(preTranslationMatrix * transformationMatrix));
            pDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * 4, 2);
        }
    }

    void RenderCirclesOut(IDirect3DDevice9* const pDev) {
        static const auto loopInDuration = 0.1e9ns; // 0.1s
        static const auto loopOutDuration = 0.6e9ns; // 0.6s
        static const auto circleInterval = 0.2e9ns; // 0.2s
        static const int circleCount = 3;

        // Determine the loop progression variables
        const auto loopDuration = chrono::steady_clock::now() - loopStart;
        float animationOpacity[circleCount];
        float animationScale[circleCount];
        for (int i = 0; i < circleCount; ++i) {
            const auto normalizedLoopDuration = loopDuration - (i * circleInterval);
            if (loopDuration < i * circleInterval) {
                // This one shouldn't be visible yet
                animationOpacity[i] = 0;
                animationScale[i] = 0;
            }
            else if (normalizedLoopDuration < loopInDuration) {
                animationOpacity[i] = static_cast<float>(normalizedLoopDuration.count()) / static_cast<float>(loopInDuration.count());
                animationScale[i] = 0.6f;
            }
            else if (normalizedLoopDuration - loopInDuration < loopOutDuration) {
                animationOpacity[i] = 1 - static_cast<float>((normalizedLoopDuration - loopInDuration).count()) / static_cast<float>(loopOutDuration.count());
                animationScale[i] = 1.2f - (animationOpacity[i] * 0.6f);
            }
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

        pDev->SetFVF(VertexShaderFvf);
        pDev->SetStreamSource(0, vertexBuffer, 0, sizeof(Vertex));
        pDev->SetTexture(0, highlightTexture);

        D3DXMATRIX identityMatrix;
        D3DXMatrixIdentity(&identityMatrix);
        pDev->SetTransform(D3DTS_VIEW, &identityMatrix);

        D3DXMATRIX projectionMatrix;
        D3DXMatrixOrthoOffCenterLH(&projectionMatrix, clientRect.left + 0.5f, clientRect.right + 0.5f, clientRect.bottom + 0.5f, clientRect.top + 0.5f, 0, 1);
        pDev->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

        D3DXMATRIX preTranslationMatrix;
        D3DXMatrixTranslation(&preTranslationMatrix, -(highlightRect.right - highlightRect.left) * 0.5f, -(highlightRect.bottom - highlightRect.top) * 0.5f, 0);

        Vertex* vertexDest;
        if (vertexBuffer->Lock(0, static_cast<UINT>(vertexCount * sizeof(Vertex)), reinterpret_cast<void**>(&vertexDest), D3DLOCK_DISCARD) < 0) {
            return;
        }
        for (int i = 0; i < circleCount; ++i) {
            SetHighlightVertexBuffer(vertexDest, i * 4, static_cast<int>(animationOpacity[i] * 255));
        }
        vertexBuffer->Unlock();

        // Draw the circles
        for (int i = 0; i < circleCount; ++i) {
            D3DXMATRIX transformationMatrix;
            D3DXVECTOR2 scaling(0.25f * animationScale[i], 0.25f * animationScale[i]);
            D3DXVECTOR2 translation(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y));
            D3DXMatrixTransformation2D(&transformationMatrix, NULL, 0, &scaling, NULL, NULL, &translation);

            pDev->SetTransform(D3DTS_WORLD, &(preTranslationMatrix * transformationMatrix));
            pDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * 4, 2);
        }
    }


    void DisableHighlightCursor(IDirect3DDevice9* const pDev) {
        if (vertexBuffer) {
            vertexBuffer->Release();
            vertexBuffer = nullptr;
        }
        if (highlightTexture) {
            highlightTexture->Release();
            highlightTexture = nullptr;
        }
        if (highlightImage) {
            highlightImage = nullptr;
        }
    }

    void EnableHighlightCursor(CursorHighlightType type, int color, HMODULE hModule, HWND hWnd, IDirect3DDevice9* const pDev) {
        if (!highlightImage) {
            UINT size;
            switch (type) {
            case CursorHighlightType::CursorArrowsHighlight:
                vertexCount = 4;
                loopMaxDuration = 1e9ns; // 1s
                highlightImage = LoadEmbeddedResource(hModule, MAKEINTRESOURCE(IDB_PNGARROW), L"PNG", &size);
                break;
            case CursorHighlightType::CursorCirclesInHighlight:
                vertexCount = 12;
                loopMaxDuration = 1.1e9ns; // 1.1s
                highlightImage = LoadEmbeddedResource(hModule, MAKEINTRESOURCE(IDB_PNGCIRCLE), L"PNG", &size);
                break;
            case CursorHighlightType::CursorCirclesOutHighlight:
                vertexCount = 12;
                loopMaxDuration = 1.1e9ns; // 1.1s
                highlightImage = LoadEmbeddedResource(hModule, MAKEINTRESOURCE(IDB_PNGCIRCLE), L"PNG", &size);
                break;
            }
            highlightType = type;
            if (D3DXCreateTextureFromFileInMemory(pDev, highlightImage, size, &highlightTexture)) {
                highlightTexture = nullptr;
                highlightImage = nullptr;
                return;
            }
        }
        if (!vertexBuffer) {
            if (pDev->CreateVertexBuffer(vertexCount * sizeof(Vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, VertexShaderFvf, D3DPOOL_DEFAULT, &vertexBuffer, NULL) < 0) {
                vertexBuffer = nullptr;
                return;
            }
        }
        highlightColor = color;
        GetClientRect(hWnd, &clientRect);
    }

    bool IsCursorHighlighted() {
        const auto loopDuration = chrono::steady_clock::now() - loopStart;
        return loopDuration < loopMaxDuration;
    }

    void RenderHighlightCursor(IDirect3DDevice9* const pDev) {
        if (!IsCursorHighlighted()) {
            // We are outside the loop, no need to render
            return;
        }

        switch (highlightType) {
        case CursorHighlightType::CursorArrowsHighlight:
            RenderArrows(pDev);
            break;
        case CursorHighlightType::CursorCirclesInHighlight:
            RenderCirclesIn(pDev);
            break;
        case CursorHighlightType::CursorCirclesOutHighlight:
            RenderCirclesOut(pDev);
            break;
        }
    }

    void TriggerHighlightCursor() {
        if (IsCursorHighlighted()) {
            // We are inside the loop, no need to reset
            return;
        }

        loopStart = chrono::steady_clock::now();
        GetCursorPos(&cursorPos);
    }

    void UpdateCursorPos(POINT pos) {
        cursorPos = pos;
    }

}
