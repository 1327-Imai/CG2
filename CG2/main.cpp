#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <vector>
#include <string>
#include <DirectXmath.h>
#include <d3dcompiler.h>

#define DIRECTINPUT_VERSION 0x0800	//DirectInoutのバージョン指定
#include <dinput.h>

#include <math.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

using namespace DirectX;

const double PI = 3.141592;

#pragma region//関数のプロトタイプ宣言
//ウィンドウプロシーシャ
LRESULT WindowProc(HWND hwnd , UINT msg , WPARAM wparam , LPARAM lparam);

#pragma endregion//関数のプロトタイプ宣言

//main関数
int WINAPI WinMain(HINSTANCE , HINSTANCE , LPSTR , int) {
	//コンソールへの文字出力
	OutputDebugStringA("Hello,DirectX!!/n");


#pragma region//ウィンドウの生成
	//ウィンドウサイズ
	const int window_width = 1280;
	const int window_height = 720;

	//ウィンドウクラスの設定
	WNDCLASSEX w{};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;		//ウィンドウプロシーシャを設定
	w.lpszClassName = L"DirectXGame";			//ウィンドウクラス名
	w.hInstance = GetModuleHandle(nullptr);		//ウィンドウハンドル
	w.hCursor = LoadCursor(NULL , IDC_ARROW);	//カーソル指定

	//ウィンドウクラスをOSに登録する
	RegisterClassEx(&w);
	//ウィンドウサイズ{ X座標 Y座標 横幅 縦幅 }
	RECT wrc = {0 , 0 , window_width , window_height};
	//自動でサイズを補正する
	AdjustWindowRect(&wrc , WS_OVERLAPPEDWINDOW , false);

	//ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow(
		w.lpszClassName ,		//クラス名
		L"DirectXGame" ,			//タイトルバーの文字
		WS_OVERLAPPEDWINDOW ,	//標準的なウィンドウスタイル
		CW_USEDEFAULT ,			//表示X座標(OSに任せる)
		CW_USEDEFAULT ,			//表示Y座標(OSに任せる)
		wrc.right - wrc.left ,	//ウィンドウ横幅
		wrc.bottom - wrc.top ,	//ウィンドウ縦幅
		nullptr ,				//親ウィンドウハンドル
		nullptr ,				//メニューハンドル
		w.hInstance ,			//呼び出しアプリケーションハンドル
		nullptr);				//オプション

	//ウィンドウを表示状態にする
	ShowWindow(hwnd , SW_SHOW);
#pragma endregion//ウィンドウの生成

#pragma region//メッセージループ
	MSG msg{}; //メッセージ

#pragma region//DirectX初期化処理
	//デバッグレイヤーの有効化

