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
	画面サイズ（高さ）
*/
static auto constexpr WINDOWHEIGHT = 960;

//! A global variable (constant).
/*!
	画面サイズ（幅）
*/
static auto constexpr WINDOWWIDTH = 1280;

//! A global variable.
/*!
	A model viewing camera
*/
CModelViewerCamera camera;

//! A global variable.
/*!
    データオブジェクト
*/
std::shared_ptr<getdata::GetData> pgd;

//! A global variable.
/*!
    軌道・電子密度の乱数生成クラスのオブジェクト
*/
std::optional<OrbitalDensityRand> podr;

//! A lambda expression.
/*!
	CDXUTTextHelperへのポインタを解放するラムダ式
	\param spline CDXUTTextHelperへのポインタ
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
    バッファーリソース
*/
D3D11_BUFFER_DESC g_bd;

//! A global variable.
/*!
    バッファーリソース2
*/
D3D11_BUFFER_DESC g_bd2;

//! A global variable.
/*!
*/
std::vector<std::int32_t> indices;

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
	インデックスバッファ
*/
ID3D11Buffer* pIndexBuffer;

//! A global variable.
/*!
	ピクセルシェーダー
*/
Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShaderBox;

//! A global variable.
/*!
	テキスト表示用
*/
std::unique_ptr<CDXUTTextHelper, decltype(deleter)> pTxtHelper(nullptr, deleter);

//! A global variable.
/*!
	頂点バッファ
*/
Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;

//! A global variable.
/*!
*/
Microsoft::WRL::ComPtr<ID3D11InputLayout> pVertexLayout;

//! A global variable.
/*!
	バーテックスシェーダー
*/
Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShaderBox;

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

//! A global variable.
/*!
    実部と虚部のどちらを描画するか
*/
auto reim = OrbitalDensityRand::Re_Im_type::REAL;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_CHANGEDEVICE        2
#define IDC_RECALC              3
#define IDC_OUTPUT              4
#define IDC_OUTPUT2             5
#define IDC_OUTPUT3             6
#define IDC_OUTPUT4             7
#define IDC_OUTPUT5             8
#define IDC_SLIDER              9
#define IDC_SLIDER2             10
#define IDC_SLIDER3             11
#define IDC_RADIOA              12
#define IDC_RADIOB              13
#define IDC_RADIOC              14
#define IDC_RADIOD              15
#define IDC_RADIOE              16

void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);

//! A function.
/*!
    描画する
    \param pd3dImmediateContext Direct3Dのデバイスコンテキスト
*/
HRESULT OnRender(ID3D11DeviceContext* pd3dImmediateContext);

//! A function.
/*!
    初期化する
*/
HRESULT OnInit(ID3D11Device* pd3dDevice);

//! A function.
/*!
    テキストファイルからデータを読み込む
*/
void ReadData();

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
void RenderText();

//! A function.
/*!
	点を描画する
	\param pd3dDevice Direct3Dのデバイス
*/
HRESULT RenderPoint(ID3D11Device* pd3dDevice);

//! A function.
/*!
	Initialize the app 
*/
void InitApp();


