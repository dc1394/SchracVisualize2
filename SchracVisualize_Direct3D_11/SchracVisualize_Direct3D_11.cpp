//--------------------------------------------------------------------------------------
// File: Tutorial08.cpp
//
// Basic introduction to DXUT
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsdlg.h"
#include "SDKmesh.h"
#include "SDKmisc.h"
#include "orbitaldensityrand/orbitaldensityrand.h"
#include "orbitaldensityrand/utility/utility.h"
#include <array>					                // for std::array
#include <numeric>                                  // for std::iota
#include <optional>                                 // for std::optional
#include <boost/cast.hpp>                           // for boost::numeric_cast
#include <boost/format.hpp>			                // for boost::wformat
#include <tbb/task_scheduler_init.h>                // for tbb::task_scheduler_init
#include <wrl.h>					                // for Microsoft::WRL::ComPtr

#pragma warning( disable : 4100 )

using namespace DirectX;
using namespace orbitaldensityrand;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

struct CBChangesEveryFrame
{
	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProjection;
};

struct CBChangesEveryFrame2
{
    XMFLOAT4X4 mWorldViewProj;
    XMFLOAT4X4 mWorld;
    XMFLOAT4 vMeshColor;
};

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

//! A global variable (constant).
/*!
    ?????????
*/
static auto constexpr MAGNIFICATION = 1.2f;

//! A global variable (constant).
/*!
    ?????(??)
*/
static auto constexpr WINDOWHEIGHT = 960;

//! A global variable (constant).
/*!
    ?????(?)
*/
static auto constexpr WINDOWWIDTH = 1280;

//! A global variable.
/*!
    CPU??????
*/
static auto const cputhreads = tbb::task_scheduler_init::default_num_threads();

//! A global variable.
/*!
    A model viewing camera
*/
CModelViewerCamera camera;

//! A lambda expression.
/*!
    CDXUTTextHelper???????????????
    \param spline CDXUTTextHelper??????
*/
static auto const deleter = [](auto ptxthelper) {
    if (ptxthelper) {
        delete ptxthelper;
        ptxthelper = nullptr;
    }
};

//! A global variable.
/*!
    manager for shared resources of dialogs
*/
CDXUTDialogResourceManager dialogResourceManager;

//! A global variable.
/*!
    ???????????
*/
auto drawdata = 1U;

//! A global variable.
/*!
    ??????
*/
double drawstarttime;

//! A global variable.
/*!
    ??????
*/
double drawendtime;

//! A global variable.
/*!
    ???????????????
*/
auto first = true;

//! A global variable.
/*!
    ???????????????
*/
auto end = false;

//! A global variable.
/*!
    ?????????
*/
D3D11_BUFFER_DESC g_bd;

//! A global variable.
/*!
    Direct3D????
*/
ID3D11Device* g_pd3dDevice;

//! A global variable.
/*!
    manages the 3D UI
*/
CDXUTDialog hud;

//! A global variable.
/*!
*/
Microsoft::WRL::ComPtr<ID3D11Buffer> pCBChangesEveryFrame;

//! A global variable.
/*!
*/
Microsoft::WRL::ComPtr<ID3D11Buffer> pCBNeverChanges;

//! A global variable.
/*!
    ?????????
*/
std::shared_ptr<getdata::GetData> pgd;

//! A global variable.
/*!
    ??????????
*/
Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;

//! A global variable.
/*!
    ??????????????????????
*/
std::optional<OrbitalDensityRand> podr;

//! A global variable.
/*!
    ?????????
*/
Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShaderBox;

//! A global variable.
/*!
    ???????
*/
std::unique_ptr<CDXUTTextHelper, decltype(deleter)> pTxtHelper(nullptr, deleter);

//! A global variable.
/*!
    ??????
*/
Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;

//! A global variable.
/*!
*/
Microsoft::WRL::ComPtr<ID3D11InputLayout> pVertexLayout;

//! A global variable.
/*!
    ???????????
*/
Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShaderBox;

//! A global variable.
/*!
    ???????????????
*/
auto reim = OrbitalDensityRand::Re_Im_type::REAL;

//! A global variable.
/*!
    Device settings dialog
*/
CD3DSettingsDlg settingsDlg;

