#pragma once
#include <d3d11.h>

/**
 * Renderer class takes care of initializing the graphics (and shaders), rendering, 
 * and cleaning up before destroying.
**/
class Renderer
{
private:
    IDXGISwapChain* _swapChain;             // the pointer to the swap chain interface

    ID3D11Device* _device;                    // the pointer to our Direct3D device interface
    ID3D11DeviceContext* _deviceContext;     // the pointer to our Direct3D device context
    ID3D11RenderTargetView* _backBuffer;    // render target

    ID3D11VertexShader* _vertexShader;    // pointer to the vertex shader COM object
    ID3D11PixelShader* _pixelShader;     // pointer to the pixel shader COM object

    ID3D11Buffer* _indexBuffer;         // Index buffer for re-using vertices
    ID3D11Buffer* _vertexBuffer;        // Vertex buffer

    ID3D11InputLayout* _inputLayout;    // Feed vertex from vertex buffer to the vertex shader

    void initShaders();
    void initScene();
public:
    Renderer();
    ~Renderer();

    void initD3D(HWND hWindow);

    void renderFrame();
};