void SetUI();

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
    V_RETURN( DXUTCompileFromFile( L"LJ_Argon_MD_Direct3D_11_Box.fx", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, pVSBlob.GetAddressOf() ) );

    // Create the vertex shader
    hr = pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, pVertexShaderBox.GetAddressOf() );
    if (FAILED(hr)) {    
        pVSBlob.Reset();
        return hr;
    }

	// Define the input layout
    std::array<D3D11_INPUT_ELEMENT_DESC, 3> layout =
    {
        "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0,
		"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0,
		"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0
    };

    // Set the input layout
    pd3dImmediateContext->IASetInputLayout( pVertexLayout.Get() );

	// Compile the pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
	V_RETURN(DXUTCompileFromFile(L"LJ_Argon_MD_Direct3D_11_Box.fx", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, pPSBlob.GetAddressOf()));

	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pPixelShaderBox.GetAddressOf());
	pPSBlob.Reset();

    OnInit(pd3dDevice);

	// Set vertex buffer
	UINT const stride = sizeof(SimpleVertex);
	auto const offset = 0U;
	pd3dImmediateContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);

    // Set index buffer
    pd3dImmediateContext->IASetIndexBuffer( pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

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

	// Setup the camera's view parameters
	static const XMVECTORF32 s_vecEye = { 0.0f, 5.0f, 5.0f, 0.0f };
	camera.SetViewParams(s_vecEye, g_XMZero);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	dialogResourceManager.OnD3D11DestroyDevice();
	settingsDlg.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	pVertexShaderBox.Reset();
	pVertexLayout.Reset();
	pVertexBuffer.Reset();
	pPixelShaderBox.Reset();
	SAFE_DELETE(pIndexBuffer);
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

	RenderPoint(pd3dDevice);

	//armd.runCalc();

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
	pd3dImmediateContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//
	// Render the cube
	//
	pd3dImmediateContext->VSSetShader(pVertexShaderBox.Get(), nullptr, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, pCBChangesEveryFrame.GetAddressOf());
	pd3dImmediateContext->PSSetShader(pPixelShaderBox.Get(), nullptr, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, pCBChangesEveryFrame.GetAddressOf());
	pd3dImmediateContext->DrawIndexed(podr->Vertexsize, 0, 0);

    OnRender(pd3dImmediateContext);

	hud.OnRender(fElapsedTime);
	ui.OnRender(fElapsedTime);
	RenderText();
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

	hud.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	hud.SetSize(170, 170);
	ui.SetLocation(pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300);
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
	//case IDC_RECALC:
	//	armd.recalc();
	//	break;

	//case IDC_SLIDER:
	//	armd.setTgiven(static_cast<double>((reinterpret_cast<CDXUTSlider *>(pControl))->GetValue()));
	//	break;

	//case IDC_SLIDER2:
	//	armd.setScale(static_cast<double>((reinterpret_cast<CDXUTSlider *>(pControl))->GetValue()) / LATTICERATIO);
	//	modLatconst = true;
	//	break;

	//case IDC_SLIDER3:
	//	armd.setNc(reinterpret_cast<CDXUTSlider *>(pControl)->GetValue());
	//	modNc = true;
	//	break;

	//case IDC_RADIOA:
	//	armd.setEnsemble(moleculardynamics::EnsembleType::NVT);
	//	break;

	//case IDC_RADIOB:
	//	armd.setEnsemble(moleculardynamics::EnsembleType::NVE);
	//	break;

	//case IDC_RADIOC:
	//	armd.setTempContMethod(moleculardynamics::TempControlMethod::LANGEVIN);
	//	break;

	//case IDC_RADIOD:
	//	armd.setTempContMethod(moleculardynamics::TempControlMethod::NOSE_HOOVER);
	//	break;

	//case IDC_RADIOE:
	//	armd.setTempContMethod(moleculardynamics::TempControlMethod::VELOCITY);
	//	break;

	default:
		BOOST_ASSERT(!"何かがおかしい！！");
		break;
	}
}


HRESULT OnInit(ID3D11Device* pd3dDevice)
{
    auto hr = S_OK;

    ZeroMemory(&g_bd, sizeof(g_bd));
    g_bd.Usage = D3D11_USAGE_DEFAULT;
    g_bd.ByteWidth = sizeof(SimpleVertex) * podr->Vertexsize;
    g_bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    g_bd.CPUAccessFlags = 0;

    // Create index buffer

    indices.resize(podr->Vertexsize);
    std::iota(indices.begin(), indices.end(), 0);

    ZeroMemory(&g_bd2, sizeof(g_bd2));
    g_bd2.Usage = D3D11_USAGE_DEFAULT;
    g_bd2.ByteWidth = sizeof(DWORD) * podr->Vertexsize;
    g_bd2.BindFlags = D3D11_BIND_INDEX_BUFFER;
    g_bd2.CPUAccessFlags = 0;
    g_bd2.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = indices.data();
    V_RETURN(pd3dDevice->CreateBuffer(&g_bd2, &InitData, &pIndexBuffer));

    return hr;
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


HRESULT RenderPoint(ID3D11Device* pd3dDevice)
{
    auto hr = S_OK;

	// Create vertex buffer
    podr->RedrawFunc(0, reim);

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = podr->Vertices().data();
	V_RETURN(pd3dDevice->CreateBuffer(&g_bd, &InitData, pVertexBuffer.ReleaseAndGetAddressOf()));
    	
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
            ::MessageBox(nullptr, utility::my_mbstowcs(e.what()).c_str(), L"エラー", MB_OK | MB_ICONWARNING);
            continue;
        }
        break;
    }
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text.
//--------------------------------------------------------------------------------------
void RenderText()
{
	pTxtHelper->Begin();
	pTxtHelper->SetInsertionPos(5, 5);
	pTxtHelper->SetForegroundColor(Colors::White);
	pTxtHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
	pTxtHelper->DrawTextLine(DXUTGetDeviceStats());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Number of atoms: %d") % armd.NumAtom).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Number of supercell: %d") % armd.Nc).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Number of MD step: %d") % armd.MD_iter).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Elapsed time: %.3f (ps)") % armd.getDeltat()).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Lattice constant: %.3f (nm)") % armd.getLatticeconst()).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Periodic length: %.3f (nm)") % armd.getPeriodiclen()).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Preset temperture: %.3f (K)") % armd.getTgiven()).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Calculation temperture: %.3f (K)") % armd.getTcalc()).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Kinetic energy: %.3f (Hartree)") % armd.Uk).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Potential energy: %.3f (Hartree)") % armd.Up).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Total energy: %.3f (Hartree)") % armd.Utot).str().c_str());
	//pTxtHelper->DrawTextLine((boost::wformat(L"Pressure: %.3f (atm)") % armd.getPressure()).str().c_str());
	pTxtHelper->End();
}