//! A global variable.
/*!
    dialog for specific controls
*/
CDXUTDialog ui;


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_CHANGEDEVICE        2
#define IDC_TOGGLEROTATION      3
#define IDC_REDRAW              4
#define IDC_LOADNEWFILE         5
#define IDC_COMBOBOX            6
#define IDC_RADIOA              7
#define IDC_RADIOB              8
#define IDC_OUTPUT              9
#define IDC_SLIDER				10

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);

//! A function.
/*!
    ??????????????
    \return ?????????
*/
std::wstring CreateWindowTitle();

//! A function.
/*!
    Initialize the app
*/
void InitApp();

//! A function.
/*!
    ????
    \param pd3dImmediateContext Direct3D???????????
*/
HRESULT OnRender(ID3D11DeviceContext* pd3dImmediateContext);

//! A function.
/*!
    ??????????????????
*/
void ReadData();

//! A function.
/*!
    ?????
    \param pd3dDevice Direct3D????
*/
void Redraw();

//! A function.
/*!
    ???????true???
*/
void RedrawFlagTrue();

//! A function.
/*!
    ?????????????
*/
void RenderText(double fTime);

//! A function.
/*!
	??????
	\param pd3dDevice Direct3D????
*/
HRESULT RenderPoint();

//! A function.
/*!
    ????????????
*/
void SetCamera();

//! A function.
/*!
    UI?????
*/
void SetUI();

//! A function.
/*!
    ???????
*/
void StopDraw();

