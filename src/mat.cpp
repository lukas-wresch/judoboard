#include "mat.h"
#include "tournament.h"
#include "app.h"
#include <sstream>
#include <iomanip>
#include <thread>
#include <algorithm>
#include <cmath>
#include "../ZED/include/log.h"
#include "../external/license/license.h"


using namespace Judoboard;



Mat::Mat(uint32_t ID, const Application* App) : IMat(ID), m_Application(App), m_Window(Application::Name)
{
	Mat::Open();
}



bool Mat::Open()
{
	if (m_Window.IsRunning())
		return true;

	if (m_Thread.joinable())
		m_Thread.join();

	m_FrameCount = 0;

	m_Thread = std::thread([this]()
	{
		//m_Window = Window(Application::Name);//Recreate

		if (!m_Window.OpenWindow())
		{
			ZED::Log::Warn("Could not open window");
			if (Window::IsDisplayConnected())//Should be able to create a window
				return;//Couldn't create a renderer (possibly)
		}

		for (int i = 0; i < 10; i++)
		{
			m_Window.HandleEvents();
			ZED::Core::Pause(10);
		}

		double display_width = (double)m_Window.GetDisplayWidth();
		ZED::Log::Info("Display width is " + std::to_string(display_width));

		auto& renderer = m_Window.GetRenderer();

		if (display_width <= 0.0)//No display?
			display_width = 1920.0;//Simulate 1080p

		double tiny = 20.0, middle = 55.0, large = 70.0, huge = 100.0, gigantic = 170.0, gigantic2 = 230.0;
		m_ScalingFactor = display_width / 1920.0;

		tiny   *= m_ScalingFactor;
		middle *= m_ScalingFactor;
		large  *= m_ScalingFactor;
		huge   *= m_ScalingFactor;
		gigantic  *= m_ScalingFactor;
		gigantic2 *= m_ScalingFactor;

		if (!renderer.InitFonts("assets/fonts/arial.ttf", (int)tiny, (int)middle, (int)large, (int)huge, (int)gigantic, (int)gigantic2))
			ZED::Log::Error("Could not load font");

		if (!m_Logo)
			m_Logo = m_Window.GetRenderer().CreateTexture("assets/logo.png");
		if (!m_Winner)
			m_Winner = m_Window.GetRenderer().CreateTexture("assets/winner.png");

		m_Graphics["winner_blue" ].SetTexture(m_Winner);
		m_Graphics["winner_white"].SetTexture(m_Winner);

		ZED::Log::Info("Logo loaded");

		if (!m_Background)
		{
			m_Background = m_Window.GetRenderer().CreateTexture(100, 100);
			m_Background->CreateGradient(0, 0, 255, 255, 255, 255);
		}

		ZED::Log::Info("Background created");

		if (!m_Logo)
			m_Logo = m_Window.GetRenderer().CreateTexture("assets/logo.png");

		ZED::Log::Info("Logo loaded");
			
		SetAudio(IsSoundEnabled(), GetSoundFilename(), GetAudioDeviceID());

		while (m_Window.IsRunning())
			Mainloop();
	});

	//while (!m_Window.IsRunning() || !m_Background || !m_Background->IsLoaded())
	while (m_FrameCount == 0)
		ZED::Core::Pause(100);

	return m_Window.IsRunning();
}



bool Mat::Close()
{
	if (AreFightersOnMat())
	{
		ZED::Log::Warn("Can not close mat, a match is still ongoing");
		return false;
	}

	if (!m_Window.IsRunning())//Check if the window is running
		ZED::Core::Pause(1000);

	m_Window.CloseWindow();
	
	if (m_Thread.joinable())
		m_Thread.join();

	//Remove texture so that they don't get freed twice
	m_Graphics["winner_blue" ].Reset();
	m_Graphics["winner_white"].Reset();

	m_Background.Delete();
	m_Logo.Delete();
	m_Winner.Delete();

	return true;
}



bool Mat::Reset()
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat())
	{
		ZED::Log::Warn("Can not reset match, the previous match is still ongoing");
		return false;
	}

	m_Scoreboards[0].Reset();
	m_Scoreboards[1].Reset();

	m_HajimeTimer.Reset();
	m_OsaekomiTimer[0].Reset();
	m_OsaekomiTimer[1].Reset();
	m_OsaekomiList.clear();
	m_IsOsaekomi = false;
	m_OsaekomiHolder = Fighter::White;

	m_MateTimestamp = 0;
	m_EndOfOsaekomiTimestamp = 0;
	m_MatchTimeBeforeGoldenScore = 0;

	m_GoldenScore = false;
	m_IsDraw = false;

	m_pMatch = nullptr;

	ZED::Log::Info("Mat got resetted");
	return true;
}



bool Mat::StartMatch(Match* NewMatch, bool UseForce)
{
	if (!NewMatch)
	{
		ZED::Log::Error("Invalid match");
		return false;
	}

	auto guard = m_mutex.LockWriteForScope();

	if (IsPaused())
	{
		ZED::Log::Warn("Can not start match since mat is paused");
		return false;
	}

	if (!NewMatch->IsAssociatedWithMat() || NewMatch->GetMatID() != GetMatID())
	{
		ZED::Log::Warn("Match does not belong to this mat");
		return false;
	}

	if (!UseForce)
		if (NewMatch->IsRunning() || NewMatch->HasConcluded())
	{
		ZED::Log::Warn("Match is already running or has been concluded");
		return false;
	}

	if (!NewMatch->HasValidFighters())
	{
		ZED::Log::Warn("Can not start a match with invalid fighters");
		return false;
	}

	if (!UseForce)
	  if (NewMatch->GetFighter(Fighter::White)->NeedsBreak() || NewMatch->GetFighter(Fighter::Blue)->NeedsBreak())
	{
		ZED::Log::Warn("Can not start a match since at least one fighter has not had his break yet");
		return false;
	}

	if (!Reset())
	{
		ZED::Log::Warn("Could not reset mat");
		return false;
	}

	m_White = *NewMatch->GetFighter(Fighter::White);
	m_Blue  = *NewMatch->GetFighter(Fighter::Blue);

	m_pMatch = NewMatch;
	bool revise_match = false;

	if (NewMatch->HasConcluded())
		revise_match = true;
	else if (NewMatch->IsScheduled())
		AddEvent(MatchLog::NeutralEvent::StartMatch);

	NewMatch->StartMatch();//Mark match is 'is running'

	NextState(State::TransitionToMatch);
	assert(m_State == State::TransitionToMatch);
	assert(AreFightersOnMat());

	if (revise_match)//Revise match
	{
		m_HajimeTimer.Set(NewMatch->GetResult().m_Time);//Get match timer

		//Read all events
		const auto& events = NewMatch->GetLog().GetEvents();
		for (auto& ev : events)
		{
			if (ev.m_Group == MatchLog::EventGroup::Neutral)
			{
				auto type = ev.m_Event;
				switch (type)
				{
				case MatchLog::NeutralEvent::EnableGoldenScore:
					SetScoreboard(Fighter::White).m_Ippon = SetScoreboard(Fighter::Blue).m_Ippon = 0;
					m_GoldenScore = true;
					break;
				case MatchLog::NeutralEvent::DisableGoldenScore:
					m_GoldenScore = false;
					break;
				case MatchLog::NeutralEvent::EnableDraw:
					m_IsDraw = true;
					break;
				case MatchLog::NeutralEvent::DisableDraw:
					m_IsDraw = false;
					break;
				}
			}

			else
			{
				Fighter whom = (Fighter)ev.m_Group;
				auto type = ev.m_BiasedEvent;

				switch (type)
				{
				case MatchLog::BiasedEvent::AddIppon:
					SetScoreboard(whom).m_Ippon++;
					break;
				case MatchLog::BiasedEvent::AddWazaari:
					SetScoreboard(whom).m_WazaAri++;
					if (SetScoreboard(whom).m_WazaAri == 2)
						SetScoreboard(whom).m_Ippon++;
					break;
				case MatchLog::BiasedEvent::AddYuko:
					SetScoreboard(whom).m_Yuko++;
					break;
				case MatchLog::BiasedEvent::AddKoka:
					SetScoreboard(whom).m_Koka++;
					break;
				case MatchLog::BiasedEvent::RemoveIppon:
					SetScoreboard(whom).m_Ippon--;
					break;
				case MatchLog::BiasedEvent::RemoveWazaari:
					SetScoreboard(whom).m_WazaAri--;
					break;
				case MatchLog::BiasedEvent::RemoveYuko:
					SetScoreboard(whom).m_Yuko--;
					break;
				case MatchLog::BiasedEvent::RemoveKoka:
					SetScoreboard(whom).m_Koka--;
					break;
				case MatchLog::BiasedEvent::AddShido:
					SetScoreboard(whom).m_Shido++;
					break;
				case MatchLog::BiasedEvent::AddHansokuMake_Direct:
					SetScoreboard(whom).m_HansokuMake = true;
					SetScoreboard(whom).m_HansokuMake_Direct = true;
					break;
				case MatchLog::BiasedEvent::AddHansokuMake_Indirect:
					SetScoreboard(whom).m_HansokuMake = true;
					SetScoreboard(whom).m_HansokuMake_Direct = false;
					break;
				case MatchLog::BiasedEvent::RemoveShido:
					SetScoreboard(whom).m_Shido--;
					break;
				case MatchLog::BiasedEvent::RemoveHansokuMake:
					SetScoreboard(whom).m_HansokuMake = false;
					break;
				case MatchLog::BiasedEvent::AddMedicalExamination:
					SetScoreboard(whom).m_MedicalExamination++;
					break;
				case MatchLog::BiasedEvent::RemoveMedicalExamination:
					SetScoreboard(whom).m_MedicalExamination--;
					break;
				case MatchLog::BiasedEvent::Hantei:
					SetScoreboard(whom).m_Hantei = true;
					break;
				case MatchLog::BiasedEvent::AddDisqualification:
					SetScoreboard(whom).m_Disqualification = Scoreboard::DisqualificationState::Disqualified;
					break;
				case MatchLog::BiasedEvent::AddNoDisqualification:
					SetScoreboard(whom).m_Disqualification = Scoreboard::DisqualificationState::NotDisqualified;
					break;
				case MatchLog::BiasedEvent::RemoveDisqualification:
				case MatchLog::BiasedEvent::RemoveNoDisqualification:
					SetScoreboard(whom).m_Disqualification = Scoreboard::DisqualificationState::Unknown;
					break;
				case MatchLog::BiasedEvent::AddGachi:
					SetScoreboard(whom).m_Gachi = true;
					break;
				case MatchLog::BiasedEvent::RemoveGachi:
					SetScoreboard(whom).m_Gachi = false;
					break;
				case MatchLog::BiasedEvent::HanteiRevoked:
					SetScoreboard( whom).m_Hantei = false;
					break;
				}
			}
		}

		AddEvent(MatchLog::NeutralEvent::ReviseMatch);
	}

	ZED::Log::Info("New match started");
	return true;
}



bool Mat::EndMatch()
{
	auto guard = m_mutex.LockWriteForScope();

	if (HasConcluded())
	{
		if (m_pMatch)//Save result in match
		{
			AddEvent(MatchLog::NeutralEvent::EndMatch);
			m_pMatch->SetResult(GetResult());
			m_pMatch->EndMatch();
		}

		//Reset mat
		NextState(State::TransitionToWaiting);

		Reset();

		ZED::Log::Info("Match ended");

		return true;
	}

	ZED::Log::Warn("Could not end match");
	return false;
}



bool Mat::HasConcluded() const
{
	auto guard = m_mutex.LockReadForScope();

	if ( (m_State == State::Running || m_State == State::TransitionToMatch) && !m_HajimeTimer.IsRunning())
	{
		//Any Hansokumake that has not been decided
		if (GetScoreboard(Fighter::White).IsUnknownDisqualification() || GetScoreboard(Fighter::Blue).IsUnknownDisqualification())
			return false;

		//Double gachi?
		if (GetScoreboard(Fighter::White).m_Gachi && GetScoreboard(Fighter::Blue).m_Gachi)
			return true;

		//Double hansokumake?
		if (GetScoreboard(Fighter::White).m_HansokuMake && GetScoreboard(Fighter::Blue).m_HansokuMake)
			return true;

		//Double ippon?
		if (GetScoreboard(Fighter::White).m_Ippon == 1 && GetScoreboard(Fighter::Blue).m_Ippon == 1)
			return false;

		//Has someone an ippon?
		if (GetScoreboard(Fighter::White).m_Ippon == 1 || GetScoreboard(Fighter::Blue).m_Ippon == 1)
			return true;

		if ((m_GoldenScore || IsOutOfTime()) && GetResult().m_Winner != Winner::Draw)
			return true;

		if (m_IsDraw && IsOutOfTime() && GetResult().m_Winner == Winner::Draw)
			return true;
	}

	return false;
}



ZED::Blob Mat::RequestScreenshot() const
{
	//m_mutex.lock();

	m_RequestScreenshot = true;

	//m_mutex.unlock();

	ZED::Log::Info("Screenshot requested");

	for (int i = 0; m_RequestScreenshot && i < 1000; i++)//Screenshot created?
		ZED::Core::Pause(20);

	ZED::Log::Info("Screenshot done");

	return m_Screenshot;
}



uint32_t Mat::GetTime2Display() const
{
	if (m_GoldenScore)//If golden score
		return m_HajimeTimer.GetElapsedTime();//Always count upwards

	if (m_pMatch)
	{
		const auto match_time = m_pMatch->GetRuleSet().GetMatchTime();

		if (match_time > 0)//If match time is not infinite
			return (match_time*1000 - m_HajimeTimer).GetElapsedTime();
	}

	return m_HajimeTimer.GetElapsedTime();//Infinite match (and default if m_pMatch is nullptr)
}



bool Mat::IsOutOfTime() const
{
	auto guard = m_mutex.LockReadForScope();
	if (!m_pMatch)
		return false;
	return !IsOsaekomi() && m_pMatch->GetRuleSet().IsOutOfTime(m_HajimeTimer.GetElapsedTime(), IsGoldenScore());
}



bool Mat::EnableGoldenScore(bool GoldenScore)
{
	auto guard = m_mutex.LockWriteForScope();

	if (m_GoldenScore == GoldenScore)
		return false;

	//Allowed by rule set?
	if (m_pMatch && !m_pMatch->GetRuleSet().IsGoldenScoreEnabled())
		return false;

	if (GoldenScore)
	{
		m_MatchTimeBeforeGoldenScore = m_HajimeTimer.GetElapsedTime();

		m_HajimeTimer.Reset();
		//In case of double ippon, reset the ippons
		SetScoreboard(Fighter::White).m_Ippon = SetScoreboard(Fighter::Blue).m_Ippon = 0;
		AddEvent(MatchLog::NeutralEvent::EnableGoldenScore);
	}
	else
	{
		if (m_pMatch)
			m_HajimeTimer = m_MatchTimeBeforeGoldenScore;
		AddEvent(MatchLog::NeutralEvent::DisableGoldenScore);
	}

	m_GoldenScore = GoldenScore;
	return true;
}



