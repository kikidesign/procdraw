#include "stdafx.h"
#include "d3d11graphics.h"
#include "color.h"
#include "win_util.h"
#include <d3dcompiler.h>

namespace procdraw {

    D3D11Graphics::D3D11Graphics(HWND hWnd) :
        hWnd_(hWnd),
        d3dFeatureLevel_(D3D_FEATURE_LEVEL_10_0)
    {
        InitD3D();
        CreateVertexShader();
        CreatePixelShader();

        // Constant Buffer
        DirectX::XMStoreFloat4x4(&cbData_.WorldViewProjection, DirectX::XMMatrixIdentity());
        CreateConstantBuffer();

        CreateTriangleVertexBuffer();
        CreateTetrahedronVertexBuffer();

        InitViewProjectionMatrix();
        ResetMatrix();
    }

    D3D11Graphics::~D3D11Graphics()
    {
        d3dContext_->ClearState();
    }

    void D3D11Graphics::Background(float h, float s, float v)
    {
        float r, g, b;
        Hsv2Rgb(h, s, v, r, g, b);
        FLOAT c[4] = { r, g, b, 1.0f };
        d3dContext_->ClearDepthStencilView(depthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        d3dContext_->ClearRenderTargetView(renderTargetView_, c);
        ResetMatrix();
    }

    void D3D11Graphics::Present()
    {
        swapChain_->Present(0, 0);
    }

    void D3D11Graphics::Triangle()
    {
        UpdateConstantBuffer();

        d3dContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        UINT stride = sizeof(ShaderVertex);
        UINT offset = 0;
        d3dContext_->IASetVertexBuffers(0, 1, &triangleVertexBuffer_.GetInterfacePtr(), &stride, &offset);

        d3dContext_->Draw(3, 0);
    }

    void D3D11Graphics::Tetrahedron()
    {
        UpdateConstantBuffer();

        d3dContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        UINT stride = sizeof(ShaderVertex);
        UINT offset = 0;
        d3dContext_->IASetVertexBuffers(0, 1, &tetrahedronVertexBuffer_.GetInterfacePtr(), &stride, &offset);

        d3dContext_->Draw(12, 0);
    }

    void D3D11Graphics::RotateX(float angle)
    {
        auto rotationMatrix = DirectX::XMMatrixRotationX(angle);
        auto matrix = DirectX::XMLoadFloat4x4(&matrix_);
        DirectX::XMStoreFloat4x4(&matrix_, rotationMatrix * matrix);
    }

    void D3D11Graphics::RotateY(float angle)
    {
        auto rotationMatrix = DirectX::XMMatrixRotationY(angle);
        auto matrix = DirectX::XMLoadFloat4x4(&matrix_);
        DirectX::XMStoreFloat4x4(&matrix_, rotationMatrix * matrix);
    }

    void D3D11Graphics::RotateZ(float angle)
    {
        auto rotationMatrix = DirectX::XMMatrixRotationZ(angle);
        auto matrix = DirectX::XMLoadFloat4x4(&matrix_);
        DirectX::XMStoreFloat4x4(&matrix_, rotationMatrix * matrix);
    }

    void D3D11Graphics::InitD3D()
    {
        // Device, context, and swap chain

        RECT rc;
        GetClientRect(hWnd_, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_HARDWARE;

        D3D_FEATURE_LEVEL feature_levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT num_feature_levels = ARRAYSIZE(feature_levels);

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd_;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, driver_type, nullptr,
            createDeviceFlags, feature_levels, num_feature_levels,
            D3D11_SDK_VERSION, &sd, &swapChain_, &d3dDevice_,
            &d3dFeatureLevel_, &d3dContext_);
        if (hr == E_INVALIDARG) {
            // Try again without D3D_FEATURE_LEVEL_11_1
            hr = D3D11CreateDeviceAndSwapChain(nullptr, driver_type, nullptr,
                createDeviceFlags, &feature_levels[1], num_feature_levels - 1,
                D3D11_SDK_VERSION, &sd, &swapChain_, &d3dDevice_,
                &d3dFeatureLevel_, &d3dContext_);
        }
        ThrowOnFail(hr);

        // Render Target View

        ThrowOnFail(swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
            reinterpret_cast<void**>(&backBuffer_)));

        ThrowOnFail(d3dDevice_->CreateRenderTargetView(backBuffer_, nullptr, &renderTargetView_));

        // Depth Stencil View

        D3D11_TEXTURE2D_DESC depthStencilDesc;
        ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
        depthStencilDesc.Width = width;
        depthStencilDesc.Height = height;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesc.CPUAccessFlags = 0;
        depthStencilDesc.MiscFlags = 0;
        ThrowOnFail(d3dDevice_->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer_));

        ThrowOnFail(d3dDevice_->CreateDepthStencilView(depthStencilBuffer_, nullptr, &depthStencilView_));

        // Configure Output-Merger

        d3dContext_->OMSetRenderTargets(1, &renderTargetView_.GetInterfacePtr(), depthStencilView_);