void SetUI()
{
	hud.SetCallback(OnGUIEvent);
	
	auto iY = 10;
	hud.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22);
	hud.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += 26, 170, 22, VK_F2);

	ui.SetCallback(OnGUIEvent);

	hud.AddButton(IDC_RECALC, L"Recalculation", 35, iY += 34, 125, 22);

	// 温度の変更
	//hud.AddStatic(IDC_OUTPUT, L"Temperture", 20, iY += 34, 125, 22);
	//hud.GetStatic(IDC_OUTPUT)->SetTextColor(D3DCOLOR_ARGB(255, 255, 255, 255));
	//hud.AddSlider(
	//	IDC_SLIDER,
	//	35,
	//	iY += 24,
	//	125,
	//	22,
	//	1,
	//	3000,
	//	boost::numeric_cast<int>(moleculardynamics::Ar_moleculardynamics::FIRSTTEMP));

	//// 格子定数の変更
	//hud.AddStatic(IDC_OUTPUT2, L"Lattice constant", 20, iY += 34, 125, 22);
	//hud.GetStatic(IDC_OUTPUT2)->SetTextColor(D3DCOLOR_ARGB(255, 255, 255, 255));
	//hud.AddSlider(
	//	IDC_SLIDER2,
	//	35,
	//	iY += 24,
	//	125,
	//	22,
	//	30,
	//	1000,
	//	boost::numeric_cast<int>(moleculardynamics::Ar_moleculardynamics::FIRSTSCALE * LATTICERATIO));

	//// スーパーセルの個数の変更
	//hud.AddStatic(IDC_OUTPUT3, L"Number of supercell", 20, iY += 34, 125, 22);
	//hud.GetStatic(IDC_OUTPUT3)->SetTextColor(D3DCOLOR_ARGB(255, 255, 255, 255));
	//hud.AddSlider(
	//	IDC_SLIDER3,
	//	35,
	//	iY += 24,
	//	125,
	//	22,
	//	1,
	//	16,
	//	moleculardynamics::Ar_moleculardynamics::FIRSTNC);

	// アンサンブルの変更
	hud.AddStatic(IDC_OUTPUT4, L"Ensemble", 20, iY += 40, 125, 22);
	hud.GetStatic(IDC_OUTPUT4)->SetTextColor(D3DCOLOR_ARGB(255, 255, 255, 255));
	hud.AddRadioButton(IDC_RADIOA, 1, L"NVT ensemble", 35, iY += 24, 125, 22, true);
	hud.AddRadioButton(IDC_RADIOB, 1, L"NVE ensemble", 35, iY += 28, 125, 22, false);

	// 温度制御法の変更
	hud.AddStatic(IDC_OUTPUT4, L"Temperture control", 15, iY += 40, 125, 22);
	hud.GetStatic(IDC_OUTPUT4)->SetTextColor(D3DCOLOR_ARGB(255, 255, 255, 255));
	hud.AddRadioButton(IDC_RADIOC, 2, L"Langevin", 20, iY += 24, 125, 22, false);
	hud.AddRadioButton(IDC_RADIOD, 2, L"Nose-Hoover", 20, iY += 28, 125, 22, false);
	hud.AddRadioButton(IDC_RADIOE, 2, L"Velocity Scaling", 20, iY += 28, 125, 22, true);
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

	// ウィンドウを生成
	auto const dispx = ::GetSystemMetrics(SM_CXSCREEN);
	auto const dispy = ::GetSystemMetrics(SM_CYSCREEN);
	auto const xpos = (dispx - WINDOWWIDTH) / 2;
	auto const ypos = (dispy - WINDOWHEIGHT) / 2;
	DXUTCreateWindow( L"test", nullptr, nullptr, nullptr, xpos, ypos);
	
    // Only require 10-level hardware or later
    DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, WINDOWWIDTH, WINDOWHEIGHT);

	// Enter into the DXUT render loop
    DXUTMainLoop();

    return DXUTGetExitCode();
}