void Mat::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	auto guard = m_mutex.LockReadForScope();

	auto print_scoreboard = [this, &Yaml](Fighter Who) {
		Yaml << YAML::BeginMap;
		Yaml << YAML::Key << "ippon"   << YAML::Value << GetScoreboard(Who).m_Ippon;
		Yaml << YAML::Key << "wazaari" << YAML::Value << GetScoreboard(Who).m_WazaAri;
		if (m_pMatch && m_pMatch->GetRuleSet().IsYukoEnabled())
			Yaml << YAML::Key << "yuko" << YAML::Value << GetScoreboard(Who).m_Yuko;
		if (m_pMatch && m_pMatch->GetRuleSet().IsKokaEnabled())
			Yaml << YAML::Key << "koka" << YAML::Value << GetScoreboard(Who).m_Koka;

		Yaml << YAML::Key << "shido"   << YAML::Value << (GetScoreboard(Who).m_HansokuMake ? 3 : GetScoreboard(Who).m_Shido);
		Yaml << YAML::Key << "medical" << YAML::Value <<  GetScoreboard(Who).m_MedicalExamination;

		Yaml << YAML::Key << "osaekomi_time"       << YAML::Value << m_OsaekomiTimer[(int)Who].GetElapsedTime();
		Yaml << YAML::Key << "is_osaekomi_running" << YAML::Value << m_OsaekomiTimer[(int)Who].IsRunning();

		if (GetScoreboard(Who).IsUnknownDisqualification())
			Yaml << YAML::Key << "unknown_disqualification" << YAML::Value << true;

		Yaml << YAML::EndMap;
	};

	Yaml << YAML::Key << "white" << YAML::Value;
	print_scoreboard(Fighter::White);
	Yaml << YAML::Key << "blue"  << YAML::Value;
	print_scoreboard(Fighter::Blue);

	Yaml << YAML::Key << "hajime_time" << YAML::Value << GetTime2Display();
	Yaml << YAML::Key << "is_hajime"   << YAML::Value << IsHajime();
	Yaml << YAML::Key << "is_osaekomi" << YAML::Value << IsOsaekomi();

	const auto result = GetResult();
	Yaml << YAML::Key << "was_mate_recent"      << YAML::Value << WasMateRecent();
	Yaml << YAML::Key << "are_fighters_on_mat"  << YAML::Value << AreFightersOnMat();
	Yaml << YAML::Key << "can_next_match_start" << YAML::Value << CanNextMatchStart();
	Yaml << YAML::Key << "has_concluded"        << YAML::Value << HasConcluded();
	Yaml << YAML::Key << "winner"               << YAML::Value << (int)result.m_Winner;
	Yaml << YAML::Key << "is_out_of_time"       << YAML::Value << IsOutOfTime();
	Yaml << YAML::Key << "no_winner_yet"        << YAML::Value << (result.m_Winner == Winner::Draw);
	Yaml << YAML::Key << "is_goldenscore"       << YAML::Value << IsGoldenScore();
	Yaml << YAML::Key << "is_hantei"            << YAML::Value << (GetScoreboard(Fighter::White).m_Hantei || GetScoreboard(Fighter::Blue).m_Hantei);
	Yaml << YAML::Key << "total_time"           << YAML::Value << result.m_Time;

	if (m_pMatch)
	{
		auto rules = m_pMatch->GetRuleSet();
		Yaml << YAML::Key << "yuko_enabled" << rules.IsYukoEnabled();
		Yaml << YAML::Key << "koka_enabled" << rules.IsKokaEnabled();
		Yaml << YAML::Key << "golden_score_enabled" << rules.IsGoldenScoreEnabled();
		Yaml << YAML::Key << "draw_enabled" << rules.IsDrawAllowed();
	}

	Yaml << YAML::EndMap;
}



void Mat::Hajime()
{
	auto guard = m_mutex.LockWriteForScope();

	if (IsHajime() && !IsSonomama())//Already hajime and not a sonomama situation?
		return;

	//Double ippons during golden score?
	if (AreFightersOnMat() && IsGoldenScore() && GetScoreboard(Fighter::White).m_Ippon == 1 && GetScoreboard(Fighter::Blue).m_Ippon == 1)
	{
		SetScoreboard(Fighter::White).m_Ippon = SetScoreboard(Fighter::Blue).m_Ippon = 0;//Remove ippons
	}

	if (AreFightersOnMat() && !IsIppon())
	{
		if (IsOsaekomi())//Yoshi
		{
			m_OsaekomiTimer[(int)GetOsaekomiHolder()].Start();

			m_HajimeTimer.Start();

			AddEvent(MatchLog::NeutralEvent::Yoshi);

			m_Graphics["yoshi"].SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -45.0, [](auto& g) { return g.m_a > 0.0; }));
			m_Graphics["sonomama"].StopAllAnimations().AddAnimation(Animation::CreateLinear(0.0, 0.0, -70.0, [](auto& g) { return g.m_a > 0.0; }));
		}

		else if (!IsOutOfTime() || WasMateRecent())//Normal hajime
		{
			bool hajime_after_match_end = false;

			if (IsOutOfTime() && WasMateRecent())
			{
				hajime_after_match_end = true;
				m_HajimeTimer = GetMaxMatchTime();//Reset timer
			}

			m_HajimeTimer.Start();
			m_OsaekomiTimer[0].Reset();
			m_OsaekomiTimer[1].Reset();

			AddEvent(MatchLog::NeutralEvent::Hajime);

			if (!hajime_after_match_end)
				m_Graphics["hajime"].SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -50.0, [](auto& g) { return g.m_a > 0.0; }));
			m_Graphics["mate"].StopAllAnimations().AddAnimation(Animation::CreateLinear(0.0, 0.0, -90.0, [](auto& g) { return g.m_a > 0.0; }));

			m_Graphics["osaekomi_text"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -15.0));
			m_Graphics["osaekomi_bar_border"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -50.0));
			m_Graphics["osaekomi_bar"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -25.0));
		}
	}

	ZED::Log::Info("Hajime");
}



void Mat::Mate()
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && (IsHajime() || IsOsaekomi()))//Mate can also be called during sonomama
	{
		m_HajimeTimer.Pause();
		m_MateTimestamp = Timer::GetTimestamp();

		if (IsOsaekomi())//Mate during osaekomi?
		{
			const auto osaekomi_holder = GetOsaekomiHolder();
			auto osaekomi_time = m_OsaekomiTimer[(int)GetOsaekomiHolder()].GetElapsedTime();

			if (osaekomi_time > EndTimeOfOsaekomi() * 1000)
				osaekomi_time = EndTimeOfOsaekomi() * 1000;
			m_OsaekomiList.emplace_back(OsaekomiEntry(osaekomi_holder, osaekomi_time));

			m_OsaekomiTimer[(int)osaekomi_holder].Pause();
			m_OsaekomiTimer[(int)osaekomi_holder].Set(osaekomi_time);
			m_EndOfOsaekomiTimestamp = Timer::GetTimestamp();
			m_IsOsaekomi = false;

			//In case mate is called during sonomama
			m_Graphics["sonomama"].StopAllAnimations().AddAnimation(Animation::CreateLinear(0.0, 0.0, -500.0, [](auto& g) { return g.m_a > 0.0; }));

			m_Graphics["osaekomi_text"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -25.0));
			m_Graphics["osaekomi_bar_border"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -50.0));
			m_Graphics["osaekomi_bar"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -25.0));

			m_Graphics["effect_osaekomi_" + Fighter2String(osaekomi_holder)].AddAnimation(Animation::CreateLinear(0.0, 0.0, -30.0, [](auto& g) { return g.m_a > 0.0; }));
		}

		AddEvent(MatchLog::NeutralEvent::Mate);

		m_Graphics["hajime"].StopAllAnimations().AddAnimation(Animation::CreateLinear(0.0, 0.0, -80.0, [](auto& g) { return g.m_a > 0.0; }));

		if (!HasConcluded())//Only show mate when the match can continue (otherwise it would be soremade)
			m_Graphics["mate"].SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -30.0, [](auto& g) { return g.m_a > 0.0; }));

		m_Graphics["effect_osaekomi_white"].StopAllAnimations().AddAnimation(Animation::CreateLinear(0.0, 0.0, -80.0, [](auto& g) { return g.m_a > 0.0; }));
		m_Graphics["effect_osaekomi_blue" ].StopAllAnimations().AddAnimation(Animation::CreateLinear(0.0, 0.0, -80.0, [](auto& g) { return g.m_a > 0.0; }));
		m_Graphics["effect_tokeda_white"  ].StopAllAnimations().AddAnimation(Animation::CreateLinear(0.0, 0.0, -80.0, [](auto& g) { return g.m_a > 0.0; }));
		m_Graphics["effect_tokeda_blue"   ].StopAllAnimations().AddAnimation(Animation::CreateLinear(0.0, 0.0, -80.0, [](auto& g) { return g.m_a > 0.0; }));
	}

	ZED::Log::Info("Mate");
}



void Mat::Sonomama()
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && IsHajime() && IsOsaekomiRunning())
	{
		m_HajimeTimer.Pause();
		m_OsaekomiTimer[0].Pause();
		m_OsaekomiTimer[1].Pause();

		AddEvent(MatchLog::NeutralEvent::Sonomama);

		m_Graphics["sonomama"].SetAlpha(255).StopAllAnimations().AddAnimation(Animation::CreateLinear(0.0, 0.0, -10.0, [](auto& g) { return g.m_a > 0.0; }));
	}
}



void Mat::AddIppon(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_Ippon == 0)
	{
		SetScoreboard(Whom).m_Ippon   = 1;
		//SetScoreboard(Whom).m_WazaAri = 0;

		if (GetScoreboard(Whom).m_WazaAri < 2)//If its not a wazari awasete ippon
			AddEvent(Whom, MatchLog::BiasedEvent::AddIppon);

		Mate();

		if (!GetScoreboard(!Whom).m_HansokuMake)//Don't show ippon effect if its due to an hansokumake
			m_Graphics["effect_ippon_" + Fighter2String(Whom)].SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -15.0, [](auto& g) { return g.m_a > 0.0; }));
	}

	ZED::Log::Info("Ippon");
}



void Mat::RemoveIppon(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_Ippon >= 1)
	{
		SetScoreboard(Whom).m_Ippon = 0;
		if (GetScoreboard(Whom).m_WazaAri == 2)//If it was created via awasete ippon
			SetScoreboard(Whom).m_WazaAri = 1;

		AddEvent(Whom, MatchLog::BiasedEvent::RemoveIppon);

		m_Graphics["effect_ippon_" + Fighter2String(Whom)].SetAlpha(0);
	}
}



void Mat::AddWazaAri(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_Ippon == 0)
	{
		SetScoreboard(Whom).m_WazaAri++;

		AddEvent(Whom, MatchLog::BiasedEvent::AddWazaari);

		m_Graphics["effect_wazaari_" + Fighter2String(Whom)].SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -20.0, [](auto& g) { return g.m_a > 0.0; }));

		if (GetScoreboard(Whom).m_WazaAri >= 2)//Waza-Ari awasete ippon?
		{
			AddEvent(Whom, MatchLog::BiasedEvent::WazariAwaseteIppon);

			AddIppon(Whom);
		}
		else if (m_GoldenScore)
			Mate();
	}

	ZED::Log::Debug("Wazaari");
}



void Mat::RemoveWazaAri(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && (GetScoreboard(Whom).m_WazaAri > 0 || GetScoreboard(Whom).m_Ippon > 0) )
	{
		if (GetScoreboard(Whom).m_Ippon > 0)
		{
			RemoveIppon(Whom);

			SetScoreboard(Whom).m_WazaAri = 1;
			AddEvent(Whom, MatchLog::BiasedEvent::AddWazaari);
		}
		else if (GetScoreboard(Whom).m_WazaAri > 0)
		{
			SetScoreboard(Whom).m_WazaAri--;
			AddEvent(Whom, MatchLog::BiasedEvent::RemoveWazaari);
		}


		m_Graphics["effect_wazaari_" + Fighter2String(Whom)].SetAlpha(0);

		if (WasEndOfOsaekomiRecent() && GetScoreboard(Whom).m_WazaAri > 0)//Remove wazari shortly after an osaekomi ended?
		{
			//Remove the first wazaari
			SetScoreboard(Whom).m_WazaAri--;
			AddEvent(Whom, MatchLog::BiasedEvent::RemoveWazaari);

			const auto osaekomi_holder = GetOsaekomiHolder();

			assert(m_OsaekomiList.size() >= 1);

			//Shift hajime and osaekomi timer for lost time
			auto shift = Timer::GetTimestamp() - m_EndOfOsaekomiTimestamp;
			m_HajimeTimer.Shift(shift);
			m_OsaekomiTimer[(int)osaekomi_holder].Shift(shift);

			//Continue osaekomi
			m_OsaekomiTimer[(int)osaekomi_holder].Start();
			m_IsOsaekomi = true;

			//Since mate was called by mistake
			Hajime();//Has to be called at the end since it resets the osaekomi timer

			m_Graphics["effect_ippon_"  + Fighter2String(osaekomi_holder)].SetAlpha(0);

			m_Graphics["hajime"].SetAlpha(0);
			m_Graphics["mate"].SetAlpha(0);

			if (IsSoundEnabled())
				StopSoundFile();
		}

		if (IsOsaekomi() && GetOsaekomiHolder() == Whom)
		{
			m_Graphics["osaekomi_bar"].m_width = 0;//We have to reset and recalculate the osaekomi bar
		}
	}
}



void Mat::AddYuko(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && m_pMatch && m_pMatch->GetRuleSet().IsYukoEnabled())
	{
		SetScoreboard(Whom).m_Yuko++;

		AddEvent(Whom, MatchLog::BiasedEvent::AddYuko);	

		m_Graphics["effect_yuko_" + Fighter2String(Whom)].SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -25.0, [](auto& g) { return g.m_a > 0.0; }));

		if (m_GoldenScore)
			Mate();
	}

	ZED::Log::Debug("Yuko");
}