#ifdef _DEBUG
	//デバッグレイヤーをオンに
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif

	//DirectX初期化処理
	HRESULT result;
	ID3D12Device* device = nullptr;
	IDXGIFactory7* dxgiFactory = nullptr;
	IDXGISwapChain4* swapChain = nullptr;
	ID3D12CommandAllocator* cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12DescriptorHeap* rtvHeap = nullptr;

	//アダプタの列挙
	//DX6Iファクトリーの生成
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//アダプターの列挙例
	std::vector<IDXGIAdapter4*>adapters;
	//ここに特定の名前を持つアダプターオブジェクトが入る
	IDXGIAdapter4* tmpAdapter = nullptr;

	//パフォーマンスが高いものをから順に、すべてのアダプターを列挙する
	for (UINT i = 0;
		 dxgiFactory->EnumAdapterByGpuPreference(i ,
		 DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE ,
		 IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND;
		 i++) {
		//動的配列につかする
		adapters.push_back(tmpAdapter);
	}

	//妥当なアダプタを選別する
	for (size_t i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC3 adapterDesc;
		//アダプターの情報を取得する
		adapters[i]->GetDesc3(&adapterDesc);

		//ソフトウェアデバイスを回避
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//デバイスを採用してループを抜ける
			tmpAdapter = adapters[i];
			break;
		}
	}

	//デバイスの生成
	//対応レベルの配列
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1 ,
		D3D_FEATURE_LEVEL_12_0 ,
		D3D_FEATURE_LEVEL_11_1 ,
		D3D_FEATURE_LEVEL_11_0 ,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (size_t i = 0; i < _countof(levels); i++) {
		//採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter , levels[i] ,
								   IID_PPV_ARGS(&device));
		if (result == S_OK) {
			//デバイスを生成できた時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}

	//コマンドリスト
	//コマンドアロケータを生成
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT ,
		IID_PPV_ARGS(&cmdAllocator));
	assert(SUCCEEDED(result));

	//コマンドリストを生成
	result = device->CreateCommandList(0 ,
									   D3D12_COMMAND_LIST_TYPE_DIRECT ,
									   cmdAllocator , nullptr ,
									   IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	//コマンドキュー
	//コマンドキューの設定
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//コマンドキューを生成
	result = device->CreateCommandQueue(&commandQueueDesc , IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));

	//スワップチェーンの設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				//色情報の書式
	swapChainDesc.SampleDesc.Count = 1;								//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;				//バックバッファ用
	swapChainDesc.BufferCount = 2;									//バッファ数を2つに設定
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;			//フリップ後は破棄
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//スワップチェーンの生成
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue , hwnd , &swapChainDesc , nullptr , nullptr ,
		(IDXGISwapChain1**)&swapChain);
	assert(SUCCEEDED(result));

	//デスクリプタヒープ
	//デスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;

	//デスクリプタヒープの生成
	device->CreateDescriptorHeap(&rtvHeapDesc , IID_PPV_ARGS(&rtvHeap));

	//バックバッファ
	std::vector<ID3D12Resource*> backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	//レンダービューターゲット
	//スワップチェーンの全てのバッファについて処理する
	for (size_t i = 0; i < backBuffers.size(); i++) {
		//スワップチェーンからバッファを取得
		swapChain->GetBuffer((UINT)i , IID_PPV_ARGS(&backBuffers[i]));
		//デスクリプタヒープのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//裏か表かでアドレスがずれる
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//レンダービューターゲットの設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//シェーダーの計算結果をSRGBに変換して書き込む
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//レンダービューターゲットの生成
		device->CreateRenderTargetView(backBuffers[i] , &rtvDesc , rtvHandle);

	}
	//フェンス
	//フェンスの生成
	ID3D12Fence* fence = nullptr;
	UINT64 fenceVal = 0;

	result = device->CreateFence(fenceVal , D3D12_FENCE_FLAG_NONE , IID_PPV_ARGS(&fence));

	//Directinputの初期化
	IDirectInput8* directInput = nullptr;
	result = DirectInput8Create(
		w.hInstance , DIRECTINPUT_VERSION , IID_IDirectInput8 ,
		(void**)&directInput , nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの生成
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard , &keyboard , NULL);
	assert(SUCCEEDED(result));

	//入力データのリセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard); //標準形式
	assert(SUCCEEDED(result));

	//排他制御レベルのリセット
	result = keyboard->SetCooperativeLevel(
		hwnd , DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

#pragma endregion//DirectX初期化処理

#pragma region//更新処理用変数

	float transrateX = 0.0f;
	float transrateY = 0.0f;
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float angle = 0;

	float affineTransrate[3][3] = {
		{1 , 0 , transrateX} ,
		{0 , 1 , transrateY} ,
		{0 , 0 , 1.0f} ,
	};

	float affineRotation[3][3] = {
		{cos(angle) , -sin(angle) , 0} ,
		{sin(angle) , cos(angle) , 0} ,
		{0 , 0 , 1.0f} ,
	};

	float affineScale[3][3] = {
		{scaleX * 1 , 0 , 0} ,
		{0 , scaleY * 1 , 0} ,
		{0 , 0 , 1.0f} ,
	};

	float boxMoved[4][3] = {0};

	float boxRotated[4][3] = {0};

	float boxEnlarged[4][3] = {0};

	float distanceFromOriginX = 0;
	float distanceFromOriginY = 0;

#pragma endregion//更新処理用変数

#pragma region//描画初期化処理

#pragma region//頂点リスト:vertices
	//頂点データ
	XMFLOAT3 vertices[] = {
		{-0.2f , -0.2f , 1.0f} , //左下 インデックス0
		{-0.2f , +0.2f , 1.0f} , //左上 インデックス1
		{+0.2f , -0.2f , 1.0f} , //右下 インデックス2
		{+0.2f , +0.2f , 1.0f} , //右上 インデックス3
	};

	//インデックスデータ
	uint16_t indices[] = {
		0 , 1 , 2 , //三角形1つ目
		1 , 2 , 3 , //三角形2つ目
	};

	//頂点データ全体のサイズ = 頂点データ一つ分のサイズ * 頂点データの要素数
	UINT sizeVB = static_cast<UINT>(sizeof(XMFLOAT3) * _countof(vertices));

	//インデックスデータ全体のサイズ
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

	//頂点バッファの設定
	D3D12_HEAP_PROPERTIES heapProp{};		//ヒープ設定
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//GPUへの転送用

	//リソース設定
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB; //頂点データ全体のサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//頂点バッファの生成
	ID3D12Resource* vertBuff = nullptr;
	result = device->CreateCommittedResource(
		&heapProp ,	//ヒープ設定
		D3D12_HEAP_FLAG_NONE ,
		&resDesc ,	//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&vertBuff)
	);
	assert(SUCCEEDED(result));

	//インデックスバッファの生成
	ID3D12Resource* indexBuff = nullptr;
	result = device->CreateCommittedResource(
		&heapProp ,	//ヒープ設定
		D3D12_HEAP_FLAG_NONE ,
		&resDesc ,	//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&indexBuff)
	);
	assert(SUCCEEDED(result));

	//GPU上のバッファに対応した仮想メモリ(メインメモリ上)を取得
	XMFLOAT3* vertMap = nullptr;
	result = vertBuff->Map(0 , nullptr , (void**)&vertMap);
	//繋がりを解除
	vertBuff->Unmap(0 , nullptr);

	//インデックスバッファをマッピング
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0 , nullptr , (void**)&indexMap);
	//全インデックスに対して
	for (int i = 0; i < _countof(indices); i++) {
		indexMap[i] = indices[i];	//座標をコピー
	}
	//繋がりを解除
	indexBuff->Unmap(0 , nullptr);

	//頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//GPU仮想アドレス
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//頂点バッファのサイズ
	vbView.SizeInBytes = sizeVB;
	//頂点１つ分のデータサイズ
	vbView.StrideInBytes = sizeof(XMFLOAT3);

	//インデックスバッファビューの作成
	D3D12_INDEX_BUFFER_VIEW ibView{};
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
#pragma endregion//頂点リスト:vertices

