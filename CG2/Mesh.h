#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <vector>
#include <string>
#include <DirectXmath.h>
#include <d3dcompiler.h>

#define DIRECTINPUT_VERSION 0x0800	//DirectInout�̃o�[�W�����w��
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
	//�R���X�g���N�^
	Mesh(XMFLOAT3 vertices[3] , ID3D12Device* device);

	~Mesh();

	//�����o�֐�
	void Update();
	void Draw(ID3D12GraphicsCommandList* commandList, ID3D12Resource* constBuffMaterial);

	//�����o�ϐ�
private:
	//���_�f�[�^
	XMFLOAT3 vertices[3];

	//�C���f�b�N�X�f�[�^
	uint16_t indices[3];

	//���_�f�[�^�̃T�C�Y
	UINT sizeVB;

	//�C���f�b�N�X�f�[�^�S�̂̃T�C�Y
	UINT sizeIB;

	//���_�o�b�t�@
	D3D12_HEAP_PROPERTIES heapProp{};

	//���\�[�X
	D3D12_RESOURCE_DESC resDesc{};

	//���_�o�b�t�@
	ID3D12Resource* vertBuff;

	//�C���f�b�N�X�o�b�t�@
	ID3D12Resource* indexBuff;

	//GPU��̃o�b�t�@�ɑΉ��������z������(���C����������)
	XMFLOAT3* vertMap;

	//�C���f�b�N�X�o�b�t�@���}�b�s���O
	uint16_t* indexMap;

	//���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	//�C���f�b�N�X�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW ibView{};
};

