#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "BufferStructs.h"
#include "RayTracing.h"


#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	// Initialize raytracing
	RayTracing::Initialize(
		Window::Width(),
		Window::Height(),
		FixPath(L"RayTracing.cso"));

	CreateGeometry();

	// Camera initialization
	camera = std::make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -15.0f), 10.0f, 0.01f, XM_PIDIV4, Window::AspectRatio());

	
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// Wait for the GPU before we shut down
	Graphics::WaitForGPU();
}



// Helper macro for getting a float between min and max
#define RandomRange(min, max) (float)rand() / RAND_MAX * (max - min) + min

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{

	// Create materials
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState{};
	std::shared_ptr<Material> greyMat = std::make_shared<Material>(pipelineState, XMFLOAT3(0.5f, 0.5f, 0.5f));

	// Load meshes
	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>(FixPath("../../Assets/Models/cube.obj").c_str());
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>(FixPath("../../Assets/Models/torus.obj").c_str());
	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>(FixPath("../../Assets/Models/sphere.obj").c_str());

	// Floor
	std::shared_ptr<GameEntity> floor = std::make_shared<GameEntity>(cubeMesh, greyMat);
	floor->GetTransform()->SetScale(50, 50, 50);
	floor->GetTransform()->SetPosition(0, -51, 0);
	entities.push_back(floor);

	
	XMFLOAT3 colors[10] =
	{
		XMFLOAT3(1, 0, 0),   // Red
		XMFLOAT3(0, 1, 0),   // Green
		XMFLOAT3(0, 0, 1),   // Blue
		XMFLOAT3(1, 1, 0),   // Yellow
		XMFLOAT3(1, 0, 1),   // Magenta
		XMFLOAT3(0, 1, 1),   // Cyan
		XMFLOAT3(1, 0.5f, 0),// Orange
		XMFLOAT3(0.5f, 0, 1),// Purple
		XMFLOAT3(0.3f, 0.7f, 0.2f),
		XMFLOAT3(0.9f, 0.9f, 0.9f)
	};

	// Toruses
	for (int i = 0; i < 5; i++)
	{
		std::shared_ptr<Material> mat =
			std::make_shared<Material>(pipelineState, colors[i]);

		std::shared_ptr<GameEntity> torus =
			std::make_shared<GameEntity>(torusMesh, mat);

		torus->GetTransform()->SetScale(1.5f, 1.5f, 1.5f);
		torus->GetTransform()->SetPosition(-8.0f + i * 4.0f, 2.0f, -3.0f);

		entities.push_back(torus);
	}

	// Spheres
	for (int i = 0; i < 5; i++)
	{
		std::shared_ptr<Material> mat =
			std::make_shared<Material>(pipelineState, colors[i + 5]);

		std::shared_ptr<GameEntity> sphere =
			std::make_shared<GameEntity>(sphereMesh, mat);

		sphere->GetTransform()->SetScale(1.0f, 1.0f, 1.0f);
		sphere->GetTransform()->SetPosition(-8.0f + i * 4.0f, 1.0f, 3.0f);

		entities.push_back(sphere);
	}

	// Create the ray tracing entity data buffer now that we have a scene
	RayTracing::CreateEntityDataBuffer(entities);

	// Once we have all of the BLASs ready, we can make a TLAS
	RayTracing::CreateTopLevelAccelerationStructureForScene(entities);


	// Finalize any initialization and wait for the GPU
	// before proceeding to the game loop
	Graphics::CloseAndExecuteCommandList();
	Graphics::WaitForGPU();
	Graphics::ResetAllocatorAndCommandList(0);

}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Resize raytracing output texture
	RayTracing::ResizeOutputUAV(Window::Width(), Window::Height());


	// Update proj matrix
	if (camera) 
	{
		camera->UpdateProjectionMatrix(Window::AspectRatio());
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	camera->Update(deltaTime);

	// Torus movment
	for (int i = 0; i < entities.size(); i++)
	{
		if (i >= 1 && i <= 5) // torus range
		{
			auto t = entities[i]->GetTransform();
			t->Rotate(deltaTime * 0.5f, deltaTime, deltaTime * 0.25f);
		}
	}

	// Sphere movment
	int sphereStart = 6;

	for (int s = 0; s < 5; s++)
	{
		int i = sphereStart + s;

		auto transform = entities[i]->GetTransform();

		// Base position
		float baseX = -8.0f + s * 4.0f;
		float baseZ = 3.0f;

		float wave = sin(totalTime * 0.8f + s) * 3.0f;

		XMFLOAT3 pos;

		if (s % 2 == 0)
		{
			pos = XMFLOAT3(baseX + wave, 1.0f, baseZ);
		}
		else
		{
			pos = XMFLOAT3(baseX, 1.0f, baseZ + wave);
		}

		transform->SetPosition(pos);
	}
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr <ID3D12Resource > currentBackBuffer =
		Graphics::BackBuffers[Graphics::SwapChainIndex()];

	// Clearing the render target
	{
		// Transition the back buffer from present to render target
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList -> ResourceBarrier(1, &rb);

		// Background color (Cornflower Blue in this case) for clearing
		float color[] = { 0.4f, 0.6f, 0.75f, 1.0f };

		// Clear the RTV
		Graphics::CommandList -> ClearRenderTargetView(
			Graphics::RTVHandles[Graphics::SwapChainIndex()],
			color,
			0, 0); // No scissor rectangles

		// Clear the depth buffer, too
		Graphics::CommandList -> ClearDepthStencilView(
			Graphics::DSVHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f, // Max depth = 1.0f
			0, // Not clearing stencil, but need a value
			0, 0); // No scissor rects
	}

	// Raytracing - Recreate the TLAS and then trace it
	{
		RayTracing::CreateTopLevelAccelerationStructureForScene(entities);
		RayTracing::Raytrace(camera, currentBackBuffer);
	}


	// Present
	{
		// Transition back to present
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList -> ResourceBarrier(1, &rb);

		// Must occur BEFORE present
		Graphics::CloseAndExecuteCommandList();

		// Present the current back buffer and move to the next one
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain -> Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);
		Graphics::AdvanceSwapChainIndex();

		// No more waitning for GPU, then reset the command list & allocator
		Graphics::ResetAllocatorAndCommandList(Graphics::SwapChainIndex());
	}
}



