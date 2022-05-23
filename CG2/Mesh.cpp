#include "Mesh.h"

Mesh::Mesh(XMFLOAT3 vertices[3],ID3D12Device* device) {

	HRESULT result;

	//���_�f�[�^
	for (int i = 0; i < 3; i++) {
		this->vertices[i] = vertices[i];
	}

	//�C���f�b�N�X�f�[�^
	uint16_t indices[3] = {
		0 , 1 , 2
	};

	//���_�f�[�^�S�̂̃T�C�Y = ���_�f�[�^����̃T�C�Y * ���_�f�[�^�̗v�f��
	sizeVB = static_cast<UINT>(sizeof(XMFLOAT3) * _countof(this->vertices));

	//�C���f�b�N�X�f�[�^�S�̂̃T�C�Y
	sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

	//���_�o�b�t�@�̐ݒ�
	//�q�[�v�ݒ�
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//GPU�ւ̓]���p

	//���\�[�X�ݒ�
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB; //���_�f�[�^�S�̂̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//���_�o�b�t�@�̐���
	result = device->CreateCommittedResource(
		&heapProp ,	//�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE ,
		&resDesc ,	//���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&vertBuff)
	);
	assert(SUCCEEDED(result));

	//�C���f�b�N�X�o�b�t�@�̐���
	result = device->CreateCommittedResource(
		&heapProp ,	//�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE ,
		&resDesc ,	//���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&indexBuff)
	);
	assert(SUCCEEDED(result));

	//GPU��̃o�b�t�@�ɑΉ��������z������(���C����������)���擾
	result = vertBuff->Map(0 , nullptr , (void**)&vertMap);
	//�q���������
	vertBuff->Unmap(0 , nullptr);

	//�C���f�b�N�X�o�b�t�@���}�b�s���O
	result = indexBuff->Map(0 , nullptr , (void**)&indexMap);
	//�S�C���f�b�N�X�ɑ΂���
	for (int i = 0; i < _countof(indices); i++) {
		indexMap[i] = indices[i];	//���W���R�s�[
	}
	//�q���������
	indexBuff->Unmap(0 , nullptr);

	//���_�o�b�t�@�r���[�̍쐬
	//GPU���z�A�h���X
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//���_�o�b�t�@�̃T�C�Y
	vbView.SizeInBytes = sizeVB;
	//���_�P���̃f�[�^�T�C�Y
	vbView.StrideInBytes = sizeof(XMFLOAT3);

	//�C���f�b�N�X�o�b�t�@�r���[�̍쐬
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
}

Mesh::~Mesh() {

}

void Mesh::Update() {
	//�S���_�ɑ΂���
	for (int i = 0; i < _countof(vertices); i++) {
		vertMap[i] = vertices[i];	//���W���R�s�[
	}
}

void Mesh::Draw(ID3D12GraphicsCommandList* commandList, ID3D12Resource* constBuffMaterial) {
	//���_�o�b�t�@�r���[�̐ݒ�R�}���h
	commandList->IASetVertexBuffers(0 , 1 , &vbView);

	//�C���f�b�N�X�o�b�t�@�r���[�̐ݒ�R�}���h
	commandList->IASetIndexBuffer(&ibView);

	//���_�o�b�t�@�\�r���[���Z�b�g����R�}���h
	commandList->SetGraphicsRootConstantBufferView(0 , constBuffMaterial->GetGPUVirtualAddress());

	//�`��R�}���h
	commandList->DrawIndexedInstanced(_countof(indices) , 1 , 0 , 0 , 0);	//�S�Ă̒��_���g���ĕ`��
}