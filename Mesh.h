#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Graphics.h"
#include "Vertex.h"

class Mesh
{

private:

	// Buffers
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView;
	Microsoft::WRL::ComPtr <ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW ibView;

	//Index and vertex count
	int indexCount;
	int vertexCount;

	// Helper method to create buffers from vertex and index data
	void CreateBuffers(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount);

	// Calculates the tangents of the vertices in a mesh
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	
public:

	// Constructor
	Mesh(Vertex *vertices, int vertexCount, unsigned int* indices, int indexCount);
	Mesh(const char* filename);

	// Destructor
	~Mesh();

	// Getters for data
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW  GetIndexBufferView();
	int GetIndexCount();
	int GetVertexCount();


};