#pragma region//頂点リスト:boxVertices
	//頂点データ
	XMFLOAT3 boxVertices[4] = {
		{0.0f , 0.0f , 0.0f}
	};

	//インデックスデータ
	uint16_t boxIndices[] = {
		0 , 1 , 2 , //三角形1つ目
		1 , 2 , 3 , //三角形2つ目
	};

	//頂点データ全体のサイズ = 頂点データ一つ分のサイズ * 頂点データの要素数
	UINT sizeBoxVB = static_cast<UINT>(sizeof(XMFLOAT3) * _countof(boxVertices));

	//インデックスデータ全体のサイズ
	UINT sizeBoxIB = static_cast<UINT>(sizeof(uint16_t) * _countof(boxIndices));

	//頂点バッファの設定
	D3D12_HEAP_PROPERTIES boxHeapProp{};		//ヒープ設定
	boxHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//GPUへの転送用

	//リソース設定
	D3D12_RESOURCE_DESC boxResDesc{};
	boxResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	boxResDesc.Width = sizeBoxIB; //頂点データ全体のサイズ
	boxResDesc.Height = 1;
	boxResDesc.DepthOrArraySize = 1;
	boxResDesc.MipLevels = 1;
	boxResDesc.SampleDesc.Count = 1;
	boxResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//頂点バッファの生成
	ID3D12Resource* boxVertBuff = nullptr;
	result = device->CreateCommittedResource(
		&boxHeapProp ,	//ヒープ設定
		D3D12_HEAP_FLAG_NONE ,
		&boxResDesc ,	//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&boxVertBuff)
	);
	assert(SUCCEEDED(result));

	//インデックスバッファの生成
	ID3D12Resource* boxIndexBuff = nullptr;
	result = device->CreateCommittedResource(
		&boxHeapProp ,	//ヒープ設定
		D3D12_HEAP_FLAG_NONE ,
		&boxResDesc ,	//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&boxIndexBuff)
	);
	assert(SUCCEEDED(result));

	//GPU上のバッファに対応した仮想メモリ(メインメモリ上)を取得
	XMFLOAT3* boxVertMap = nullptr;
	result = boxVertBuff->Map(0 , nullptr , (void**)&boxVertMap);
	//繋がりを解除
	boxVertBuff->Unmap(0 , nullptr);

	//インデックスバッファをマッピング
	uint16_t* boxIndexMap = nullptr;
	result = boxIndexBuff->Map(0 , nullptr , (void**)&boxIndexMap);
	//全インデックスに対して
	for (int i = 0; i < _countof(boxIndices); i++) {
		boxIndexMap[i] = boxIndices[i];	//座標をコピー
	}
	//繋がりを解除
	boxIndexBuff->Unmap(0 , nullptr);

	//頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW boxVbView{};
	//GPU仮想アドレス
	boxVbView.BufferLocation = boxVertBuff->GetGPUVirtualAddress();
	//頂点バッファのサイズ
	boxVbView.SizeInBytes = sizeBoxVB;
	//頂点１つ分のデータサイズ
	boxVbView.StrideInBytes = sizeof(XMFLOAT3);

	//インデックスバッファビューの作成
	D3D12_INDEX_BUFFER_VIEW boxIbView{};
	boxIbView.BufferLocation = boxIndexBuff->GetGPUVirtualAddress();
	boxIbView.Format = DXGI_FORMAT_R16_UINT;
	boxIbView.SizeInBytes = sizeBoxIB;
