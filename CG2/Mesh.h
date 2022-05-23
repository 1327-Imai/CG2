#pragma once

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

class Mesh {
public:
	//コンストラクタ
	Mesh(XMFLOAT3 vertices[3] , ID3D12Device* device);

	~Mesh();

	//メンバ関数
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList, ID3D12Resource* constBuffMaterial);

	//メンバ変数
private:
	//頂点データ
	XMFLOAT3 vertices[3];

	//インデックスデータ
	uint16_t indices[3];

	//頂点データのサイズ
	UINT sizeVB;

	//インデックスデータ全体のサイズ
	UINT sizeIB;

	//頂点バッファ
	D3D12_HEAP_PROPERTIES heapProp{};

	//リソース
	D3D12_RESOURCE_DESC resDesc{};

	//頂点バッファ
	ID3D12Resource* vertBuff;

	//インデックスバッファ
	ID3D12Resource* indexBuff;

	//GPU上のバッファに対応した仮想メモリ(メインメモリ上)
	XMFLOAT3* vertMap;

	//インデックスバッファをマッピング
	uint16_t* indexMap;

	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	//インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView{};
};

