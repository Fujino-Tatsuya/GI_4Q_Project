#include "stdafx.h"

#include "WindowManager.h"
#include "SceneManager.h"
#include "NavigationManager.h"
#include "RNG.h"
#include "SoundManager.h"

#include "TestScene.h"
#include "TitleScene.h"
#include "HyojeTestScene.h"
#include "TaehyeonTestScene.h"
#include "EndingScene.h"

#include "GameManager.h"

using namespace std;

int main()
{
	#ifdef _DEBUG
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	#endif

	WindowManager& windowManager = WindowManager::GetInstance();
	windowManager.Initialize(L"Aurora");

	NavigationManager::GetInstance().Initialize();

	SceneManager& sceneManager = SceneManager::GetInstance();
	sceneManager.Initialize();
	sceneManager.ChangeScene("TitleScene");

	RNG::GetInstance().Initialize();

	SoundManager& soundManager = SoundManager::GetInstance();
	soundManager.Initialize();

	GameManager& gameManager = GameManager::GetInstance();
	gameManager.Initialize();

	while (windowManager.ProcessMessages())
	{
		soundManager.Update();
		sceneManager.Run();
	}

	windowManager.Finalize();

	sceneManager.Finalize();

	#ifdef _DEBUG
	ImGui::DestroyContext();
	#endif
}