#pragma endregion//頂点リスト:boxVertices

	//頂点シェーダーファイルの読み込みとコンパイル
	ID3DBlob* vsBlob = nullptr;		//頂点シェーダーオブジェクト
	ID3DBlob* psBlob = nullptr;		//ピクセルシェーダーオブジェクト
	ID3DBlob* errorBlob = nullptr;	//エラーオブジェクト

	// 頂点シェーダーの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"BasicVS.hlsl" ,									//シェーダーファイル名
		nullptr ,
		D3D_COMPILE_STANDARD_FILE_INCLUDE ,					//インクルード可能にする
		"main" ,											//エントリーポイント名
		"vs_5_0" ,											//シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION ,	//デバッグ用設定
		0 ,
		&vsBlob ,
		&errorBlob
	);

	//シェーダーコードのエラー
	//エラーなら
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer() ,
					errorBlob->GetBufferSize() ,
					error.begin());
		error += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	//ピクセルシェーダーの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"BasicPS.hlsl" ,									//シェーダーファイル名
		nullptr ,
		D3D_COMPILE_STANDARD_FILE_INCLUDE ,					//インクルード可能にする
		"main" ,												//エントリーポイント名
		"ps_5_0" ,											//シェーダーモデル設定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION ,	//デバッグ用設定
		0 ,
		&psBlob ,
		&errorBlob
	);

	//シェーダーコードのエラー
	//エラーなら
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer() ,
					errorBlob->GetBufferSize() ,
					error.begin());
		error += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	//頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION" , 0 , DXGI_FORMAT_R32G32B32_FLOAT , 0 ,
			D3D12_APPEND_ALIGNED_ELEMENT ,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0
		} ,
	};

	//グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	//シェーダーの設定
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	//サンプルマスクの設定
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;	//標準設定

	//ラスタライザの設定
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;		//カリングしない
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;		//ポリゴン内塗りつぶし
	//pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;	//ワイヤーフレーム表示
	pipelineDesc.RasterizerState.DepthClipEnable = true;				//深度クリッピングを有効に

	//ブレンドステート
	//pipelineDesc.BlendState.RenderTarget[0].RenderTargetWriteMask
	//	= D3D12_COLOR_WRITE_ENABLE_ALL;	//RGBA全てのチャンネルを描画

	//レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//共通設定(アルファ値)
	blenddesc.BlendEnable = true;					//ブレンドを有効にする
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;	//加算
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;		//ソースの値を100%使う
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;	//デストの値を  0%使う

	//加算合成
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;	//加算
	blenddesc.SrcBlend = D3D12_BLEND_ONE;	//ソースの値を100%使う
	blenddesc.DestBlend = D3D12_BLEND_ONE;	//デストの値を100%使う

	//頂点レイアウトの設定
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	//図形の形状設定
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//その他の設定
	pipelineDesc.NumRenderTargets = 1;								//描画対象は1つ
	pipelineDesc.RTVFormats[0] - DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//0~255指定のRGBA
	pipelineDesc.SampleDesc.Count = 1;								//1ピクセルにつき1回のサンプリング

	//ルートパラメータ
	//ルートパラメータの設定
	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//定数バッファビュー
	rootParam.Descriptor.ShaderRegister = 0;					//定数バッファ番号
	rootParam.Descriptor.RegisterSpace = 0;						//デフォルト値
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//全てのシェーダーから見える

	//ルートシグネチャ
	ID3D12RootSignature* rootSignature;

	//ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = &rootParam;	//ルートパラメータの先頭アドレス
	rootSignatureDesc.NumParameters = 1;		//ルートパラメータ数

	//ルートシグネチャのシリアライズ
	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(&rootSignatureDesc , D3D_ROOT_SIGNATURE_VERSION_1_0 ,
										 &rootSigBlob , &errorBlob);
	assert(SUCCEEDED(result));

	result = device->CreateRootSignature(0 , rootSigBlob->GetBufferPointer() , rootSigBlob->GetBufferSize() ,
										 IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(result));
	rootSigBlob->Release();

	//パイプラインにルートシグネチャをセット
	pipelineDesc.pRootSignature = rootSignature;

	//パイプラインステートの生成
	ID3D12PipelineState* pipelineState = nullptr;
	result = device->CreateGraphicsPipelineState(&pipelineDesc , IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));

	//定数バッファ用データ構造体(マテリアル)
	struct ConstBufferDataMaterial {
		XMFLOAT4 color; //色(RGBA)
	};

	//定数バッファの生成
	//ヒープ設定
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//GPUへの転送用

	//リソース設定
	D3D12_RESOURCE_DESC cbResourceDesc{};
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff;	//256バイトアラインメント
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* constBuffMaterial = nullptr;
	//定数バッファの生成
	result = device->CreateCommittedResource(
		&cbHeapProp ,	//ヒープ設定
		D3D12_HEAP_FLAG_NONE ,
		&cbResourceDesc ,	//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&constBuffMaterial)
	);
	assert(SUCCEEDED(result));

	//定数バッファのマッピング
	ConstBufferDataMaterial* constMapMaterial = nullptr;
	result = constBuffMaterial->Map(0 , nullptr , (void**)&constMapMaterial); // マッピング
	assert(SUCCEEDED(result));

	//定数バッファへのデータ転送
	//値を書き込むと自動的に転送される
	constMapMaterial->color = XMFLOAT4(1 , 0 , 0 , 0.5f);