void Mat::RemoveYuko(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_Yuko > 0 && m_pMatch && m_pMatch->GetRuleSet().IsYukoEnabled())
	{
		SetScoreboard(Whom).m_Yuko--;

		AddEvent(Whom, MatchLog::BiasedEvent::RemoveYuko);

		m_Graphics["effect_yuko_" + Fighter2String(Whom)].SetAlpha(0);
	}
}



void Mat::AddKoka(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && m_pMatch && m_pMatch->GetRuleSet().IsKokaEnabled())
	{
		SetScoreboard(Whom).m_Koka++;

		AddEvent(Whom, MatchLog::BiasedEvent::AddKoka);

		m_Graphics["effect_koka_" + Fighter2String(Whom)].SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -25.0, [](auto& g) { return g.m_a > 0.0; }));

		if (m_GoldenScore)
			Mate();
	}

	ZED::Log::Debug("Koka");
}



void Mat::RemoveKoka(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_Koka > 0 && m_pMatch && m_pMatch->GetRuleSet().IsKokaEnabled())
	{
		SetScoreboard(Whom).m_Koka--;

		AddEvent(Whom, MatchLog::BiasedEvent::RemoveKoka);

		m_Graphics["effect_koka_" + Fighter2String(Whom)].SetAlpha(0);
	}
}



void Mat::Hantei(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && !GetScoreboard(Whom).m_Hantei)
	{
		SetScoreboard(Whom).m_Hantei  = true;
		SetScoreboard(!Whom).m_Hantei = false;

		AddEvent(Whom, MatchLog::BiasedEvent::Hantei);
	}

	ZED::Log::Info("Hantei");
}



void Mat::RevokeHantei()
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && (GetScoreboard(Fighter::White).m_Hantei || GetScoreboard(Fighter::Blue).m_Hantei))
	{
		if (GetScoreboard(Fighter::White).m_Hantei)
			AddEvent(Fighter::White, MatchLog::BiasedEvent::HanteiRevoked);
		if (GetScoreboard(Fighter::Blue).m_Hantei)
			AddEvent(Fighter::Blue,  MatchLog::BiasedEvent::HanteiRevoked);

		SetScoreboard(Fighter::White).m_Hantei = false;
		SetScoreboard(Fighter::Blue ).m_Hantei = false;
	}

	ZED::Log::Info("Revoke Hantei");
}



void Mat::SetAsDraw(bool Enable)
{
	auto guard = m_mutex.LockWriteForScope();

	if (m_IsDraw == Enable)
		return;

	if (AreFightersOnMat() && GetResult().m_Winner == Winner::Draw && m_pMatch && m_pMatch->GetRuleSet().IsDrawAllowed())
	{
		if (Enable)
		{
			AddEvent(MatchLog::NeutralEvent::EnableDraw);
			m_IsDraw = true;
		}
		else
		{
			AddEvent(MatchLog::NeutralEvent::DisableDraw);
			m_IsDraw = false;
		}
	}

	ZED::Log::Debug("Draw");
}



void Mat::AddShido(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_Shido < 3)
	{
		SetScoreboard(Whom).m_Shido++;

		AddEvent(Whom, MatchLog::BiasedEvent::AddShido);
		m_Graphics["effect_shido_" + Fighter2String(Whom)].SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -20.0, [](auto& g) { return g.m_a > 0.0; }));

		if (GetScoreboard(Whom).m_Shido == 3)//Hansoku-make
			AddHansokuMake(Whom, false);//Add indirect hansokumake
	}

	ZED::Log::Debug("Shido");
}



void Mat::RemoveShido(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_Shido > 0)
	{
		AddEvent(Whom, MatchLog::BiasedEvent::RemoveShido);
		m_Graphics["effect_shido_" + Fighter2String(Whom)].StopAllAnimations().SetAlpha(0);

		if (GetScoreboard(Whom).m_HansokuMake)
			RemoveHansokuMake(Whom);

		SetScoreboard(Whom).m_Shido--;
	}
}



void Mat::AddHansokuMake(Fighter Whom, bool Direct)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && !GetScoreboard(Whom).m_HansokuMake)
	{
		SetScoreboard(Whom).m_HansokuMake = true;
		SetScoreboard(Whom).m_HansokuMake_Direct = Direct;

		if (Direct)
		{
			SetScoreboard(Whom).m_Disqualification = Scoreboard::DisqualificationState::Unknown;
			AddEvent(Whom, MatchLog::BiasedEvent::AddHansokuMake_Direct);
		}
		else
		{
			SetScoreboard(Whom).m_Disqualification = Scoreboard::DisqualificationState::NotDisqualified;
			AddEvent(Whom, MatchLog::BiasedEvent::AddHansokuMake_Indirect);
		}

		m_Graphics["effect_hansokumake_" + Fighter2String(Whom)].SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -10.0, [](auto& g) { return g.m_a > 0.0; }));

		AddIppon(!Whom);
	}

	ZED::Log::Info("Hansokumake");
}



void Mat::RemoveHansokuMake(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_HansokuMake)
	{
		SetScoreboard(Whom).m_HansokuMake = false;
		SetScoreboard(Whom).m_HansokuMake_Direct = false;

		AddEvent(Whom, MatchLog::BiasedEvent::RemoveHansokuMake);
		m_Graphics["effect_hansokumake_" + Fighter2String(Whom)].StopAllAnimations().SetAlpha(0);

		RemoveIppon(!Whom);//Remove the ippon of the other fighter (that he got due to the HansokuMake)
	}
}



void Mat::AddDisqualification(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_HansokuMake && GetScoreboard(Whom).m_HansokuMake_Direct && !GetScoreboard(Whom).IsDisqualified())
	{
		SetScoreboard(Whom).m_Disqualification = Scoreboard::DisqualificationState::Disqualified;

		AddEvent(Whom, MatchLog::BiasedEvent::AddDisqualification);
	}

	ZED::Log::Info("Disqualification");
}



void Mat::AddNoDisqualification(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_HansokuMake && GetScoreboard(Whom).m_HansokuMake_Direct && GetScoreboard(Whom).IsDisqualified())
	{
		//Can we simply remove the previous disqualification?
		if (m_pMatch && !m_pMatch->HasConcluded())
		{
			SetScoreboard(Whom).m_Disqualification = Scoreboard::DisqualificationState::Unknown;
			AddEvent(Whom, MatchLog::BiasedEvent::RemoveDisqualification);
		}
	}


	if (AreFightersOnMat() && GetScoreboard(Whom).m_HansokuMake && GetScoreboard(Whom).m_HansokuMake_Direct && !GetScoreboard(Whom).IsDisqualified())
	{
		SetScoreboard(Whom).m_Disqualification = Scoreboard::DisqualificationState::NotDisqualified;
		AddEvent(Whom, MatchLog::BiasedEvent::AddNoDisqualification);
	}

	ZED::Log::Info("No Disqualification");
}



void Mat::RemoveDisqualification(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).IsDisqualified())
	{
		SetScoreboard(Whom).m_Disqualification = Scoreboard::DisqualificationState::Unknown;
		AddEvent(Whom, MatchLog::BiasedEvent::RemoveDisqualification);
	}
}



void Mat::RemoveNoDisqualification(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).IsNotDisqualified())
	{
		SetScoreboard(Whom).m_Disqualification = Scoreboard::DisqualificationState::Unknown;

		AddEvent(Whom, MatchLog::BiasedEvent::RemoveNoDisqualification);
	}
}



void Mat::AddMedicalExamination(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_MedicalExamination < 2)
	{
		SetScoreboard(Whom).m_MedicalExamination++;
		AddEvent(Whom, MatchLog::BiasedEvent::AddMedicalExamination);
	}
}



void Mat::RemoveMedicalExamination(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_MedicalExamination > 0)
	{
		SetScoreboard(Whom).m_MedicalExamination--;
		AddEvent(Whom, MatchLog::BiasedEvent::RemoveMedicalExamination);
	}
}



void Mat::AddGachi(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && !GetScoreboard(Whom).m_Gachi)
	{
		SetScoreboard(Whom).m_Gachi = true;

		AddEvent(Whom, MatchLog::BiasedEvent::AddGachi);

		AddIppon(!Whom);
	}
}



void Mat::RemoveGachi(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && GetScoreboard(Whom).m_Gachi)
	{
		SetScoreboard(Whom).m_Gachi = false;

		AddEvent(Whom, MatchLog::BiasedEvent::RemoveGachi);
	}
}



void Mat::Osaekomi(Fighter Whom)
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat())
	{
		if (IsHajime() && !IsOsaekomiRunning())//No Osaekomi time yet, start new osaekomi
		{
			m_OsaekomiTimer[(int)Whom].Reset();
			m_OsaekomiTimer[(int)Whom].Start();
			m_IsOsaekomi = true;
			m_OsaekomiHolder = Whom;

			AddEvent(Whom, MatchLog::BiasedEvent::Osaekomi);

			m_Graphics["osaekomi_text"].StopAllAnimations().SetPosition(0, 0, 255);
			m_Graphics["osaekomi_bar_border"].StopAllAnimations().SetPosition(0, 0, 0).SetAlpha(255);
			m_Graphics["osaekomi_bar"].StopAllAnimations().SetPosition(0, 0, 0).SetAlpha(255);
			m_Graphics["osaekomi_bar"].SetWidth(0);

			m_Graphics["effect_osaekomi_" + Fighter2String(Whom)].StopAllAnimations().SetAlpha(255);
		}

		else if (IsOsaekomi() && Whom != GetOsaekomiHolder())//Switch osaekomi
		{
			AddEvent(MatchLog::NeutralEvent::OsaekomiSwitched);

			m_OsaekomiTimer[(int)Whom] = m_OsaekomiTimer[(int)!Whom];
			m_OsaekomiTimer[(int)!Whom].Stop();
			m_OsaekomiHolder = Whom;

			m_Graphics["osaekomi_bar"].m_width = 0;//Recalculate osaekomi bar

			m_Graphics["effect_osaekomi_" + Fighter2String(!Whom)].StopAllAnimations().SetAlpha(0);
			m_Graphics["effect_osaekomi_" + Fighter2String(Whom) ].StopAllAnimations().SetAlpha(255);
		}

		else if (!IsHajime() && WasMateRecent())
		{
			Hajime();
			if (IsHajime())//Hajime() could fail, if someone has an ippon -> stack overflow
				Osaekomi(Whom);

			if (IsSoundEnabled())
				StopSoundFile();
		}
	}
}



uint32_t Mat::GetMaxMatchTime()
{
	auto guard = m_mutex.LockReadForScope();

	if (!m_pMatch)
		return 0;

	if (!IsGoldenScore())
		return m_pMatch->GetRuleSet().GetMatchTime() * 1000;
	return m_pMatch->GetRuleSet().GetGoldenScoreTime() * 1000;
}



void Mat::PlaySoundFile() const
{
	if (!IsSoundEnabled()) return;

	if (!m_AudioDevice.IsValid() || m_AudioDevice.GetDeviceIndex() != GetAudioDeviceID())
		m_AudioDevice = ZED::SoundDevice(GetAudioDeviceID());

	assert(m_AudioDevice.IsValid());
	assert(m_Application);
	if (m_Application)
	{
		auto sound = m_Application->GetSound(GetSoundFilename());
		assert(sound);
		assert(sound->IsValid());
		if (sound)
			m_AudioDevice.Play(*sound);
	}
}



void Mat::Tokeda()
{
	auto guard = m_mutex.LockWriteForScope();

	if (AreFightersOnMat() && IsOsaekomi())
	{
		const auto osaekomi_holder = GetOsaekomiHolder();

		m_OsaekomiList.emplace_back(OsaekomiEntry(osaekomi_holder, m_OsaekomiTimer[(int)osaekomi_holder].GetElapsedTime()));

		m_OsaekomiTimer[(int)osaekomi_holder].Pause();
		m_IsOsaekomi = false;

		if (IsOutOfTime() && IsHajime())
		{
			Mate();
			PlaySoundFile();
		}

		AddEvent(MatchLog::NeutralEvent::Tokeda);

		m_Graphics["osaekomi_text"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -15.0));
		m_Graphics["osaekomi_bar_border"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -50.0));
		m_Graphics["osaekomi_bar"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -25.0));

		m_Graphics["effect_osaekomi_" + Fighter2String(osaekomi_holder)].AddAnimation(Animation::CreateLinear(0.0, 0.0, -55.0, [](auto& g) { return g.m_a > 0.0; }));
		m_Graphics["effect_tokeda_"   + Fighter2String(osaekomi_holder)].StopAllAnimations().SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -25.0, [](auto& g) { return g.m_a > 0.0; }));
	}

	else if (AreFightersOnMat() && WasMateRecent())//Tokeda after mate?
	{
		const auto osaekomi_holder = GetOsaekomiHolder();
		if (m_OsaekomiTimer[(int)osaekomi_holder].GetElapsedTime() >= EndTimeOfOsaekomi() * 1000)
		{
			assert(m_OsaekomiList.size() >= 1);

			if (!m_OsaekomiList.empty())
				m_OsaekomiList[m_OsaekomiList.size() - 1].m_Time--;
			m_OsaekomiTimer[(int)osaekomi_holder].Shift(-1);

			AddEvent(MatchLog::NeutralEvent::Tokeda);

			SetScoreboard(osaekomi_holder).m_Ippon = 0;

			//m_Graphics["osaekomi_bar"].m_width = 0;//To force an update
			//UpdateOsaekomiGraphics();

			//Shift hajime timer for lost time
			m_HajimeTimer.Shift(Timer::GetTimestamp() - m_MateTimestamp);
			//Since mate was called by mistake
			Hajime();//Has to be called at the end since it resets the osaekomi timer

			m_Graphics["effect_tokeda_" + Fighter2String(osaekomi_holder)].StopAllAnimations().SetAlpha(255).AddAnimation(Animation::CreateLinear(0.0, 0.0, -25.0, [](auto& g) { return g.m_a > 0.0; }));
			m_Graphics["effect_ippon_"  + Fighter2String(osaekomi_holder)].SetAlpha(0);

			m_Graphics["hajime"].SetAlpha(0);
			m_Graphics["mate"].SetAlpha(0);

			m_Graphics["osaekomi_text"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -15.0));
			m_Graphics["osaekomi_bar_border"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -50.0));
			m_Graphics["osaekomi_bar"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -25.0));

			if (IsSoundEnabled())
				StopSoundFile();
		}
	}
}



const std::vector<Match> Mat::GetNextMatches() const
{
	m_mutex.LockRead();
	auto ret = m_NextMatches;
	m_mutex.UnlockRead();
	return ret;
}



