#pragma once
#include "Player.h"
#include <string>

class Panel;

enum class EScene
{
	Title,
    Main,
	Result,
};

enum class EMainState
{
    None,
    Tutorial,
    Stage1,
    Stage2,
    StageBoss
};

enum class ETutorialStep
{
	WASD,
	Dash,
	Reload,
	Shoot,
	AutoReload,
	DeadEye,
    End
};


class GameManager : public Singleton<GameManager>
{
	friend class Singleton<GameManager>;

    Panel* m_optionPanel = nullptr;
    Panel* m_cheatPanel = nullptr;

///GameFlow
    Player* m_Player = nullptr;

    bool m_Pause = false;
    bool m_isSuccess = false;
    bool m_isCheat = false;

    EScene m_CurrentScene = EScene::Title;
    EMainState m_MainState = EMainState::None;
	EMainState m_PrevMainState = EMainState::None;
    EMainState m_QueuedMainState = EMainState::None;
    ETutorialStep m_TutorialStep = ETutorialStep::WASD;
///GameFlowEnd

///SCORE
    //점수 관련 변수
	int     m_currentScore = 0;
	int     m_multiplier = 1;         // 1, 2, 4, 8
	int     m_killCountForNextLevel = 0;
	float   m_lastKillTime = 0.0f;
	bool    m_isCombatStarted = false;
	float   m_decayTimer = 0.0f;

    void TempPrint();   // UI에 넣기 전에 저장할 위치
///SCORE END

///RANKING -> LOG
    std::vector<std::pair<std::string, int>> m_rankings;
///RANKING END

	Panel* m_tutorialPanel = nullptr;  
	Panel* m_tutorialPopup = nullptr;  
	bool   m_tutorialPopupOpen = false;
	bool   m_stepPopupShown = false;   


	
public:
    void Initialize();
    void Finalize();

    Player* GetPlayerPtr();
///GameFlow

    void Update();
    void OnSceneEnter(EScene type);
    void OnSceneUpdate();
    void OnSceneRender();
    void OnSceneExit();

    bool IsPaused() const { return m_Pause; }
    void SetPaused(bool v) { m_Pause = v; }
    void RegisterOptionPanel(Panel* panel) { m_optionPanel = panel; }
    void RegisterCheatPanel(Panel* panel) { m_cheatPanel = panel; }
    void ToggleOption();
    void ToggleCheatPanel();
    void CheatGoto(EMainState state, bool forceTeleport = false);
    void CheatGotoByActionKey(const std::string& actionKey);
    bool IsCheat() const { return m_isCheat; }
    bool IsSuccess() const { return m_isSuccess; }
    void SetSuccess(bool v) { m_isSuccess = v; }

    void MainSceneControl();

    void ChangeMainState(EMainState next);

	void OnStageEnter(EMainState state);
	void OnStageExit(EMainState state);

    void TutorialControl();
    void Stage1Control();
    void Stage2Control();
    void Stage3Control();

///GameFlowEND

///SCORE
    void ScoreUpdate();
    void AddKill();             // Enemy.cpp - Die()
    void OnPlayerHit();         // Player.cpp - 뭐임
    void OnRhythmMiss();        // 리듬 미스 시 호출 (스택 초기화)
    void ScoreReset();          // 씬 바뀔때?

    ETutorialStep GetTutorialStep() const { return m_TutorialStep; }
	void SetTutorialStep(ETutorialStep step) { m_TutorialStep = step; }

    //void RenderInfo();
	std::function<void()> RenderInfo();

    const int GetScore() { return m_currentScore; }
    void SetScore(int num) { m_currentScore = num; }

    const int GetMultiplier() { return m_multiplier; }
    void SetMultiplier(int num) { m_multiplier = num; }
///SCORE END

///HELPER 
    void ForceShowCursor(bool show){
        if (show) { while (ShowCursor(TRUE) < 0) {}
        } else {    while (ShowCursor(FALSE) >= 0) {}  }
    }
///HELPER END

///RANKING -> LOG
    void SaveRankings() const;
    void LoadRankings();
    void AddScore(const std::string& playedAt, int score);
    std::string GetGradeTextureName(int score) const;
    const std::vector<std::pair<std::string, int>>& GetTopScores() const { return m_rankings; }
///RANKING END

	void ShowTutorialPopup();
	void CloseTutorialPopup();
	void NextTutorialStep();
	bool AnyInputDown();

	std::string ToString(EMainState state)
	{
		switch (state)
		{
		case EMainState::None:      return "None";
		case EMainState::Tutorial:  return "Tutorial";
		case EMainState::Stage1:    return "Stage1";
		case EMainState::Stage2:    return "Stage2";
		case EMainState::StageBoss: return "StageBoss";
		default:                    return "Unknown";
		}
	}
};