#pragma endregion//描画初期化処理

#pragma region//ゲームループ
	while (true) {

#pragma region//ウィンドウメッセージ処理
		//メッセージがある?
		if (PeekMessage(&msg , nullptr , 0 , 0 , PM_REMOVE)) {
			TranslateMessage(&msg);	//キー入力メッセージの処理
			DispatchMessage(&msg);	//プロシーシャにメッセージを送る
		}

		//×ボタンで終了メッセージが来たらゲームループを抜ける
		if (msg.message == WM_QUIT) {
			break;
		}
#pragma endregion//ウィンドウメッセージ処理

#pragma region//DirectX毎フレーム処理

		//キーボード情報の取得開始
		keyboard->Acquire();

		//前キーの入力状態を取得する
		BYTE key[256] = {};
		keyboard->GetDeviceState(sizeof(key) , key);

#pragma region//更新処理

		//キー入力に応じて変数の値を増減する
		//移動
		if (key[DIK_UP]) {
			transrateY += 5.0/window_height;
		}
		if (key[DIK_DOWN]) {
			transrateY -= 5.0 / window_height;
		}
		if (key[DIK_LEFT]) {
			transrateX -= 5.0 / window_width;
		}
		if (key[DIK_RIGHT]) {
			transrateX += 5.0 / window_width;
		}

		//拡大縮小
		if (key[DIK_A]) {
			if (0 < scaleX) {
				scaleX -= 0.01;
			}
		}
		if (key[DIK_D]) {
			scaleX += 0.01;
		}
		if (key[DIK_S]) {
			if (0 < scaleY) {
				scaleY -= 0.01;
			}
		}
		if (key[DIK_W]) {
			scaleY += 0.01;
		}

		//回転
		if (key[DIK_Q]) {
			angle -= PI / 360;
		}
		if (key[DIK_E]) {
			angle += PI / 360;
		}

		//リセット
		if (key[DIK_R]) {
			transrateX = 0;
			transrateY = 0;
			scaleX = 1;
			scaleY = 1;
			angle = 0;
		}

		//変数の値をアフィン行列のそれぞれ対応する場所に代入
		affineTransrate[0][2] = transrateX;
		affineTransrate[1][2] = transrateY;
		affineScale[0][0] = scaleX;
		affineScale[1][1] = scaleY;
		affineRotation[0][0] = cos(angle);
		affineRotation[0][1] = sin(angle);
		affineRotation[1][0] = -sin(angle);
		affineRotation[1][1] = cos(angle);


		//アフィン変換
#pragma region//移動
	//原点からの距離を取得
		distanceFromOriginX = vertices[0].x;
		distanceFromOriginY = vertices[0].y;

		//原点まで移動する(現在の座標から原点からの距離を引く)
		for (int i = 0; i < 4; i++) {
			vertices[i].x -= distanceFromOriginX;
			vertices[i].y -= distanceFromOriginY;
		}

		//アフィン変換をする
		for (int i = 0; i < 4; i++) {
			boxMoved[i][0] =
				affineTransrate[0][0] * vertices[i].x +
				affineTransrate[0][1] * vertices[i].y +
				affineTransrate[0][2] * vertices[i].z;

			boxMoved[i][1] =
				affineTransrate[1][0] * vertices[i].x +
				affineTransrate[1][1] * vertices[i].y +
				affineTransrate[1][2] * vertices[i].z;

			boxMoved[i][2] =
				affineTransrate[2][0] * vertices[i].x +
				affineTransrate[2][1] * vertices[i].y +
				affineTransrate[2][2] * vertices[i].z;
		};

		//原点からの距離を足して元の位置に戻す
		for (int i = 0; i < 4; i++) {
			vertices[i].x += distanceFromOriginX;
			vertices[i].y += distanceFromOriginY;
			boxMoved[i][0] += distanceFromOriginX;
			boxMoved[i][1] += distanceFromOriginY;
		}
#pragma endregion//移動

#pragma region//回転
		//原点からの距離を取得
		distanceFromOriginX = boxMoved[0][0];
		distanceFromOriginY = boxMoved[0][1];

		//原点まで移動する(現在の座標から原点からの距離を引く)
		for (int i = 0; i < 4; i++) {
			boxMoved[i][0] -= distanceFromOriginX;
			boxMoved[i][1] -= distanceFromOriginY;
		}

		//アフィン変換をする
		for (int i = 0; i < 4; i++) {
			boxRotated[i][0] =
				affineRotation[0][0] * boxMoved[i][0] +
				affineRotation[0][1] * boxMoved[i][1] +
				affineRotation[0][2] * boxMoved[i][2];

			boxRotated[i][1] =
				affineRotation[1][0] * boxMoved[i][0] +
				affineRotation[1][1] * boxMoved[i][1] +
				affineRotation[1][2] * boxMoved[i][2];

			boxRotated[i][2] =
				affineRotation[2][0] * boxMoved[i][0] +
				affineRotation[2][1] * boxMoved[i][1] +
				affineRotation[2][2] * boxMoved[i][2];
		};

		//原点からの距離を足して元の位置に戻す
		for (int i = 0; i < 4; i++) {
			boxMoved[i][0] += distanceFromOriginX;
			boxMoved[i][1] += distanceFromOriginY;
			boxRotated[i][0] += distanceFromOriginX;
			boxRotated[i][1] += distanceFromOriginY;
		}
#pragma endregion//回転

#pragma region//拡大縮小
		//原点からの距離を取得
		distanceFromOriginX = boxRotated[0][0];
		distanceFromOriginY = boxRotated[0][1];

		//原点まで移動する(現在の座標から原点からの距離を引く)
		for (int i = 0; i < 4; i++) {
			boxRotated[i][0] -= distanceFromOriginX;
			boxRotated[i][1] -= distanceFromOriginY;
		}

		//アフィン変換をする
		for (int i = 0; i < 4; i++) {
			boxEnlarged[i][0] =
				affineScale[0][0] * boxRotated[i][0] +
				affineScale[0][1] * boxRotated[i][1] +
				affineScale[0][2] * boxRotated[i][2];

			boxEnlarged[i][1] =
				affineScale[1][0] * boxRotated[i][0] +
				affineScale[1][1] * boxRotated[i][1] +
				affineScale[1][2] * boxRotated[i][2];

			boxEnlarged[i][2] =
				affineScale[2][0] * boxRotated[i][0] +
				affineScale[2][1] * boxRotated[i][1] +
				affineScale[2][2] * boxRotated[i][2];
		};

		//原点からの距離を足して元の位置に戻す
		for (int i = 0; i < 4; i++) {
			boxRotated[i][0] += distanceFromOriginX;
			boxRotated[i][1] += distanceFromOriginY;
			boxEnlarged[i][0] += distanceFromOriginX;
			boxEnlarged[i][1] += distanceFromOriginY;
		}
#pragma endregion//拡大縮小

		//アフィン返還した行列を頂点リスト:boxVerticesに代入
		for (int i = 0; i < _countof(boxVertices); i++) {
			boxVertices[i].x = boxEnlarged[i][0];
			boxVertices[i].y = boxEnlarged[i][1];
			boxVertices[i].z = boxEnlarged[i][2];
		}

		//全頂点に対して
		for (int i = 0; i < _countof(boxVertices); i++) {
			boxVertMap[i] = boxVertices[i];	//座標をコピー
		}

#pragma endregion//更新処理

#pragma region//描画処理
		//バックバッファの番号を取得（2つなので0番か1番）
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

		//1.リソースバリアで書き込み可能に変更
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex];				//バックバッファを指定
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;		//表示状態から
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	//描画状態へ
		commandList->ResourceBarrier(1 , &barrierDesc);

		//2.描画先の変更
		//レンダーターゲットビューのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		commandList->OMSetRenderTargets(1 , &rtvHandle , false , nullptr);

		//3.画面クリア
		FLOAT clearColor[] = {0.1f , 0.25f , 0.5f , 0.0f};

		if (key[DIK_SPACE]) {

			clearColor[0] = 0.5f;
			clearColor[1] = 0.1f;
			clearColor[2] = 0.25f;
			clearColor[3] = 0.0f;

		}

		commandList->ClearRenderTargetView(rtvHandle , clearColor , 0 , nullptr);

		//4.描画コマンド