//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void InitApp()
{
	settingsDlg.Init(&dialogResourceManager);
	hud.Init(&dialogResourceManager);
	ui.Init(&dialogResourceManager);

    ReadData();
    podr.emplace(pgd);
    SetUI();
}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	bool* pbNoFurtherProcessing, void* pUserContext)
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = dialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	// Pass messages to settings dialog if its active
	if (settingsDlg.IsActive())
	{
		settingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = hud.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;
	*pbNoFurtherProcessing = ui.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	// Pass all remaining windows messages to camera so it can respond to user input
	camera.HandleMessages(hWnd, uMsg, wParam, lParam);

	return 0;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
    auto hr = S_OK;

    g_pd3dDevice = pd3dDevice;

    auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(dialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
	V_RETURN(settingsDlg.OnD3D11CreateDevice(pd3dDevice));
	pTxtHelper.reset(new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &dialogResourceManager, 15));

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // Compile the vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> pVSBlob;
    V_RETURN( DXUTCompileFromFile( L"SchracVisualize_Direct3D_11.fx", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, pVSBlob.GetAddressOf() ) );

    // Create the vertex shader
    hr = pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, pVertexShaderBox.GetAddressOf() );
    if (FAILED(hr)) {    
        pVSBlob.Reset();
        return hr;
    }

	// Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0,
        "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
    };

    auto const numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), pVertexLayout.GetAddressOf());

    // Set the input layout
    pd3dImmediateContext->IASetInputLayout( pVertexLayout.Get() );

	// Compile the pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
	V_RETURN(DXUTCompileFromFile(L"SchracVisualize_Direct3D_11.fx", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, pPSBlob.GetAddressOf()));

	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pPixelShaderBox.GetAddressOf());
	pPSBlob.Reset();

    Redraw();

	// Set vertex buffer
	UINT const stride = sizeof(SimpleVertex);
	auto const offset = 0U;
	pd3dImmediateContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);

    // Set index buffer
    pd3dImmediateContext->IASetIndexBuffer( pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0 );

    // Set primitive topology
    pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // Create the constant buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.ByteWidth = sizeof(CBChangesEveryFrame);
    V_RETURN( pd3dDevice->CreateBuffer( &bd, nullptr, pCBChangesEveryFrame.GetAddressOf() ) );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
    StopDraw();
	dialogResourceManager.OnD3D11DestroyDevice();
	settingsDlg.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	pVertexShaderBox.Reset();
	pVertexLayout.Reset();
	pVertexBuffer.Reset();
	pPixelShaderBox.Reset();
	pIndexBuffer.Reset();
	pCBNeverChanges.Reset();
	pCBChangesEveryFrame.Reset();
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
	double fTime, float fElapsedTime, void* pUserContext)
{
	//using namespace moleculardynamics;

	// If the settings dialog is being shown, then render it instead of rendering the app's scene
	if (settingsDlg.IsActive())
	{
		settingsDlg.OnRender(fElapsedTime);
		return;
	}

	RenderPoint();

    //
	// Clear the back buffer
	//
	pd3dImmediateContext->ClearRenderTargetView(DXUTGetD3D11RenderTargetView(), Colors::Black);

	//
	// Clear the depth stencil
	//
	pd3dImmediateContext->ClearDepthStencilView(DXUTGetD3D11DepthStencilView(), D3D11_CLEAR_DEPTH, 1.0, 0);

	// Get the projection & view matrix from the camera class
	auto const mProj = camera.GetProjMatrix();
	auto const mView = camera.GetViewMatrix();

	// Set the per object constant data
	auto const mWorld = camera.GetWorldMatrix();

	// Update constant buffer that changes once per frame
	D3D11_MAPPED_SUBRESOURCE MappedResource;
    auto const hr = pd3dImmediateContext->Map(pCBChangesEveryFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	auto pCB = reinterpret_cast<CBChangesEveryFrame*>(MappedResource.pData);
	XMStoreFloat4x4(&pCB->mWorld, XMMatrixTranspose(mWorld));
	XMStoreFloat4x4(&pCB->mView, XMMatrixTranspose(mView));
	XMStoreFloat4x4(&pCB->mProjection, XMMatrixTranspose(mProj));
	pd3dImmediateContext->Unmap(pCBChangesEveryFrame.Get(), 0);

	// Set vertex buffer
	UINT const stride = sizeof(SimpleVertex);
	auto const offset = 0U;
	pd3dImmediateContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);

	// Set index buffer
	pd3dImmediateContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//
	// Render the cube
	//
	pd3dImmediateContext->VSSetShader(pVertexShaderBox.Get(), nullptr, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, pCBChangesEveryFrame.GetAddressOf());
	pd3dImmediateContext->PSSetShader(pPixelShaderBox.Get(), nullptr, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, pCBChangesEveryFrame.GetAddressOf());
	pd3dImmediateContext->DrawIndexed(static_cast<UINT>(podr->Verticessize), 0, 0);

    OnRender(pd3dImmediateContext);

	hud.OnRender(fElapsedTime);
	ui.OnRender(fElapsedTime);
	RenderText(fTime);
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	dialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	auto hr = S_OK;

	V_RETURN(dialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(settingsDlg.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	// Setup the camera's projection parameters
	auto const fAspectRatio = pBackBufferSurfaceDesc->Width / static_cast<FLOAT>(pBackBufferSurfaceDesc->Height);
	camera.SetProjParams(XM_PI / 4, fAspectRatio, 2.0f, 4000.0f);
	camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
	camera.SetButtonMasks(MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON);

	hud.SetLocation(static_cast<std::int32_t>(pBackBufferSurfaceDesc->Width) - 170, 0);
	hud.SetSize(170, 170);
	ui.SetLocation(static_cast<std::int32_t>(pBackBufferSurfaceDesc->Width) - 170, static_cast<std::int32_t>(pBackBufferSurfaceDesc->Height) - 300);
	ui.SetSize(170, 300);

    return hr;
}


//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved(void* pUserContext)
{
	return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	// Update the camera's position based on user input 
	camera.FrameMove(fElapsedTime);
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	switch (nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen();
		break;

	case IDC_CHANGEDEVICE:
		settingsDlg.SetActive(!settingsDlg.IsActive());
		break;

    case IDC_REDRAW:
        RedrawFlagTrue();
        Redraw();
        break;

    case IDC_LOADNEWFILE:
        StopDraw();
        ReadData();
        podr.emplace(pgd);
        first = true;
        ::SetWindowText(DXUTGetHWND(), CreateWindowTitle().c_str());
        Redraw();
        break;

    case IDC_COMBOBOX:
    {
        auto const pItem = (static_cast<CDXUTComboBox*>(pControl))->GetSelectedItem();
        if (pItem)
        {
            drawdata = reinterpret_cast<std::uint32_t>(pItem->pData);
            RedrawFlagTrue();
        }
        break;
    }

    case IDC_RADIOA:
        reim = OrbitalDensityRand::Re_Im_type::REAL;
        RedrawFlagTrue();
        Redraw();
        break;

    case IDC_RADIOB:
        reim = OrbitalDensityRand::Re_Im_type::IMAGINARY;
        RedrawFlagTrue();
        Redraw();
        break;

    case IDC_SLIDER:
        RedrawFlagTrue();
        podr->Verticessize(static_cast<std::vector<SimpleVertex>::size_type>((reinterpret_cast<CDXUTSlider*>(pControl))->GetValue()));
        Redraw();
        break;

	default:
		BOOST_ASSERT(!"???????!");
		break;
	}
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1: // Change as needed                
                break;

            default:
                break;
        }
    }
}