uint32_t Mat::EndTimeOfOsaekomi() const
{
	auto guard = m_mutex.LockReadForScope();
	const bool hasWazaari = GetScoreboard(GetOsaekomiHolder()).m_WazaAri >= 1;
	return m_pMatch->GetRuleSet().GetOsaeKomiTime(hasWazaari);
}



void Mat::Process()
{
	if (!AreFightersOnMat())
		return;

	m_mutex.LockRead();

	if (IsOsaekomiRunning() && m_OsaekomiTimer[(int)GetOsaekomiHolder()].GetElapsedTime() >= EndTimeOfOsaekomi() * 1000)
	{
		m_mutex.LockWrite();

		if (GetScoreboard(GetOsaekomiHolder()).m_WazaAri == 1)
			AddWazaAri(GetOsaekomiHolder());
		else
			AddIppon(GetOsaekomiHolder());

		UpdateGraphics();
      
		m_mutex.UnlockWrite();
    
		if (IsSoundEnabled())
			PlaySoundFile();
	}

	m_mutex.UnlockRead();

	if (IsOutOfTime() && IsHajime())
	{
		Mate();
		m_HajimeTimer = GetMaxMatchTime();//Don't overflow timer

		if (IsSoundEnabled())
			PlaySoundFile();
	}
}



void Mat::RenderBackgroundEffect(float alpha) const
{
	const float PI = 3.14159265358979323846f;

	if (alpha >= 359.5f)
		alpha = 359.5f;
	if (alpha <= 270.5f)
		alpha = 270.5f;
	float beta = alpha - 270.0f;

	if (!m_Background)
		return;

	m_Background->SetSizeX(1.0f);
	m_Background->SetSizeY(1.0f);
	m_Background->SetAngle(alpha);

	float m = std::tan(beta / (180.0f / PI));

	auto& renderer = m_Window.GetRenderer();
	const int width  = renderer.GetWidth();
	const int height = renderer.GetHeight();

	renderer.FillRect(ZED::Rect(0, 0, width, height), 0, 0, 255);

	if (m > 0.0f && m < 1.0f)
	{
		float x = width/2.0f + height/(2.0f * m);
		float y = 0.0f;

		float delta = 12.0f;
		if (m < 0.1f)
			delta = 180.0f;

		for (; y < height + 50.0f; y += delta * m, x -= delta)
		{
			int w = (int)x - 50 + 20;
			int h = (int)y - 50 + 20;

			if (w > 0 && h > 0)
				renderer.FillRect(ZED::Rect(0, 0, w, h), 255, 255, 255);
		}

		y = 0.0f;
		for (float x = width / 2.0f + height / (2.0f * m); y < height + 50.0f; y += 50.0f * m, x -= 50.0f)
			renderer.RenderTransformedRotated(*m_Background, (int)x - 50, (int)y - 50);
	}
	else
	{
		float y = 0.0f;
		for (float x = width / 2.0f + height / (2.0f * m); y < height + 50.0f; y += 10.0f, x -= 10.0f / m)
		{
			int w = (int)x - 50 + 20;
			int h = (int)y - 50 + 20;

			if (w > 0 && h > 0)
				renderer.FillRect(ZED::Rect(0, 0, w, h), 255, 255, 255);
		}

		y = 0.0f;
		for (float x = width / 2.0f + height / (2.0f * m); y < height + 50.0f; y += 50.0f, x -= 50.0f / m)
			renderer.RenderTransformedRotated(*m_Background, (int)x - 50, (int)y - 50);
	}
}



bool Mat::Animation::Process(GraphicElement& Graphic, double dt)
{
	if (m_AnimateTill && !m_AnimateTill(Graphic))
		return true;//Cancel animation

	switch (m_Type)
	{
		case Type::MoveConstantVelocity:
			Graphic.m_x += m_vx * dt;
			Graphic.m_y += m_vy * dt;
			Graphic.m_a += m_va * dt;
			return false;

		case Type::ScaleSinus:
			if (Graphic.m_a <= 0.0)
				return false;

			Graphic->SetSize((float)(m_BaseSize + m_Amplitude * sin(m_Frequency * (double)Timer::GetTimestamp())) );
			m_Amplitude -= dt * m_Amplitude * m_DampingQuadratic;
			m_Amplitude -= dt * m_DampingLinear;
			if (m_Amplitude < 0.0)
			{
				m_Amplitude = 0.0;
				return true;
			}
			return false;
	}

	return true;
}