#pragma region//グラフィックコマンド

		//ビューポート設定のコマンド
		D3D12_VIEWPORT viewport{};
		viewport.Width = window_width;		//横幅
		viewport.Height = window_height - 0;	//縦幅
		viewport.TopLeftX = 0;	//左上X
		viewport.TopLeftY = 0;					//左上Y
		viewport.MinDepth = 0.0f;				//最小深度
		viewport.MaxDepth = 1.0f;				//最大深度

		//ビューポート設定コマンドを、コマンドリストに積む
		commandList->RSSetViewports(1 , &viewport);

		//シザー矩形
		D3D12_RECT scissorRect{};
		scissorRect.left = 0;									//切り抜き座標左
		scissorRect.right = scissorRect.left + window_width;	//切り抜き座標右
		scissorRect.top = 0;						//切り抜き座標上
		scissorRect.bottom = scissorRect.top + window_height;	//切り抜き座標下

		//シザー矩形コマンドを、コマンドリストに積む
		commandList->RSSetScissorRects(1 , &scissorRect);

		//パイプラインステートとルートシグネチャの設定コマンド
		commandList->SetPipelineState(pipelineState);
		commandList->SetGraphicsRootSignature(rootSignature);

		//プリミティブ形状の設定コマンド
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//頂点バッファビューの設定コマンド
		commandList->IASetVertexBuffers(0 , 1 , &boxVbView);

		//インデックスバッファビューの設定コマンド
		commandList->IASetIndexBuffer(&boxIbView);

		//頂点バッファ―ビューをセットするコマンド
		commandList->SetGraphicsRootConstantBufferView(0 , constBuffMaterial->GetGPUVirtualAddress());

		//描画コマンド
		commandList->DrawIndexedInstanced(_countof(boxIndices) , 1 , 0 , 0 , 0);	//全ての頂点を使って描画