HRESULT OnRender(ID3D11DeviceContext* pd3dImmediateContext)
{
    auto hr = S_OK;

    // Get the projection & view matrix from the camera class
    auto const mProj = camera.GetProjMatrix();
    auto const mView = camera.GetViewMatrix();
    auto const mworld = camera.GetWorldMatrix();

    auto const mWorldViewProjection = mworld * mView * mProj;

    // Update constant buffer that changes once per frame
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V(pd3dImmediateContext->Map(pCBChangesEveryFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
    auto pCB = reinterpret_cast<CBChangesEveryFrame2*>(MappedResource.pData);
    XMStoreFloat4x4(&pCB->mWorldViewProj, XMMatrixTranspose(mWorldViewProjection));
    XMStoreFloat4x4(&pCB->mWorld, XMMatrixTranspose(mworld));

    pd3dImmediateContext->Unmap(pCBChangesEveryFrame.Get(), 0);

    //
    // Set the Vertex Layout
    //
    pd3dImmediateContext->IASetInputLayout(pVertexLayout.Get());

    pd3dImmediateContext->VSSetConstantBuffers(0, 1, pCBNeverChanges.GetAddressOf());
    pd3dImmediateContext->VSSetConstantBuffers(1, 1, pCBChangesEveryFrame.GetAddressOf());
    pd3dImmediateContext->PSSetConstantBuffers(1, 1, pCBChangesEveryFrame.GetAddressOf());

    return hr;
}

std::wstring CreateWindowTitle()
{
    std::string windowtitle;
    switch (pgd->Rho_wf_type_) {
    case getdata::GetData::Rho_Wf_type::RHO:
        windowtitle = "Electron density";
        break;

    case getdata::GetData::Rho_Wf_type::WF:
        windowtitle = "Wavefunction";
        break;
    }
    windowtitle += " in " + pgd->Atomname() + " for " + pgd->Orbital() + " orbital";

    return utility::my_mbstowcs(windowtitle);
}

HRESULT RenderPoint()
{
    auto hr = S_OK;

    auto const index = drawdata & 0x0F;
        switch (pgd->L) {
        case 0:
            (*podr)(0, reim);
            break;

        case 1:
        {
            switch (index) {
            case 1:
                (*podr)(1, reim);
                break;

            case 2:
                (*podr)(-1, reim);
                break;

            case 3:
                (*podr)(0, reim);
                break;

            default:
                BOOST_ASSERT(!"index????????!");
                break;
            }
        }
        break;

        case 2:
        {
            switch (index) {
            case 1:
                (*podr)(-2, reim);
                break;

            case 2:
                (*podr)(-1, reim);
                break;

            case 3:
                (*podr)(1, reim);
                break;

            case 4:
                (*podr)(2, reim);
                break;

            case 5:
                (*podr)(0, reim);
                break;

            default:
                BOOST_ASSERT(!"index????????!");
                break;
            }
        }
        break;

        case 3:
        {
            switch (index) {
            case 1:
                (*podr)(1, reim);
                break;

            case 2:
                (*podr)(-1, reim);
                break;

            case 3:
                (*podr)(2, reim);
                break;

            case 4:
                (*podr)(-2, reim);
                break;

            case 5:
                (*podr)(3, reim);
                break;

            case 6:
                (*podr)(-3, reim);
                break;

            case 7:
                (*podr)(0, reim);
                break;

            default:
                BOOST_ASSERT(!"index????????!");
                break;
            }
        }
        break;

        default:
            BOOST_ASSERT(!"???????????!");
            break;
        }

	// Create vertex buffer
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = podr->Vertices().data();
	V_RETURN(g_pd3dDevice->CreateBuffer(&g_bd, &InitData, pVertexBuffer.ReleaseAndGetAddressOf()));

	return hr;
}

void ReadData()
{
    while (true) {
        try {
            pgd.reset();
            pgd = std::make_shared<getdata::GetData>(utility::myOpenFile());
        }
        catch (std::runtime_error const & e) {
            ::MessageBox(nullptr, utility::my_mbstowcs(e.what()).c_str(), L"???", MB_OK | MB_ICONWARNING);
            continue;
        }
        break;
    }
}

//! A function.
/*!
    ?????
*/
void Redraw()
{
    ZeroMemory(&g_bd, sizeof(g_bd));
    g_bd.Usage = D3D11_USAGE_DEFAULT;
    g_bd.ByteWidth = static_cast<UINT>(sizeof(SimpleVertex) * podr->Verticessize);
    g_bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    g_bd.CPUAccessFlags = 0;

    // Create index buffer

    static std::vector<std::int32_t> indices;
    indices.resize(podr->Verticessize);
    std::iota(indices.begin(), indices.end(), 0);

    // ?????????
    D3D11_BUFFER_DESC bd;

    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = static_cast<UINT>(sizeof(DWORD) * podr->Verticessize);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = indices.data();
    g_pd3dDevice->CreateBuffer(&bd, &InitData, &pIndexBuffer);

    SetCamera();
}

void RedrawFlagTrue()
{
    StopDraw();
    podr->Thread_end = false;
    podr->Redraw = true;
    first = true;
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text.
//--------------------------------------------------------------------------------------
void RenderText(double fTime)
{
    if (!podr->Complete && first) {
        drawstarttime = fTime;
        end = false;
        first = false;
    }
    else if (podr->Complete && !end) {
        drawendtime = fTime - drawstarttime;
        end = true;
    }

    std::wstring str;
    double calctime;
    if (end) {
        calctime = drawendtime;
    }
    else {
        calctime = fTime - drawstarttime;
    }

	pTxtHelper->Begin();
	pTxtHelper->SetInsertionPos(5, 5);
	pTxtHelper->SetForegroundColor(Colors::Yellow);
	pTxtHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
	pTxtHelper->DrawTextLine(DXUTGetDeviceStats());
    pTxtHelper->DrawTextLine((boost::wformat(L"CPU threads: %d") % cputhreads).str().c_str());
    pTxtHelper->DrawTextLine((boost::wformat(L"Total vertices = %d") % podr->Verticessize()).str().c_str());
    pTxtHelper->DrawTextLine((boost::wformat(L"Calculation time = %.3f(sec)") % calctime).str().c_str());

	pTxtHelper->End();
}

void SetCamera()
{
    // Initialize the view matrix
    auto const pos = static_cast<float>(podr->Rmax) * MAGNIFICATION;
    // Setup the camera's view parameters
    XMVECTORF32 s_vecEye = { 0.0f, pos, -pos, 0.0f };
    XMVECTORF32 s_vecAt = { 0.0f, 0.0f, 0.0f, 0.0f };
    camera.SetViewParams(s_vecEye, s_vecAt);
}

void SetUI()
{
	hud.SetCallback(OnGUIEvent);
	
	auto iY = 10;
	hud.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22);
	hud.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += 26, 170, 22, VK_F2);

    hud.AddButton(IDC_REDRAW, L"Redraw", 0, iY += 34, 170, 22);
    hud.AddButton(IDC_LOADNEWFILE, L"Load new file", 0, iY += 24, 170, 22);

    // Combobox
    CDXUTComboBox* pCombo;
    hud.AddComboBox(IDC_COMBOBOX, 35, iY += 34, 125, 22, L'O', false, &pCombo);
    if (pCombo)
    {
        pCombo->SetDropHeight(100);
        pCombo->RemoveAllItems();
        BOOST_ASSERT(pgd->N > static_cast<std::int32_t>(pgd->L));
        auto orbital(std::to_wstring(pgd->N));
        switch (pgd->L) {
        case 0:
        {
            orbital += L's';
            pCombo->AddItem(orbital.c_str(), reinterpret_cast<LPVOID>(0x11111111));
        }
        break;

        case 1:
        {
            pCombo->AddItem((orbital + L"px").c_str(), reinterpret_cast<LPVOID>(0x11111111));
            pCombo->AddItem((orbital + L"py").c_str(), reinterpret_cast<LPVOID>(0x12121212));
            pCombo->AddItem((orbital + L"pz").c_str(), reinterpret_cast<LPVOID>(0x13131313));
        }
        break;

        case 2:
        {
            pCombo->AddItem((orbital + L"dxy").c_str(), reinterpret_cast<LPVOID>(0x11111111));
            pCombo->AddItem((orbital + L"dyz").c_str(), reinterpret_cast<LPVOID>(0x12121212));
            pCombo->AddItem((orbital + L"dzx").c_str(), reinterpret_cast<LPVOID>(0x13131313));
            pCombo->AddItem((orbital + L"dx^2-y^2").c_str(), reinterpret_cast<LPVOID>(0x14141414));
            pCombo->AddItem((orbital + L"dz^2").c_str(), reinterpret_cast<LPVOID>(0x15151515));
        }
        break;

        case 3:
        {
            pCombo->AddItem((orbital + L"fxz^2").c_str(), reinterpret_cast<LPVOID>(0x11111111));
            pCombo->AddItem((orbital + L"fyz^2").c_str(), reinterpret_cast<LPVOID>(0x12121212));
            pCombo->AddItem((orbital + L"fz(x^2-y^2)").c_str(), reinterpret_cast<LPVOID>(0x13131313));
            pCombo->AddItem((orbital + L"fxyz").c_str(), reinterpret_cast<LPVOID>(0x14141414));
            pCombo->AddItem((orbital + L"fx(x^2-3y^2)").c_str(), reinterpret_cast<LPVOID>(0x15151515));
            pCombo->AddItem((orbital + L"fy(3x^2-y^2)").c_str(), reinterpret_cast<LPVOID>(0x16161616));
            pCombo->AddItem((orbital + L"fz^2").c_str(), reinterpret_cast<LPVOID>(0x17171717));
        }
        break;

        default:
            throw std::runtime_error("g???????????????");
            break;
        }

        if (pgd->Rho_wf_type_ == getdata::GetData::Rho_Wf_type::WF) {
            // Radio buttons
            hud.AddRadioButton(IDC_RADIOA, 1, L"Real part", 35, iY += 34, 125, 22, true, L'1');
            hud.AddRadioButton(IDC_RADIOB, 1, L"Imaginary part", 35, iY += 28, 125, 22, false, L'2');
        }
    }

    // ?????
    hud.AddStatic(IDC_OUTPUT, L"Vertices size", 20, iY += 34, 125, 22);
    hud.GetStatic(IDC_OUTPUT)->SetTextColor(D3DCOLOR_ARGB(255, 255, 255, 255));
    hud.AddSlider(IDC_SLIDER, 35, iY += 24, 125, 22, 0, 1000000, OrbitalDensityRand::VERTICESSIZE_INIT_VALUE);

	ui.SetCallback(OnGUIEvent);
}

void StopDraw()
{
    podr->Thread_end = true;
    if (podr->Pth()->joinable()) {
        podr->Pth()->join();
    }
}

//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#ifdef _DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device
    // that is available on the system depending on which D3D callbacks are set below

    // Set general DXUT callbacks
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );

    // Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    // Perform any application-level initialization here

    DXUTInit( true, true, nullptr ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen

	InitApp();

	// ????????
	auto const dispx = ::GetSystemMetrics(SM_CXSCREEN);
	auto const dispy = ::GetSystemMetrics(SM_CYSCREEN);
	auto const xpos = (dispx - WINDOWWIDTH) / 2;
	auto const ypos = (dispy - WINDOWHEIGHT) / 2;
	DXUTCreateWindow(CreateWindowTitle().c_str(), nullptr, nullptr, nullptr, xpos, ypos);
	
    // Only require 10-level hardware or later
    DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, WINDOWWIDTH, WINDOWHEIGHT);

	// Enter into the DXUT render loop
    DXUTMainLoop();

    return DXUTGetExitCode();
}