void Mat::NextState(State NextState) const
{
	if (m_State == NextState)
		return;

	auto guard = m_mutex.LockWriteForScope();

	if (m_State == State::StartUp && NextState == State::TransitionToMatch)
		this->NextState(State::TransitionToWaiting);
	if (m_State == State::TransitionToWaiting && NextState == State::TransitionToMatch)
		this->NextState(State::Waiting);

	m_CurrentStateStarted = Timer::GetTimestamp();

	auto width  = m_Window.GetRenderer().GetWidth();
	auto height = m_Window.GetRenderer().GetHeight();

	const int name_height   = (int)(30.0 * m_ScalingFactor);
	const int club_height   = name_height + (int)(100.0 * m_ScalingFactor);

	const int score_height  = (int)(300.0 * m_ScalingFactor);
	const int score_margin  = (int)(120.0 * m_ScalingFactor);
	const int score_padding = (int)(200.0 * m_ScalingFactor);

	const int effect_row1 = height/2    + (int)(55.0 * m_ScalingFactor);
	const int effect_row2 = effect_row1 + (int)(90.0 * m_ScalingFactor);
	const int effect_row3 = effect_row2 + (int)(90.0 * m_ScalingFactor);

	auto& renderer = m_Window.GetRenderer();

	switch (NextState)
	{
		case State::StartUp:
			m_Graphics["mat_name"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -30.0, [](auto& g) { return g.m_a > 0.0; }));
			break;

		case State::Waiting:
			for (auto& g : m_Graphics)
				g.second.StopAllAnimations();

			m_Graphics["hajime"].SetAlpha(0.0);
			m_Graphics["mate"].SetAlpha(0.0);
			m_Graphics["sonomama"].SetAlpha(0.0);
			m_Graphics["yoshi"].SetAlpha(0.0);

			m_Graphics["effect_ippon_white"].SetAlpha(0.0);
			m_Graphics["effect_ippon_blue" ].SetAlpha(0.0);

			m_Graphics["effect_wazaari_white"].SetAlpha(0.0);
			m_Graphics["effect_wazaari_blue" ].SetAlpha(0.0);

			m_Graphics["effect_yuko_white"].SetAlpha(0.0);
			m_Graphics["effect_yuko_blue" ].SetAlpha(0.0);

			m_Graphics["effect_koka_white"].SetAlpha(0.0);
			m_Graphics["effect_koka_blue" ].SetAlpha(0.0);

			m_Graphics["effect_osaekomi_white"].SetAlpha(0.0);
			m_Graphics["effect_osaekomi_blue" ].SetAlpha(0.0);

			m_Graphics["effect_tokeda_white"].SetAlpha(0.0);
			m_Graphics["effect_tokeda_blue" ].SetAlpha(0.0);

			m_Graphics["effect_shido_white"].SetAlpha(0.0);
			m_Graphics["effect_shido_blue" ].SetAlpha(0.0);

			m_Graphics["effect_hansokumake_white"].SetAlpha(0.0);
			m_Graphics["effect_hansokumake_blue" ].SetAlpha(0.0);
			break;

		case State::TransitionToMatch:
		{
			for (int i = 0; i < 2; i++)
			{
				m_Graphics["next_matches_white_" + std::to_string(i)].AddAnimation(Animation::CreateLinear(+100.0, 0.0, -83.0));
				m_Graphics["next_matches_blue_"  + std::to_string(i)].AddAnimation(Animation::CreateLinear(-100.0, 0.0, -83.0));
				m_Graphics["next_matches_white2_" + std::to_string(i)].AddAnimation(Animation::CreateLinear(+100.0, 0.0, -83.0));
				m_Graphics["next_matches_blue2_"  + std::to_string(i)].AddAnimation(Animation::CreateLinear(-100.0, 0.0, -83.0));
				m_Graphics["next_matches_white_club_" + std::to_string(i)].AddAnimation(Animation::CreateLinear(+100.0, 0.0, -83.0));
				m_Graphics["next_matches_blue_club_"  + std::to_string(i)].AddAnimation(Animation::CreateLinear(-100.0, 0.0, -83.0));
			}

			m_Graphics["blue_name"].SetPosition(50, name_height-620, 80)
				.AddAnimation(Animation::CreateLinear(0.0, 67.0, 20.0, [=](auto& g) { return g.m_y < name_height; }));

			m_Graphics["white_name"].SetPosition(width/2 + 60, name_height-620, 80)
				.AddAnimation(Animation::CreateLinear(0.0, 67.0, 20.0, [=](auto& g) { return g.m_y < name_height; }));

			m_Graphics["blue_club"].SetPosition(50, club_height, -150)
				.AddAnimation(Animation::CreateLinear(0.0, 0.0, 30.0, [](auto& g) { return g.m_a < 255.0; }));

			m_Graphics["white_club"].SetPosition(width/2 + 60, club_height, -150)
				.AddAnimation(Animation::CreateLinear(0.0, 0.0, 30.0, [](auto& g) { return g.m_a < 255.0; }));


			//Scores
			double a = 25.0;

			m_Graphics["blue_ippon"].SetPosition(score_margin, score_height - 500, 80).Center()
				.AddAnimation(Animation::CreateLinear(0.0, 67.0, a, [=](auto& g) { return g.m_y < score_height; }));

			m_Graphics["blue_wazari"].SetPosition(score_margin + score_padding, score_height - 500, 80).Center()
				.AddAnimation(Animation::CreateLinear(0.0, 67.0, a, [=](auto& g) { return g.m_y < score_height; }));

			m_Graphics["blue_yuko"].SetPosition(score_margin + 2*score_padding, score_height - 500, 80).Center()
				.AddAnimation(Animation::CreateLinear(0.0, 67.0, a, [=](auto& g) { return g.m_y < score_height; }));

			m_Graphics["blue_koka"].SetPosition(score_margin + 3*score_padding, score_height - 500, 80).Center()
				.AddAnimation(Animation::CreateLinear(0.0, 67.0, a, [=](auto& g) { return g.m_y < score_height; }));

			m_Graphics["white_ippon"].SetPosition(width/2 + score_margin, score_height - 500, 80).Center()
				.AddAnimation(Animation::CreateLinear(0.0, 67.0, a,  [=](auto& g) { return g.m_y < score_height; }));

			m_Graphics["white_wazari"].SetPosition(width/2 + score_margin + score_padding, score_height - 500, 80).Center()
				.AddAnimation(Animation::CreateLinear(0.0, 67.0, a, [=](auto& g) { return g.m_y < score_height; }));

			m_Graphics["white_yuko"].SetPosition(width/2 + score_margin + 2*score_padding, score_height - 500, 80).Center()
				.AddAnimation(Animation::CreateLinear(0.0, 67.0, a, [=](auto& g) { return g.m_y < score_height; }));

			m_Graphics["white_koka"].SetPosition(width/2 + score_margin + 3*score_padding, score_height - 500, 80).Center()
				.AddAnimation(Animation::CreateLinear(0.0, 67.0, a, [=](auto& g) { return g.m_y < score_height; }));


			if (GetIpponStyle() == IpponStyle::SpelledOut)
			{
				m_Graphics["white_ippon"].m_x += (int)(100.0 * m_ScalingFactor);
				m_Graphics["blue_ippon"].m_x  += (int)(100.0 * m_ScalingFactor);
			}


			//Etc.
			m_Graphics["timer"].SetPosition(width/2, height/2, -120).Center()
				.AddAnimation(Animation::CreateLinear(0.0, 0.0, 33.0, [](auto& g) { return g.m_a < 255.0; }));

			m_Graphics["mat_name"].AddAnimation(Animation::CreateLinear(0.0, 0.0, 30.0, [](auto& g) { return g.m_a < 255.0; }));

			m_Graphics["next_match"].StopAllAnimations()
				.AddAnimation(Animation::CreateLinear(0.0, 0.0, -38.0, [](auto& g) { return g.m_a > 0.0; }));

			m_Graphics["following_match"].StopAllAnimations()
				.AddAnimation(Animation::CreateLinear(0.0, 0.0, -40.0, [](auto& g) { return g.m_a > 0.0; }));

			//Setup rectangle for age group
			if (m_pMatch && m_pMatch->GetMatchTable() && m_pMatch->GetMatchTable()->GetAgeGroup())
			{
				auto name = m_pMatch->GetMatchTable()->GetAgeGroup()->GetName();
				int pos_y = (int)(300.0 * m_ScalingFactor);

				m_Graphics["age_group_rect_text"].UpdateTexture(renderer, name, ZED::Color(0, 0, 0), ZED::FontSize::Gigantic)
					.StopAllAnimations().Center().SetPosition(width/2, height/2 + pos_y, -100)
					.AddAnimation(Animation::CreateLinear(0.0, 0.0,  45.0, [](auto& g) { return g.m_a < 400.0; }))
					.AddAnimation(Animation::CreateLinear(0.0, 0.0, -30.0, [](auto& g) { return g.m_a >   0.0; }));

				auto text_width = m_Graphics["age_group_rect_text"]->GetWidth();

				m_Graphics["age_group_rect"].SetColor(ZED::Color(255, 255, 0))
					.StopAllAnimations().Center().SetPosition(width/2 - (text_width + 10)/2, height/2 + pos_y - (int)(80.0 * m_ScalingFactor), -100)
					.SetSize(text_width + 20, (int)(180.0 * m_ScalingFactor))
					.AddAnimation(Animation::CreateLinear(0.0, 0.0,  40.0, [](auto& g) { return g.m_a < 400.0; }))
					.AddAnimation(Animation::CreateLinear(0.0, 0.0, -30.0, [](auto& g) { return g.m_a >   0.0; }));

				m_Graphics["age_group_rect_bar"].SetColor(ZED::Color(0, 0, 0))
					.StopAllAnimations().Center().SetPosition(width/2 - (text_width + 10)/2 - 10, height/2 + pos_y - (int)(90.0 * m_ScalingFactor), -100)
					.SetSize(text_width + 40, (int)(200.0 * m_ScalingFactor))
					.AddAnimation(Animation::CreateLinear(0.0, 0.0,  40.0, [](auto& g) { return g.m_a < 400.0; }))
					.AddAnimation(Animation::CreateLinear(0.0, 0.0, -35.0, [](auto& g) { return g.m_a >   0.0; }));
			}

			m_Graphics["hajime"].UpdateTexture(renderer, "Hajime", ZED::Color(255, 255, 255));
			m_Graphics["mate"  ].UpdateTexture(renderer, "Mate",   ZED::Color(0, 0, 0));
			m_Graphics["sonomama"].UpdateTexture(renderer, "Sonomama", ZED::Color(0, 0, 0));
			m_Graphics["yoshi"].UpdateTexture(renderer, "Yoshi", ZED::Color(255, 255, 255));

			//Effects
			m_Graphics["effect_wazaari_white"].UpdateTexture(renderer, "Wazaari", ZED::Color(0, 0, 0));
			m_Graphics["effect_wazaari_blue" ].UpdateTexture(renderer, "Wazaari", ZED::Color(255, 255, 255));

			m_Graphics["effect_yuko_white"].UpdateTexture(renderer, "Yuko", ZED::Color(0, 0, 0));
			m_Graphics["effect_yuko_blue" ].UpdateTexture(renderer, "Yuko", ZED::Color(255, 255, 255));

			m_Graphics["effect_koka_white"].UpdateTexture(renderer, "Koka", ZED::Color(0, 0, 0));
			m_Graphics["effect_koka_blue" ].UpdateTexture(renderer, "Koka", ZED::Color(255, 255, 255));

			m_Graphics["effect_osaekomi_white"].UpdateTexture(renderer, "Osaekomi", ZED::Color(0, 0, 0));
			m_Graphics["effect_osaekomi_blue" ].UpdateTexture(renderer, "Osaekomi", ZED::Color(255, 255, 255));

			m_Graphics["effect_tokeda_white"].UpdateTexture(renderer, "Tokeda", ZED::Color(0, 0, 0));
			m_Graphics["effect_tokeda_blue" ].UpdateTexture(renderer, "Tokeda", ZED::Color(255, 255, 255));

			m_Graphics["effect_shido_white"].UpdateTexture(renderer, "Shido", ZED::Color(0, 0, 0));
			m_Graphics["effect_shido_blue" ].UpdateTexture(renderer, "Shido", ZED::Color(255, 255, 255));

			m_Graphics["effect_hansokumake_white"].UpdateTexture(renderer, "Hansokumake", ZED::Color(0, 0, 0));
			m_Graphics["effect_hansokumake_blue" ].UpdateTexture(renderer, "Hansokumake", ZED::Color(255, 255, 255));

			m_Graphics["effect_ippon_blue" ].Left().SetPosition((int)(20.0 * m_ScalingFactor), effect_row1);
			m_Graphics["effect_ippon_white"].Left().SetPosition(width - (int)(550.0 * m_ScalingFactor), effect_row1);

			m_Graphics["effect_wazaari_blue" ].SetPosition((int)(20.0 * m_ScalingFactor), effect_row2);
			m_Graphics["effect_wazaari_white"].SetPosition(width - (int)(550.0 * m_ScalingFactor), effect_row2);

			m_Graphics["effect_yuko_blue" ].SetPosition((int)(300.0 * m_ScalingFactor), effect_row1);
			m_Graphics["effect_yuko_white"].SetPosition(width - (int)(270.0 * m_ScalingFactor), effect_row1);

			m_Graphics["effect_koka_blue" ].SetPosition((int)(300.0 * m_ScalingFactor), effect_row2);
			m_Graphics["effect_koka_white"].SetPosition(width - (int)(270.0 * m_ScalingFactor), effect_row2);

			m_Graphics["effect_osaekomi_blue" ].SetPosition((int)(20.0 * m_ScalingFactor), effect_row3);
			m_Graphics["effect_osaekomi_white"].SetPosition(width - (int)(490.0 * m_ScalingFactor), effect_row3);

			m_Graphics["effect_tokeda_blue" ].SetPosition((int)(520.0 * m_ScalingFactor), effect_row3);
			m_Graphics["effect_tokeda_white"].SetPosition(width - (int)(850.0 * m_ScalingFactor), effect_row3);

			m_Graphics["effect_ippon_white"].StopAllAnimations();
			m_Graphics["effect_ippon_white"].AddAnimation(Animation::CreateScaleSinus(0.25 * m_ScalingFactor, 0.007, 1.0, 0.01, 0.4));
			m_Graphics["effect_ippon_white"].GetAnimations()[0].RunInParallel();

			m_Graphics["effect_ippon_blue"].StopAllAnimations();
			m_Graphics["effect_ippon_blue"].AddAnimation(Animation::CreateScaleSinus(0.25 * m_ScalingFactor, 0.007, 1.0, 0.01, 0.4));
			m_Graphics["effect_ippon_blue"].GetAnimations()[0].RunInParallel();

			m_Graphics["effect_ippon_white"].UpdateTexture(renderer, "Ippon", ZED::Color(0, 0, 0));
			m_Graphics["effect_ippon_blue" ].UpdateTexture(renderer, "Ippon", ZED::Color(255, 255, 255));
			m_Graphics["effect_ippon_white"].Centralize();
			m_Graphics["effect_ippon_blue" ].Centralize();

			//Same as ippon
			m_Graphics["effect_shido_blue" ].SetPosition((int)(20.0 * m_ScalingFactor), effect_row1);
			m_Graphics["effect_shido_white"].SetPosition(width - (int)(550.0 * m_ScalingFactor), effect_row1);

			//Almost the same as wazaari
			m_Graphics["effect_hansokumake_blue" ].SetPosition((int)(20.0 * m_ScalingFactor), effect_row3);
			m_Graphics["effect_hansokumake_white"].SetPosition(width - (int)(645.0 * m_ScalingFactor), effect_row3);

			//Winner animation
			if (m_Winner)
			{
				const double base_size = 0.75 * m_ScalingFactor;

				m_Graphics["winner_blue"].SetAlpha(-400.0);
				m_Graphics["winner_blue"].Center();
				m_Graphics["winner_blue"].SetPosition((int)(220.0 * m_ScalingFactor) + (int)(m_Winner->GetWidth() * base_size / 2.0f),
					height / 2 + (int)(-100.0 * m_ScalingFactor) + (int)(m_Winner->GetHeight() * base_size / 2.0f));

				m_Graphics["winner_blue"].StopAllAnimations();
				m_Graphics["winner_blue"].AddAnimation(Animation::CreateScaleSinus(0.1 * m_ScalingFactor, 0.004, base_size));
				m_Graphics["winner_blue"].GetAnimations()[0].RunInParallel();
				m_Graphics["winner_blue"].AddAnimation(Animation::CreateLinear(0.0, 0.0, 140.0, [=](auto& g) { return g.m_a < 255.0; }));

				m_Graphics["winner_white"].SetAlpha(-400.0);
				m_Graphics["winner_white"].Center();
				m_Graphics["winner_white"].SetPosition(width - (int)(220.0 * m_ScalingFactor) - (int)(m_Winner->GetWidth() * base_size / 2.0f),
					height / 2 + (int)(-100.0 * m_ScalingFactor) + (int)(m_Winner->GetHeight() * base_size / 2.0f));

				m_Graphics["winner_white"].StopAllAnimations();
				m_Graphics["winner_white"].AddAnimation(Animation::CreateScaleSinus(0.1 * m_ScalingFactor, 0.004, base_size));
				m_Graphics["winner_white"].GetAnimations()[0].RunInParallel();
				m_Graphics["winner_white"].AddAnimation(Animation::CreateLinear(0.0, 0.0, 140.0, [=](auto& g) { return g.m_a < 255.0; }));
			}

			break;
		}

		case State::Running:
			break;

		case State::TransitionToWaiting:
			for (int i = 0; i < 2; i++)
			{
				double shift_y = 125.0;
				if (i > 0)
					shift_y += 495.0;

				shift_y *= m_ScalingFactor;

				
				m_Graphics["next_matches_blue_" + std::to_string(i)].StopAllAnimations()
																	 .SetPosition(40 - 1320, (int)shift_y, 0)
																	 .AddAnimation(Animation::CreateLinear( 140.0, 0.0, 32.0, [](auto& g) { return g.m_x < 40.0; }));

				m_Graphics["next_matches_white_"  + std::to_string(i)].StopAllAnimations().Right()
					.SetPosition(width - 40 + 1320, (int)shift_y, 0)
					.AddAnimation(Animation::CreateLinear(-140.0, 0.0, 32.0, [=](auto& g) { return g.m_x > width - 40; }));


				m_Graphics["next_matches_blue2_" + std::to_string(i)].StopAllAnimations()
					.SetPosition(40 - 1320, (int)shift_y + (int)(158.0*m_ScalingFactor), 0)
					.AddAnimation(Animation::CreateLinear( 140.0, 0.0, 32.0, [](auto& g) { return g.m_x < 40.0; }));

				m_Graphics["next_matches_white2_"  + std::to_string(i)].StopAllAnimations().Right()
					.SetPosition(width - 40 + 1320, (int)shift_y + (int)(158.0*m_ScalingFactor), 0)
					.AddAnimation(Animation::CreateLinear(-140.0, 0.0, 32.0, [=](auto& g) { return g.m_x > width - 40; }));

				m_Graphics["next_matches_blue_club_" + std::to_string(i)].StopAllAnimations()
					.SetPosition(40 - 1320, (int)shift_y + (int)(330.0*m_ScalingFactor), 0)
					.AddAnimation(Animation::CreateLinear( 140.0, 0.0, 32.0, [](auto& g) { return g.m_x < 40.0; }));

				m_Graphics["next_matches_white_club_"  + std::to_string(i)].StopAllAnimations().Right()
					.SetPosition(width - 40 + 1320, (int)shift_y + (int)(330.0*m_ScalingFactor), 0)
					.AddAnimation(Animation::CreateLinear(-140.0, 0.0, 32.0, [=](auto& g) { return g.m_x > width - 40; }));
			}

			m_Graphics["next_match"].StopAllAnimations().Center()
				.SetPosition(width/2, (int)(80*m_ScalingFactor), -115)
				.AddAnimation(Animation::CreateLinear(0.0, 0.0, 35.0, [](auto& g) { return g.m_a < 255.0; }));

			m_Graphics["following_match"].StopAllAnimations().Center()
				.SetPosition(width/2, (int)(565 * m_ScalingFactor), -120)
				.AddAnimation(Animation::CreateLinear(0.0, 0.0, 32.0, [](auto& g) { return g.m_a < 255.0; }));

			m_Graphics["osaekomi_bar_border"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -60.0));
			m_Graphics["osaekomi_text"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -40.0));
			m_Graphics["osaekomi_bar" ].AddAnimation(Animation::CreateLinear(0.0, 0.0, -40.0));


			m_Graphics["white_name"].AddAnimation(Animation::CreateLinear(0.0, -67.0, -25.0));
			m_Graphics["blue_name" ].AddAnimation(Animation::CreateLinear(0.0, -67.0, -25.0));

			m_Graphics["white_club"].AddAnimation(Animation::CreateLinear(0.0, -67.0, -25.0));
			m_Graphics["blue_club" ].AddAnimation(Animation::CreateLinear(0.0, -67.0, -25.0));

			m_Graphics["white_ippon" ].AddAnimation(Animation::CreateLinear(0.0, -67.0, -18.0));
			m_Graphics["white_wazari"].AddAnimation(Animation::CreateLinear(0.0, -67.0, -18.0));
			m_Graphics["white_yuko"].AddAnimation(Animation::CreateLinear(0.0, -67.0, -18.0));
			m_Graphics["white_koka"].AddAnimation(Animation::CreateLinear(0.0, -67.0, -18.0));

			m_Graphics["blue_ippon" ].AddAnimation(Animation::CreateLinear(0.0, -67.0, -18.0));
			m_Graphics["blue_wazari"].AddAnimation(Animation::CreateLinear(0.0, -67.0, -18.0));
			m_Graphics["blue_yuko"].AddAnimation(Animation::CreateLinear(0.0, -67.0, -18.0));
			m_Graphics["blue_koka"].AddAnimation(Animation::CreateLinear(0.0, -67.0, -18.0));

			m_Graphics["timer"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -33.0));

			m_Graphics["mat_name"].SetPosition(20, height - (int)(70.0 * m_ScalingFactor))
								  .AddAnimation(Animation::CreateLinear(0.0, 0.0, 10.0, [=](auto& g) { return g.m_a < 255.0; }));

			m_Graphics["winner_blue" ].AddAnimation(Animation::CreateLinear(0.0, 0.0, -40.0));
			m_Graphics["winner_white"].AddAnimation(Animation::CreateLinear(0.0, 0.0, -40.0));
			break;
	}

	m_State = NextState;
}