#pragma endregion//グラフィックコマンド

		//5.リソースバリアを戻す
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//描画状態から
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;			//表示状態へ
		commandList->ResourceBarrier(1 , &barrierDesc);

		//命令のクローズ
		result = commandList->Close();
		assert(SUCCEEDED(result));
		//コマンドリストの実行
		ID3D12CommandList* commandLists[] = {commandList};
		commandQueue->ExecuteCommandLists(1 , commandLists);

		//画面に表示するバッファをフリップ(裏表の入れ替え)
		result = swapChain->Present(1 , 0);
		assert(SUCCEEDED(result));

		//コマンドの実行完了を待つ
		commandQueue->Signal(fence , ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr , false , false , nullptr);
			fence->SetEventOnCompletion(fenceVal , event);
			WaitForSingleObject(event , INFINITE);
			CloseHandle(event);
		}

		//キューをクリア
		result = cmdAllocator->Reset();
		assert(SUCCEEDED(result));
		//再びコマンドリストをためる準備
		result = commandList->Reset(cmdAllocator , nullptr);
		assert(SUCCEEDED(result));
#pragma endregion//描画処理

#pragma endregion//DirectX毎フレーム処理

	}
#pragma endregion//ゲームループ

	//ウィンドウクラス登録解除
	UnregisterClass(w.lpszClassName , w.hInstance);
#pragma endregion//メッセージループ

	return 0;
}


#pragma region//関数の定義
//ウィンドウプロシーシャ
LRESULT WindowProc(HWND hwnd , UINT msg , WPARAM wparam , LPARAM lparam) {

	//メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		//ウィンドウが破棄された
	case WM_DESTROY:
	//OSに対して、アプリの終了を伝える
	PostQuitMessage(0);
	return 0;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd , msg , wparam , lparam);
}
#pragma endregion//関数の定義