        // Viewport

        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        d3dContext_->RSSetViewports(1, &vp);
    }

    ID3D10BlobPtr D3D11Graphics::CompileShaderFromFile(_In_ LPCWSTR pFileName,
        _In_ LPCSTR pEntrypoint, _In_ LPCSTR pTarget)
    {
        ID3D10BlobPtr compiledShader;
        ID3D10BlobPtr errorMessages;
        HRESULT hr = D3DCompileFromFile(pFileName, nullptr, nullptr,
            pEntrypoint, pTarget, 0, 0, &compiledShader, &errorMessages);
        if (errorMessages != nullptr) {
            throw std::runtime_error(static_cast<char*>(errorMessages->GetBufferPointer()));
        }
        ThrowOnFail(hr);
        return compiledShader;
    }

    void D3D11Graphics::CreateVertexShader()
    {
        // Create shader

        ID3D10BlobPtr vs = CompileShaderFromFile(L"shaders\\shaders1.hlsl", "vertex_shader", "vs_4_0");
        ThrowOnFail(d3dDevice_->CreateVertexShader(vs->GetBufferPointer(),
            vs->GetBufferSize(), nullptr, &vertexShader_));
        d3dContext_->VSSetShader(vertexShader_, 0, 0);

        // Input layout

        D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        ThrowOnFail(d3dDevice_->CreateInputLayout(inputElementDescriptions,
            ARRAYSIZE(inputElementDescriptions), vs->GetBufferPointer(),
            vs->GetBufferSize(), &inputLayout_));
        d3dContext_->IASetInputLayout(inputLayout_);
    }

    void D3D11Graphics::CreatePixelShader()
    {
        ID3D10BlobPtr ps = CompileShaderFromFile(L"shaders\\shaders1.hlsl", "pixel_shader", "ps_4_0");
        ThrowOnFail(d3dDevice_->CreatePixelShader(ps->GetBufferPointer(),
            ps->GetBufferSize(), nullptr, &pixelShader_));
        d3dContext_->PSSetShader(pixelShader_, 0, 0);
    }

    void D3D11Graphics::CreateConstantBuffer()
    {
        D3D11_BUFFER_DESC cbDesc;
        ZeroMemory(&cbDesc, sizeof(cbDesc));
        cbDesc.ByteWidth = sizeof(CBufferPerObject);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA cbSubresourceData;
        ZeroMemory(&cbSubresourceData, sizeof(cbSubresourceData));
        cbSubresourceData.pSysMem = &cbData_;

        ThrowOnFail(d3dDevice_->CreateBuffer(&cbDesc, &cbSubresourceData, &constantBuffer_));

        d3dContext_->VSSetConstantBuffers(0, 1, &constantBuffer_.GetInterfacePtr());
    }

    ID3D11BufferPtr D3D11Graphics::CreateVertexBuffer(ShaderVertex *vertices, int numVertices)
    {
        D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
        vertexBufferDesc.ByteWidth = sizeof(ShaderVertex) * numVertices;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData;
        ZeroMemory(&vertexSubresourceData, sizeof(vertexSubresourceData));
        vertexSubresourceData.pSysMem = vertices;

        ID3D11BufferPtr vertexBuffer;
        ThrowOnFail(d3dDevice_->CreateBuffer(&vertexBufferDesc,
            &vertexSubresourceData, &vertexBuffer));

        return vertexBuffer;
    }

    void D3D11Graphics::CreateTriangleVertexBuffer()
    {
        ShaderVertex vertices[] =
        {
            ShaderVertex(DirectX::XMFLOAT4(-1.0f, 1.0f, 0.0f, 1.0f),
                DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)),
            ShaderVertex(DirectX::XMFLOAT4(1.0f, -1.0f, 0.0f, 1.0f),
                DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)),
            ShaderVertex(DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f),
                DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f))
        };

        triangleVertexBuffer_ = CreateVertexBuffer(vertices, ARRAYSIZE(vertices));
    }

    void D3D11Graphics::CreateTetrahedronVertexBuffer()
    {
        auto vertex1 = DirectX::XMFLOAT4(1, 0, -M_SQRT1_2, 1);
        auto vertex2 = DirectX::XMFLOAT4(0, 1, M_SQRT1_2, 1);
        auto vertex3 = DirectX::XMFLOAT4(0, -1, M_SQRT1_2, 1);
        auto vertex4 = DirectX::XMFLOAT4(-1, 0, -M_SQRT1_2, 1);

        auto red = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        auto green = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
        auto blue = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
        auto yellow = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

        ShaderVertex vertices[] =
        {
            ShaderVertex(vertex1, red),
            ShaderVertex(vertex4, red),
            ShaderVertex(vertex2, red),
            ShaderVertex(vertex1, blue),
            ShaderVertex(vertex3, blue),
            ShaderVertex(vertex4, blue),
            ShaderVertex(vertex1, green),
            ShaderVertex(vertex2, green),
            ShaderVertex(vertex3, green),
            ShaderVertex(vertex2, yellow),
            ShaderVertex(vertex4, yellow),
            ShaderVertex(vertex3, yellow)
        };

        tetrahedronVertexBuffer_ = CreateVertexBuffer(vertices, ARRAYSIZE(vertices));
    }

    void D3D11Graphics::InitViewProjectionMatrix()
    {
        // TODO move the View-Projection matrix to its own 'camera' class

        // View
        auto eye = DirectX::XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);
        auto at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        auto up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        auto viewMatrix = DirectX::XMMatrixLookAtLH(eye, at, up);

        // Projection
        // TODO angle of view?
        auto projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, 1.0f, 1.0f, 100.0f);

        // View-Projection
        DirectX::XMStoreFloat4x4(&viewProjectionMatrix_, viewMatrix * projectionMatrix);
    }

    void D3D11Graphics::ResetMatrix()
    {
        matrix_ = viewProjectionMatrix_;
    }

    void D3D11Graphics::UpdateConstantBuffer()
    {
        // TODO remove matrix_ and just use cbData_.WorldViewProjection?
        cbData_.WorldViewProjection = matrix_;

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ZeroMemory(&mappedResource, sizeof(mappedResource));

        ThrowOnFail(d3dContext_->Map(constantBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        memcpy(mappedResource.pData, &cbData_, sizeof(cbData_));
        d3dContext_->Unmap(constantBuffer_, 0);
    }

}