void Mat::UpdateGraphics() const
{
	if (m_State == State::StartUp)
		return;

	auto& renderer = m_Window.GetRenderer();

	const unsigned int width  = renderer.GetWidth();
	const unsigned int height = renderer.GetHeight();

	auto guard = m_mutex.LockWriteForScope();

	//Update graphics

	std::string name = Localizer::Translate("Current Match");

	if (m_NextMatches.size() >= 1 && m_NextMatches[0].GetMatchTable())
		name += " (" + m_NextMatches[0].GetMatchTable()->GetDescription() + ")";

	m_Graphics["next_match"].UpdateTexture(renderer, name, ZED::Color(255, 0, 0), ZED::FontSize::Huge);
	while (m_Graphics["next_match"]->GetWidth() > width)
		m_Graphics["next_match"].Shorten(renderer);

	name = Localizer::Translate("Next Match");
	if (m_NextMatches.size() >= 2 && m_NextMatches[1].GetMatchTable())
		name += " (" + m_NextMatches[1].GetMatchTable()->GetDescription() + ")";

	m_Graphics["following_match"].UpdateTexture(renderer, name, ZED::Color(255, 0, 0), ZED::FontSize::Huge);
	while (m_Graphics["following_match"]->GetWidth() > width)
		m_Graphics["following_match"].Shorten(renderer);

	m_Graphics["mat_name"].UpdateTexture(renderer, GetName(), ZED::Color(255, 255, 255), ZED::FontSize::Middle);

	switch (m_State)
	{
	case State::StartUp:
		break;
	case State::Waiting:
	case State::TransitionToWaiting:
	{
		//ZED::FontSize FontSize = ZED::FontSize::Huge;
		ZED::FontSize FontSize = ZED::FontSize::Gigantic;

		for (size_t i = m_NextMatches.size(); i < 2; i++)
		{
			m_Graphics["next_matches_white_" + std::to_string(i)].Clear();
			m_Graphics["next_matches_blue_"  + std::to_string(i)].Clear();
			m_Graphics["next_matches_white2_" + std::to_string(i)].Clear();
			m_Graphics["next_matches_blue2_"  + std::to_string(i)].Clear();
			m_Graphics["next_matches_white_club_" + std::to_string(i)].Clear();
			m_Graphics["next_matches_blue_club_"  + std::to_string(i)].Clear();
		}

		int i = 0;
		for (auto match : m_NextMatches)
		{
			if (!match.HasValidFighters())
				continue;

			//Update judoka name
			if (GetNameStyle() == NameStyle::GivenName)
			{
				m_Graphics["next_matches_white_"  + std::to_string(i)].UpdateTexture(renderer, match.GetFighter(Fighter::White)->GetFirstname(), ZED::Color(0, 0, 0), FontSize);
				m_Graphics["next_matches_blue_"   + std::to_string(i)].UpdateTexture(renderer, match.GetFighter(Fighter::Blue )->GetFirstname(), ZED::Color(255, 255, 255), FontSize);
				m_Graphics["next_matches_white2_" + std::to_string(i)].UpdateTexture(renderer, match.GetFighter(Fighter::White)->GetLastname(), ZED::Color(0, 0, 0), FontSize);
				m_Graphics["next_matches_blue2_"  + std::to_string(i)].UpdateTexture(renderer, match.GetFighter(Fighter::Blue )->GetLastname(), ZED::Color(255, 255, 255), FontSize);
			}
			else
			{
				m_Graphics["next_matches_white_"  + std::to_string(i)].UpdateTexture(renderer, match.GetFighter(Fighter::White)->GetLastname() + ",", ZED::Color(0, 0, 0), FontSize);
				m_Graphics["next_matches_blue_"   + std::to_string(i)].UpdateTexture(renderer, match.GetFighter(Fighter::Blue )->GetLastname() + ",", ZED::Color(255, 255, 255), FontSize);
				m_Graphics["next_matches_white2_" + std::to_string(i)].UpdateTexture(renderer, match.GetFighter(Fighter::White)->GetFirstname(), ZED::Color(0, 0, 0), FontSize);
				m_Graphics["next_matches_blue2_"  + std::to_string(i)].UpdateTexture(renderer, match.GetFighter(Fighter::Blue )->GetFirstname(), ZED::Color(255, 255, 255), FontSize);
			}

			//Update clubs of judokas
			if (match.GetFighter(Fighter::White)->GetClub())
				m_Graphics["next_matches_white_club_" + std::to_string(i)].UpdateTexture(renderer, match.GetFighter(Fighter::White)->GetClub()->GetName(), ZED::Color(0, 0, 0), ZED::FontSize::Middle);
			else
				m_Graphics["next_matches_white_club_" + std::to_string(i)].Clear();

			if (match.GetFighter(Fighter::Blue)->GetClub())
				m_Graphics["next_matches_blue_club_"  + std::to_string(i)].UpdateTexture(renderer, match.GetFighter(Fighter::Blue)->GetClub()->GetName(), ZED::Color(255, 255, 255), ZED::FontSize::Middle);
			else
				m_Graphics["next_matches_blue_club_"  + std::to_string(i)].Clear();

			//Shorten names if necessary
			while (m_Graphics["next_matches_white_" + std::to_string(i)] && m_Graphics["next_matches_white_" + std::to_string(i)]->GetWidth() > std::max((width - 25.0 * m_ScalingFactor)/2, 100.0))
				m_Graphics["next_matches_white_" + std::to_string(i)].Shorten(renderer);
			while (m_Graphics["next_matches_blue_"  + std::to_string(i)] && m_Graphics["next_matches_blue_"  + std::to_string(i)]->GetWidth() > std::max((width - 25.0 * m_ScalingFactor)/2, 100.0))
				m_Graphics["next_matches_blue_"  + std::to_string(i)].Shorten(renderer);

			i++;

			if (i == 2)
				break;
		}

		break;
	}



	case State::TransitionToMatch:
		//Update fighter's names
		m_Graphics["white_name"].UpdateTexture(renderer, GetFighter(Fighter::White).GetName(GetNameStyle()), ZED::Color(0, 0, 0));
		m_Graphics["blue_name" ].UpdateTexture(renderer, GetFighter(Fighter::Blue ).GetName(GetNameStyle()), ZED::Color(255, 255, 255));

		while (m_Graphics["white_name"] && m_Graphics["white_name"]->GetWidth() > std::max((width - 25.0*m_ScalingFactor)/2, 100.0))
			m_Graphics["white_name"].Shorten(renderer);
		while (m_Graphics["blue_name" ] && m_Graphics["blue_name" ]->GetWidth() > std::max((width - 25.0*m_ScalingFactor)/2, 100.0))
			m_Graphics["blue_name"].Shorten(renderer);

		if (GetFighter(Fighter::White).GetClub())
			m_Graphics["white_club"].UpdateTexture(renderer, GetFighter(Fighter::White).GetClub()->GetName(), ZED::Color(0, 0, 0), ZED::FontSize::Large);
		else
			m_Graphics["white_club"].Clear();
		if (GetFighter(Fighter::Blue).GetClub())
			m_Graphics["blue_club" ].UpdateTexture(renderer, GetFighter(Fighter::Blue ).GetClub()->GetName(), ZED::Color(255, 255, 255), ZED::FontSize::Large);
		else
			m_Graphics["blue_club" ].Clear();

	//fall through

	case State::Running:
	{
		//Update scoreboard
		switch (GetIpponStyle())
		{
		case IpponStyle::SingleDigit:
			m_Graphics["white_wazari"].UpdateTexture(renderer, std::to_string(GetScoreboard(Fighter::White).m_Ippon ? 2 : GetScoreboard(Fighter::White).m_WazaAri), ZED::Color(0, 0, 0), ZED::FontSize::Gigantic);
			m_Graphics["blue_wazari" ].UpdateTexture(renderer, std::to_string(GetScoreboard(Fighter::Blue ).m_Ippon ? 2 : GetScoreboard(Fighter::Blue ).m_WazaAri), ZED::Color(255, 255, 255), ZED::FontSize::Gigantic);
			break;

		case IpponStyle::DoubleDigit:
			m_Graphics["white_ippon" ].UpdateTexture(renderer, std::to_string(GetScoreboard(Fighter::White).m_Ippon), ZED::Color(0, 0, 0), ZED::FontSize::Gigantic);
			m_Graphics["white_wazari"].UpdateTexture(renderer, std::to_string(GetScoreboard(Fighter::White).m_Ippon ? 0 : GetScoreboard(Fighter::White).m_WazaAri), ZED::Color(0, 0, 0), ZED::FontSize::Gigantic);

			m_Graphics["blue_ippon" ].UpdateTexture(renderer, std::to_string(GetScoreboard(Fighter::Blue).m_Ippon), ZED::Color(255, 255, 255), ZED::FontSize::Gigantic);
			m_Graphics["blue_wazari"].UpdateTexture(renderer, std::to_string(GetScoreboard(Fighter::Blue).m_Ippon ? 0 : GetScoreboard(Fighter::Blue).m_WazaAri), ZED::Color(255, 255, 255), ZED::FontSize::Gigantic);
			break;

		case IpponStyle::SpelledOut:
		{
			for (Fighter f = Fighter::White; f <= Fighter::Blue; ++f)
			{
				auto wazaari = &m_Graphics["white_wazari"];
				auto ippon   = &m_Graphics["white_ippon"];
				auto color   = ZED::Color(0, 0, 0);

				if (f == Fighter::Blue)
				{
					wazaari = &m_Graphics["blue_wazari"];
					ippon   = &m_Graphics["blue_ippon"];
					color   = ZED::Color(255, 255, 255);
				}

				if (GetScoreboard(f).m_WazaAri == 2)
				{
					if (wazaari->GetAnimations().size() == 0 && ippon->m_a < 1.0)
					{
						wazaari->SetAlpha(320.0).AddAnimation(Animation::CreateLinear(0.0, 0.0, -60.0, [](auto& g) { return g.m_a > 0.0; }));
						ippon->SetAlpha(-180.0).StopAllAnimations().AddAnimation(Animation::CreateLinear(0.0, 0.0, 40.0, [](auto& g) { return g.m_a < 255.0; }));
					}
				}
				else if (GetScoreboard(f).m_WazaAri == 1)
					wazaari->SetAlpha(255.0);

				if (GetScoreboard(f).m_WazaAri > 0)
					wazaari->UpdateTexture(renderer, std::to_string(GetScoreboard(f).m_WazaAri) + " Wazaari", color, ZED::FontSize::Huge);
				else
					wazaari->UpdateTexture(renderer, "", color, ZED::FontSize::Huge);

				if (GetScoreboard(f).m_Ippon == 1)
					ippon->UpdateTexture(renderer, "Ippon", color, ZED::FontSize::Huge);
				else
					ippon->SetAlpha(0.0).UpdateTexture(renderer, "", color, ZED::FontSize::Huge);
			}

			break;
		}
		}//End of switch (GetIpponStyle())


		m_Graphics["white_yuko"].UpdateTexture(renderer, std::to_string(GetScoreboard(Fighter::White).m_Yuko), ZED::Color(0, 0, 0), ZED::FontSize::Gigantic);
		m_Graphics["white_koka"].UpdateTexture(renderer, std::to_string(GetScoreboard(Fighter::White).m_Koka), ZED::Color(0, 0, 0), ZED::FontSize::Gigantic);

		m_Graphics["blue_yuko" ].UpdateTexture(renderer, std::to_string(GetScoreboard(Fighter::Blue).m_Yuko),  ZED::Color(255, 255, 255), ZED::FontSize::Gigantic);
		m_Graphics["blue_koka" ].UpdateTexture(renderer, std::to_string(GetScoreboard(Fighter::Blue).m_Koka),  ZED::Color(255, 255, 255), ZED::FontSize::Gigantic);


		if (m_pMatch)
		{
			std::string mat_matchtable = GetName();
			if (m_pMatch->GetMatchTable())
				mat_matchtable += "   " + m_pMatch->GetMatchTable()->GetDescription();
			m_Graphics["mat_name"].UpdateTexture(renderer, mat_matchtable, ZED::Color(255, 255, 255), ZED::FontSize::Middle);


			//Update timer
			Timer time(GetTime2Display());
			if (GetTimerStyle() == TimerStyle::HundredsMS)
				m_Graphics["timer"].UpdateTexture(renderer, time.ToStringWithHundreds(), ZED::Color(0, 0, 0), ZED::FontSize::Gigantic2);
			else if (GetTimerStyle() == TimerStyle::OnlySeconds)
				m_Graphics["timer"].UpdateTexture(renderer, time.ToStringInSeconds(),    ZED::Color(0, 0, 0), ZED::FontSize::Gigantic2);
			else if (GetTimerStyle() == TimerStyle::Full)
				m_Graphics["timer"].UpdateTexture(renderer, time.ToString(), ZED::Color(0, 0, 0), ZED::FontSize::Gigantic2);
			
			
			//Update osaekomi
			UpdateOsaekomiGraphics();

			//Update text effects
			if (m_Graphics["timer"])
			{
				m_Graphics["hajime"].SetPosition(width/2  - m_Graphics["timer"]->GetWidth()/2 - (int)(100.0*m_ScalingFactor),
												 height/2 + (int)(m_Graphics["timer"]->GetHeight() * 0.40));

				m_Graphics["mate"  ].SetPosition(width/2  + (int)(100.0*m_ScalingFactor),
												 height/2 + (int)(m_Graphics["timer"]->GetHeight() * 0.40));

				m_Graphics["yoshi"].SetPosition(width/2  - m_Graphics["timer"]->GetWidth()/2 - (int)(10.0*m_ScalingFactor),
												height/2 + (int)(m_Graphics["timer"]->GetHeight() * 0.40));

				m_Graphics["sonomama"].SetPosition(width/2  + (int)(100.0*m_ScalingFactor),
												   height/2 + (int)(m_Graphics["timer"]->GetHeight() * 0.40));
			}
		}
	}
	break;
	}
}



void Mat::UpdateOsaekomiGraphics() const
{
	auto guard = m_mutex.LockWriteForScope();

	auto& renderer = m_Window.GetRenderer();

	const unsigned int width  = renderer.GetWidth();
	const unsigned int height = renderer.GetHeight();

	const int osaekomi_max_width = width;
	const int osaekomi_y = height/2 + (int)(340.0*m_ScalingFactor);

	Fighter fighter = GetOsaekomiHolder();
	auto& osaekomi_text = m_Graphics["osaekomi_text"];
	auto& osaekomi_bar  = m_Graphics["osaekomi_bar"];

	if (GetOsaekomiStyle() == OsaekomiStyle::ProgressBar)
	{
		osaekomi_text.UpdateTexture(renderer, m_OsaekomiTimer[(int)fighter].ToStringOnlySeconds(), ZED::Color(0, 0, 0), ZED::FontSize::Huge);

		int new_width = m_OsaekomiTimer[(int)fighter].GetElapsedTime() * osaekomi_max_width / (EndTimeOfOsaekomi() * 1000);
		if (new_width == 0)//Seems like no osaekomi is running
			new_width = -100;//Move way to the left

		if (osaekomi_text)
		{
			const int new_text_x = new_width - osaekomi_text->GetWidth() - 2;
			osaekomi_text.StopAllAnimations().SetPosition(new_text_x, osaekomi_y + 1, 255);
		}

		auto& osaekomi_bar_border = m_Graphics["osaekomi_bar_border"];
		osaekomi_bar_border.SetPosition(0, osaekomi_y, 255);
		osaekomi_bar_border.m_width  = new_width + 5;
		osaekomi_bar_border.m_height = (int)(100.0 * m_ScalingFactor) + 10;
		osaekomi_bar_border.m_color  = ZED::Color(0, 0, 0);


		osaekomi_bar.SetPosition(0, osaekomi_y + 5, 255);
		//osaekomi_bar.m_color = ZED::Color(30, 150, 30);
		osaekomi_bar.m_color = ZED::Color(255, 0, 0);
		osaekomi_bar.m_width = new_width;
		osaekomi_bar.m_height = (int)(100.0 * m_ScalingFactor);
	}
	else
	{
		osaekomi_text.UpdateTexture(renderer, m_OsaekomiTimer[(int)fighter].ToStringOnlySeconds(), ZED::Color(0, 0, 0), ZED::FontSize::Gigantic);

		auto& osaekomi_bar_border = m_Graphics["osaekomi_bar_border"];

		osaekomi_bar_border.SetColor(ZED::Color(0, 0, 0));
		osaekomi_bar.SetColor(ZED::Color(255, 0, 0));

		//New osaekomi bar
		double rect_width  = 190.0;
		double rect_height = 150.0;
		double outer_rect_width  = rect_width  + 10.0;
		double outer_rect_height = rect_height + 10.0;
		osaekomi_text.Center().SetPosition(width/2, osaekomi_y);
		osaekomi_bar. Center().SetPosition(width/2 - (int)(rect_width/2.0 * m_ScalingFactor), osaekomi_y - (int)(rect_height/2.0 * m_ScalingFactor))
					.SetSize((int)(rect_width * m_ScalingFactor), (int)(rect_height * m_ScalingFactor));
		osaekomi_bar_border.Center().SetPosition(width/2 - (int)(outer_rect_width/2.0 * m_ScalingFactor), osaekomi_y - (int)(outer_rect_height/2.0 * m_ScalingFactor))
					.SetSize((int)(outer_rect_width * m_ScalingFactor), (int)(outer_rect_height * m_ScalingFactor));
	}
}



void Mat::RenderScore(double dt) const
{
	auto& renderer = m_Window.GetRenderer();

	if (GetIpponStyle() == IpponStyle::DoubleDigit || GetIpponStyle() == IpponStyle::SpelledOut)
	{
		m_Graphics["white_ippon"].Render(renderer, dt);
		m_Graphics["blue_ippon" ].Render(renderer, dt);
	}

	m_Graphics["white_wazari"].Render(renderer, dt);
	m_Graphics["blue_wazari" ].Render(renderer, dt);

	if (m_pMatch && m_pMatch->GetRuleSet().IsYukoEnabled())
	{
		m_Graphics["white_yuko"].Render(renderer, dt);
		m_Graphics["blue_yuko" ].Render(renderer, dt);
	}
	if (m_pMatch && m_pMatch->GetRuleSet().IsKokaEnabled())
	{
		m_Graphics["white_koka"].Render(renderer, dt);
		m_Graphics["blue_koka" ].Render(renderer, dt);
	}
}



void Mat::RenderTimer(double dt) const
{
	auto& renderer = m_Window.GetRenderer();

	const int width  = renderer.GetWidth();
	const int height = renderer.GetHeight();

	if (!m_Graphics["timer"])
		return;

	const int box_width  = m_Graphics["timer"]->GetWidth() + 28;
	const int box_top    = height/2 - (int)(m_Graphics["timer"]->GetHeight()*0.40);
	const int box_bottom = height/2 + (int)(m_Graphics["timer"]->GetHeight()*0.40);

	if (m_Graphics["timer"].m_a > 0.0)
	{
		unsigned char a = (unsigned char)m_Graphics["timer"].m_a;

		if (m_Graphics["timer"].m_a > 255.0)
			a = 255;

		//Render box
		renderer.FillRect(ZED::Rect(width/2 - box_width/2-3-5, box_top-5, box_width+10, box_bottom - box_top+10), 0, 0, 0, a);
		renderer.FillRect(ZED::Rect(width/2 - box_width/2-3, box_top, box_width, box_bottom - box_top), 255, 255, 0, a);
	}

	m_Graphics["timer"].Render(renderer, dt);
}



void Mat::RenderShidos(double dt) const
{
	auto& renderer = m_Window.GetRenderer();

	const int width  = renderer.GetWidth();
	const int height = renderer.GetHeight();

	unsigned char alpha = 255;
	if (m_Graphics["timer"])
	{
		if (m_Graphics["timer"].m_a < 0.0)
			return;
		alpha = (unsigned char)m_Graphics["timer"].m_a;//Abuse alpha value of timer
		if (m_Graphics["timer"].m_a >= 255.0)
			alpha = 255;
	}

	for (Fighter fighter = Fighter::White; fighter <= Fighter::Blue; fighter++)
	{
		//Render shidos
		int shido_start_x = (int)( 50.0 * m_ScalingFactor);
		int shido_start_y = (int)(400.0 * m_ScalingFactor);

		if (fighter == Fighter::White)
			shido_start_x = width - (int)(630.0 * m_ScalingFactor);

		int shido_x = shido_start_x;
		int shido_y = shido_start_y;

		const int shido_width = (int)(100.0 * m_ScalingFactor);
		const int shido_height = (int)(1.375 * shido_width);//Same aspect ratio as in football
		const int margin = (int)(10.0 * m_ScalingFactor);

		for (int shido = 0; shido < GetScoreboard(fighter).m_Shido; shido++)
		{
			renderer.FillRect(ZED::Rect(shido_x, shido_y, shido_width, shido_height), 0, 0, 0, alpha);
			renderer.FillRect(ZED::Rect(shido_x + margin, shido_y + margin, shido_width - 2*margin, shido_height - 2*margin), 255, 255, 0, alpha);

			shido_x += (int)(30.0 * m_ScalingFactor);
			shido_y += (int)(30.0 * m_ScalingFactor);
		}

		//Render hansokumake
		int hansokumake_x = shido_start_x + (int)(180.0 * m_ScalingFactor);
		int hansokumake_y = shido_start_y;

		if (GetScoreboard(fighter).m_HansokuMake)
		{
			renderer.FillRect(ZED::Rect(hansokumake_x, hansokumake_y, shido_width, shido_height), 0, 0, 0, alpha);
			renderer.FillRect(ZED::Rect(hansokumake_x + margin, hansokumake_y + margin, shido_width - 2*margin, shido_height - 2*margin), 255, 0, 0, alpha);
		}

		//Render medical examinations
		int medical_x = shido_start_x + (int)(350.0 * m_ScalingFactor);
		int medical_y = shido_start_y;

		for (int examinations = 0; examinations < GetScoreboard(fighter).m_MedicalExamination; examinations++)
		{
			const int medical_size = (int)(100.0 * m_ScalingFactor);
			const int medical_size_interior = (int)(80.0 * m_ScalingFactor);
			const int medical_cross_length  = (int)(60.0 * m_ScalingFactor);
			const int medical_cross_width   = (int)(20.0 * m_ScalingFactor);

			//Derived constants
			const int margin = medical_size - medical_size_interior;

			renderer.FillRect(ZED::Rect(medical_x, medical_y, medical_size, medical_size), 0, 0, 0, alpha);//Black border
			renderer.FillRect(ZED::Rect(medical_x + margin / 2, medical_y + margin / 2, medical_size_interior, medical_size_interior), 0, 255, 0, alpha);//Green interior
			renderer.FillRect(ZED::Rect(medical_x + margin, medical_y + 2 * margin, medical_cross_length, medical_cross_width), 255, 255, 255, alpha);//White bar
			renderer.FillRect(ZED::Rect(medical_x + 2 * margin, medical_y + margin, medical_cross_width, medical_cross_length), 255, 255, 255, alpha);//White bar

			medical_x += (int)(115.0 * m_ScalingFactor);
		}
	}
}



bool Mat::Render(double dt) const
{
	if (!m_Window.HandleEvents())
		return false;

	if (!m_Background)
		return true;

	if (dt > 0.2)//In case the application was stuck for some time
		dt = 0.2;//Might happen on device with a small amount of RAM during swapping
	
	UpdateGraphics();
	
	auto& renderer = m_Window.GetRenderer();
	
	renderer.Lock();
	renderer.ClearDisplay();
	
	const int width  = renderer.GetWidth();
	const int height = renderer.GetHeight();

	
	//Render background
	renderer.FillRect(ZED::Rect(0, 0, width/2 - 50, height), 0, 0, 255);
	renderer.FillRect(ZED::Rect(width/2 + 50, 0, width, height), 255, 255, 255);

	if (m_Background && m_Background->GetType() == ZED::Type::OpenGL)
	{
		m_Background->SetSizeX(1.0f);
		m_Background->SetSizeY(20.0f);
		m_Background->SetAngle(0.0f);
		renderer.RenderTransformedRotated(*m_Background, width / 2 - 50, 0);
	}
	else if (m_Background)
	{
		m_Background->SetAngle(0.0f);
		for (int y = 0; y < height + 100; y += 100)
			renderer.RenderTransformed(*m_Background, width / 2 - 50, y);
	}

	m_mutex.LockRead();
	
	//Render graphics
	switch (m_State)
	{
	case State::StartUp://Render startup screen
	{
		//Render animated background
		const int border = height / 2 + (int)((float)height / 4.0f * (sinf((float)Timer::GetTimestamp() / 2000.0f))) + 50;

		renderer.FillRect(ZED::Rect(0, 0, width, border - 50), 255, 255, 255);
		renderer.FillRect(ZED::Rect(0, border + 50, width, height), 0, 0, 255);

		if (m_Background->GetType() == ZED::Type::OpenGL)
		{
			m_Background->SetSizeX(20.0f);
			m_Background->SetSizeY(1.0f);
			m_Background->SetAngle(90.0f);
			renderer.RenderTransformedRotated(*m_Background, 0, border - 50);
		}
		else
		{
			m_Background->SetAngle(90.0f);
			for (int x = -2; x < width + 100; x += 99)
				renderer.RenderTransformedRotated(*m_Background, x, border - 51);
		}


		if (m_Logo)//Render logo
		{
			m_Logo->SetSize(renderer.GetWidth() / 1920.0f);
			renderer.RenderTransformed(*m_Logo, width/2 - (int)(m_Logo->GetWidth()/2.0f * m_Logo->GetSizeX()), 50);
		}

		auto name = renderer.RenderFont(ZED::FontSize::Gigantic, GetName(), ZED::Color(0, 0, 0));
		if (name)
			renderer.RenderTransformed(name, width/2 - name->GetWidth()/2, height/2);

		auto appname = renderer.RenderFont(ZED::FontSize::Middle, Application::Name, ZED::Color(0, 0, 0));
		if (appname)
			renderer.RenderTransformed(appname, 30, height - 120);

		auto version = renderer.RenderFont(ZED::FontSize::Middle, Localizer::Translate("Version") + std::string(": ") + Application::Version, ZED::Color(0, 0, 0));
		if (version)
			renderer.RenderTransformed(version, 30, height - 70);

		if (License::GetLicenseState() == License::State::Valid)
		{
			auto license = renderer.RenderFont(ZED::FontSize::Middle, Localizer::Translate("License") + std::string(":"), ZED::Color(0, 0, 0));
			if (license)
				renderer.RenderTransformed(license, width/2 - license->GetWidth()/2, height - 120);

			auto license_key = renderer.RenderFont(ZED::FontSize::Middle, License::GetUserID(), ZED::Color(0, 0, 0));
			if (license_key)
				renderer.RenderTransformed(license_key, width/2 - license_key->GetWidth()/2, height - 70);
		}
		else
		{
			auto trial = renderer.RenderFont(ZED::FontSize::Middle, Localizer::Translate("Trial version"), ZED::Color(0, 0, 0));
			if (trial)
				renderer.RenderTransformed(trial, width/2 - trial->GetWidth()/2, height - 120);
		}

		auto date = ZED::Core::GetDate();

		std::stringstream day, time;
		day << date.day << "." << date.month << "." << date.year;
		time << std::setfill('0') << std::setw(1) << date.hour << ":" << std::setw(2) << date.minute << ":" << std::setw(2) << date.second;

		auto text_date = renderer.RenderFont(ZED::FontSize::Middle, day.str(), ZED::Color(0, 0, 0));
		if (text_date)
			renderer.RenderTransformed(text_date, width - 250, height - 120);
		auto text_time = renderer.RenderFont(ZED::FontSize::Middle, time.str(), ZED::Color(0, 0, 0));
		if (text_time)
			renderer.RenderTransformed(text_time, width - 250, height - 70);

		//Is there a next match?
		if (m_NextMatches.size() > 0)
			NextState(State::TransitionToWaiting);//Transition to waiting screen

		break;
	}

	case State::Waiting:

		for (int i = 0; i < 2; i++)
		{
			m_Graphics["next_matches_white_" + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_blue_"  + std::to_string(i)].Render(renderer, dt);

			m_Graphics["next_matches_white2_" + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_blue2_"  + std::to_string(i)].Render(renderer, dt);

			m_Graphics["next_matches_white_club_" + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_blue_club_"  + std::to_string(i)].Render(renderer, dt);
		}

		m_Graphics["next_match"].Render(renderer, dt);
		m_Graphics["following_match"].Render(renderer, dt);

		m_Graphics["mat_name"].Render(renderer, dt);

		break;



	case State::TransitionToMatch:
		for (int i = 0; i < 2; i++)
		{
			m_Graphics["next_matches_white_" + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_blue_"  + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_white2_" + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_blue2_"  + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_white_club_" + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_blue_club_"  + std::to_string(i)].Render(renderer, dt);
		}

		m_Graphics["next_match"].Render(renderer, dt);
		m_Graphics["following_match"].Render(renderer, dt);

		m_Graphics["age_group_rect_bar"].Render(renderer, dt);
		m_Graphics["age_group_rect"].Render(renderer, dt);
		m_Graphics["age_group_rect_text"].Render(renderer, dt);

		if (Timer::GetTimestamp() - m_CurrentStateStarted >= 10 * 1000)
			NextState(State::Running);

	//fall through

	case State::Running:
		//Render osaekomi
		m_Graphics["osaekomi_bar_border"].Render(renderer, dt);
		m_Graphics["osaekomi_bar" ].Render(renderer, dt);
		m_Graphics["osaekomi_text"].Render(renderer, dt);

		//Render golden score
		if (m_GoldenScore && m_Graphics["timer"])
		{
			//const int gs_pox_x = (int)(55.0 * m_ScalingFactor);
			//const int gs_pox_y = (int)(78.0 * m_ScalingFactor);

			auto gs = renderer.RenderFont(ZED::FontSize::Huge, "GS", ZED::Color(0, 0, 0));
			//renderer.FillRect(ZED::Rect(   width/2 + m_Graphics["timer"]->GetWidth()/2 + gs_pox_x, height/2 - m_Graphics["timer"]->GetHeight()/2 + gs_pox_y, (int)(165.0*m_ScalingFactor), (int)(105.0*m_ScalingFactor)), 0, 0, 0);
			//renderer.FillRect(ZED::Rect(   width/2 + m_Graphics["timer"]->GetWidth()/2 + gs_pox_x + (int)(5.0*m_ScalingFactor),  height/2 - m_Graphics["timer"]->GetHeight()/2 + gs_pox_y + (int)(5.0*m_ScalingFactor), (int)(155.0*m_ScalingFactor), (int)( 95.0*m_ScalingFactor)), 255, 255, 0);
			//renderer.RenderTransformed(gs, width/2 + m_Graphics["timer"]->GetWidth()/2 + gs_pox_x + (int)(10.0*m_ScalingFactor), height/2 - m_Graphics["timer"]->GetHeight()/2 + gs_pox_y - (int)(3.0*m_ScalingFactor));

			const int gs_pos_x = width/2  - (int)(82.0 * m_ScalingFactor);
			const int gs_pos_y = height/2 + (int)(m_Graphics["timer"]->GetHeight()*0.40);

			renderer.FillRect(ZED::Rect(gs_pos_x, gs_pos_y, (int)(165.0*m_ScalingFactor), (int)(105.0*m_ScalingFactor)), 0, 0, 0);//Black border
			renderer.FillRect(ZED::Rect(gs_pos_x + (int)(5.0*m_ScalingFactor), gs_pos_y + (int)(5.0*m_ScalingFactor), (int)(155.0*m_ScalingFactor), (int)( 95.0*m_ScalingFactor)), 255, 255, 0);
			renderer.RenderTransformed(gs, gs_pos_x + (int)(9.0 * m_ScalingFactor), gs_pos_y);
		}


		m_Graphics["white_name"].Render(renderer, dt);
		m_Graphics["blue_name" ].Render(renderer, dt);
		m_Graphics["white_club"].Render(renderer, dt);
		m_Graphics["blue_club" ].Render(renderer, dt);

		RenderScore(dt);
		RenderTimer(dt);
		RenderShidos(dt);

		m_Graphics["matchtable"].Render(renderer, dt);
		m_Graphics["mat_name"  ].Render(renderer, dt);

		m_Graphics["age_group_rect_bar"].Render(renderer, dt);
		m_Graphics["age_group_rect"].Render(renderer, dt);
		m_Graphics["age_group_rect_text"].Render(renderer, dt);

		//Render text effects
		m_Graphics["hajime"].Render(renderer, dt);
		m_Graphics["mate"  ].Render(renderer, dt);
		m_Graphics["sonomama"].Render(renderer, dt);
		m_Graphics["yoshi"].Render(renderer, dt);

		m_Graphics["effect_ippon_white"].Render(renderer, dt);
		m_Graphics["effect_ippon_blue" ].Render(renderer, dt);

		m_Graphics["effect_wazaari_white"].Render(renderer, dt);
		m_Graphics["effect_wazaari_blue" ].Render(renderer, dt);

		m_Graphics["effect_yuko_white"].Render(renderer, dt);
		m_Graphics["effect_yuko_blue" ].Render(renderer, dt);

		m_Graphics["effect_koka_white"].Render(renderer, dt);
		m_Graphics["effect_koka_blue" ].Render(renderer, dt);

		m_Graphics["effect_osaekomi_white"].Render(renderer, dt);
		m_Graphics["effect_osaekomi_blue" ].Render(renderer, dt);

		m_Graphics["effect_tokeda_white"].Render(renderer, dt);
		m_Graphics["effect_tokeda_blue" ].Render(renderer, dt);

		m_Graphics["effect_shido_white"].Render(renderer, dt);
		m_Graphics["effect_shido_blue" ].Render(renderer, dt);

		m_Graphics["effect_hansokumake_white"].Render(renderer, dt);
		m_Graphics["effect_hansokumake_blue" ].Render(renderer, dt);


		if (HasConcluded() && m_Winner)//Render winner animation
		{
			if (GetResult().m_Winner == Winner::Blue)
				m_Graphics["winner_blue"].Render(renderer, dt);

			else if (GetResult().m_Winner == Winner::White)
				m_Graphics["winner_white"].Render(renderer, dt);
		}

		break;



	case State::TransitionToWaiting:

		m_Graphics["osaekomi_bar_border"].Render(renderer, dt);
		m_Graphics["osaekomi_bar" ].Render(renderer, dt);
		m_Graphics["osaekomi_text"].Render(renderer, dt);

		m_Graphics["white_name"].Render(renderer, dt);
		m_Graphics["blue_name" ].Render(renderer, dt);
		m_Graphics["white_club"].Render(renderer, dt);
		m_Graphics["blue_club" ].Render(renderer, dt);

		RenderScore(dt);
		RenderTimer(dt);
		RenderShidos(dt);

		m_Graphics["matchtable"].Render(renderer, dt);
		m_Graphics["mat_name"  ].Render(renderer, dt);

		//Render winner animation
		if (m_Graphics["winner_blue"].m_a > 0)
			m_Graphics["winner_blue"].Render(renderer, dt);
		if (m_Graphics["winner_white"].m_a > 0)
			m_Graphics["winner_white"].Render(renderer, dt);

		for (int i = 0; i < 2; i++)
		{
			m_Graphics["next_matches_white_" + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_blue_"  + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_white2_" + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_blue2_"  + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_white_club_" + std::to_string(i)].Render(renderer, dt);
			m_Graphics["next_matches_blue_club_"  + std::to_string(i)].Render(renderer, dt);
		}

		m_Graphics["next_match"].Render(renderer, dt);
		m_Graphics["following_match"].Render(renderer, dt);

		m_Graphics["mat_name"].Render(renderer, dt);

		if (Timer::GetTimestamp() - m_CurrentStateStarted >= 12 * 1000)
			NextState(State::Waiting);

		break;


	case State::Paused:
		if (m_Logo)//Render logo
		{
			m_Logo->SetSize(renderer.GetWidth() / 1920.0f);
			renderer.RenderTransformed(*m_Logo, width / 2 - (int)(m_Logo->GetWidth() / 2.0f * m_Logo->GetSizeX()), 50);
		}

		auto appname = renderer.RenderFont(ZED::FontSize::Middle, Application::Name, ZED::Color(0, 0, 0));
		if (appname)
			renderer.RenderTransformed(appname, 30, height - 120);

		auto version = renderer.RenderFont(ZED::FontSize::Middle, "Version: " + Application::Version, ZED::Color(0, 0, 0));
		if (version)
			renderer.RenderTransformed(version, 30, height - 70);

		auto date = ZED::Core::GetDate();

		std::stringstream day, time;
		day << date.day << "." << date.month << "." << date.year;
		time << std::setfill('0') << std::setw(1) << date.hour << ":" << std::setw(2) << date.minute << ":" << std::setw(2) << date.second;

		auto text_date = renderer.RenderFont(ZED::FontSize::Middle, day.str(), ZED::Color(0, 0, 0));
		if (text_date)
			renderer.RenderTransformed(text_date, width - 250, height - 120);
		auto text_time = renderer.RenderFont(ZED::FontSize::Middle, time.str(), ZED::Color(0, 0, 0));
		if (text_time)
			renderer.RenderTransformed(text_time, width - 250, height - 70);

		auto name = renderer.RenderFont(ZED::FontSize::Gigantic, GetName(), ZED::Color(0, 0, 0));
		if (name)
			renderer.RenderTransformed(name, width / 2 - name->GetWidth() / 2, height - name->GetHeight() - 10);

		auto font = renderer.RenderFont(ZED::FontSize::Gigantic, Localizer::Translate("Pause"), ZED::Color(0, 0, 0));
		renderer.FillRect(ZED::Rect(     -10 + width/2 - font->GetWidth()/2, height/2 - font->GetHeight()/2, font->GetWidth(), font->GetHeight()), 255, 255, 255);
		renderer.RenderTransformed(*font.data, width/2 - font->GetWidth()/2, height/2 - font->GetHeight()/2);
	}





#ifdef _DEBUG
	float frameRate = 1.0f / (float)dt;

	static float frameRate_average = 20.0f;
	static int frameTime_worst = 0;
	const float alpha = 0.01f;

	if ((int)(dt*1000.0f) > frameTime_worst && m_State != State::Waiting)
		frameTime_worst = (int)(dt*1000.0f);

	frameRate_average = (1.0f - alpha) * frameRate_average + alpha * frameRate;
	//if (rand() % 10 == 0)
		//printf("FPS %d\n", (int)frameRate);

	//auto fps = renderer.RenderFont(ZED::FontSize::Small, "FPS: " + std::to_string((int)frameRate) + " (avg: " + std::to_string((int)frameRate_average) + ")", ZED::Color(255, 0, 255));
	auto fps = renderer.RenderFont(ZED::FontSize::Small, "FPS: " + std::to_string((int)frameRate), ZED::Color(255, 0, 255));
	if (fps)
		renderer.RenderTransformed(fps, 5, 5);

	auto draw_calls = renderer.RenderFont(ZED::FontSize::Small, "DrawCalls: " + std::to_string(renderer.GetNumDrawCalls()), ZED::Color(255, 0, 255));
	if (draw_calls)
		renderer.RenderTransformed(draw_calls, 5, 25);

	//auto state = renderer.RenderFont(ZED::FontSize::Middle, "State: " + std::to_string((int)m_State), ZED::Color(255, 0, 255));
	//renderer.RenderTransformed(state, 5, 125);

	//auto worst = renderer.RenderFont(ZED::FontSize::Large, "W: " + std::to_string(frameTime_worst) + "ms", ZED::Color(255, 0, 255));
	//renderer.RenderTransformed(worst, 5, 185);

	renderer.ResetNumDrawCalls();
#endif

	m_mutex.UnlockRead();

	renderer.UpdateDisplay();

	if (m_RequestScreenshot)
	{
		ZED::Log::Debug("Taking screenshot");

		ZED::PNG image = renderer.TakeScreenshot();
		
		if (image)
		{
			auto lambda = [&](ZED::PNG&& Img) {
				Img.ConvertTo(ZED::ColorType::R8G8B8);
				if (renderer.GetType() == ZED::Type::OpenGL)
					Img.Flip();
				m_Screenshot = ZED::PNG::Compress(Img);//Compress image
				//Img.Save("screenshot.png");
			};

			std::thread(lambda, std::move(image)).detach();
		}

		m_RequestScreenshot = false;
	}

	renderer.Unlock();

	return true;
}



bool Mat::Mainloop()
{
	m_Window.HandleEvents();

	auto frameStart = Timer::GetTimestamp();

	if (m_FrameCount%10 == 0 && m_State != State::Running && m_Application)
	{
		bool success;
		auto nextMatches = m_Application->GetNextMatches(GetMatID(), success);

		if (success)
		{
			auto guard = m_mutex.LockWriteForScope();

			if (nextMatches.size() == 0 && m_State == State::Waiting)
				NextState(State::StartUp);

			m_NextMatches = std::move(nextMatches);
		}
	}

	Process();

	Render((double)m_LastFrameTime * 0.001f);

	if (m_QueueSound)
	{
		PlaySoundFile();
		m_QueueSound = false;
	}

	//Calculate frame time
	uint32_t target_frameTime = 40;//25 FPS when idle

	if (IsDoingAnimation() || AreFightersOnMat())
	{
		if (m_Window.GetRenderer().GetType() != ZED::Type::OpenGL)
		{
			target_frameTime = 100;
			ZED::Core::Pause(50);
		}
		else
			target_frameTime = 15;
	}

	else
	{
		if (m_Window.GetRenderer().GetType() != ZED::Type::OpenGL)
			target_frameTime = 250;

		int time_to_pause = target_frameTime - (Timer::GetTimestamp() - frameStart);
		if (time_to_pause > 0)
			ZED::Core::Pause(time_to_pause);
	}

	while (Timer::GetTimestamp() - frameStart < target_frameTime)
	{
		Process();
		ZED::Core::Pause(1);
	}

	m_LastFrameTime = Timer::GetTimestamp() - frameStart;
	m_FrameCount++;

	return true;
}



void Mat::AddEvent(MatchLog::NeutralEvent NewEvent)
{
	if (m_pMatch)
		m_pMatch->GetLog().AddEvent(NewEvent, m_HajimeTimer.GetElapsedTime());
}



void Mat::AddEvent(Fighter Whom, MatchLog::BiasedEvent NewEvent)
{
	if (m_pMatch)
		m_pMatch->GetLog().AddEvent(Whom, NewEvent, m_HajimeTimer.GetElapsedTime());
}



Match::Result Mat::GetResult() const
{
	auto guard = m_mutex.LockReadForScope();

	auto time = m_HajimeTimer.GetElapsedTime();

	if (IsGoldenScore() && m_pMatch)
		time += m_MatchTimeBeforeGoldenScore;//Total time (with last second osaekomi) before golden score


	//Double hansokumake
	if (GetScoreboard(Fighter::White).m_HansokuMake && GetScoreboard(Fighter::Blue).m_HansokuMake)
		return Match::Result(Winner::Draw, Match::Score::Draw, time);

	//Double gachi
	if (GetScoreboard(Fighter::White).m_Gachi && GetScoreboard(Fighter::Blue).m_Gachi)
		return Match::Result(Winner::Draw, Match::Score::Draw, time);

	if (GetScoreboard(Fighter::White).m_Ippon)
		return Match::Result(Winner::White, Match::Score::Ippon, time);
	if (GetScoreboard(Fighter::Blue).m_Ippon)
		return Match::Result(Winner::Blue, Match::Score::Ippon, time);

	if (GetScoreboard(Fighter::White).m_WazaAri > GetScoreboard(Fighter::Blue).m_WazaAri)
		return Match::Result(Winner::White, Match::Score::Wazaari, time);
	if (GetScoreboard(Fighter::White).m_WazaAri < GetScoreboard(Fighter::Blue).m_WazaAri)
		return Match::Result(Winner::Blue, Match::Score::Wazaari, time);

	if (GetScoreboard(Fighter::White).m_Yuko > GetScoreboard(Fighter::Blue).m_Yuko)
		return Match::Result(Winner::White, Match::Score::Yuko, time);
	if (GetScoreboard(Fighter::White).m_Yuko < GetScoreboard(Fighter::Blue).m_Yuko)
		return Match::Result(Winner::Blue, Match::Score::Yuko, time);

	if (GetScoreboard(Fighter::White).m_Koka > GetScoreboard(Fighter::Blue).m_Koka)
		return Match::Result(Winner::White, Match::Score::Koka, time);
	if (GetScoreboard(Fighter::White).m_Koka < GetScoreboard(Fighter::Blue).m_Koka)
		return Match::Result(Winner::Blue, Match::Score::Koka, time);

	//if (GetScoreboard(Fighter::White).m_Shido < GetScoreboard(Fighter::Blue).m_Shido)
		//return Match::Result(Winner::White, Match::Score::Hantei, time);
	//if (GetScoreboard(Fighter::White).m_Shido > GetScoreboard(Fighter::Blue).m_Shido)
		//return Match::Result(Winner::Blue, Match::Score::Hantei, time);

	if (GetScoreboard(Fighter::White).m_Hantei)
		return Match::Result(Winner::White, Match::Score::Hantei, time);
	if (GetScoreboard(Fighter::Blue).m_Hantei)
		return Match::Result(Winner::Blue, Match::Score::Hantei, time);

	return Match::Result(Winner::Draw, Match::Score::Draw, time);
}