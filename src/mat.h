#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <functional>
#include "judoka.h"
#include "timer.h"
#include "match.h"
#include "imat.h"
#include "rule_set.h"
#include "window.h"



namespace Judoboard
{
	class Application;



	class Mat final : public IMat
	{
		friend class Application;

	public:
		struct Scoreboard
		{
			enum class DisqualificationState
			{
				Unknown, Disqualified, NotDisqualified
			};

			void Reset()
			{
				m_Ippon = m_WazaAri = m_Yuko = m_Koka = 0;
				m_Shido = 0;

				m_HansokuMake = false;
				m_Disqualification = DisqualificationState::Unknown;

				m_MedicalExamination = 0;
				m_Hantei = false;
				m_Gachi = false;
			}

			bool IsUnknownDisqualification() const { return m_HansokuMake && m_Disqualification == DisqualificationState::Unknown; }
			bool IsDisqualified()    const { return m_HansokuMake && m_Disqualification == DisqualificationState::Disqualified; }
			bool IsNotDisqualified() const { return m_HansokuMake && m_Disqualification == DisqualificationState::NotDisqualified; }

			uint16_t m_Ippon = 0;
			uint16_t m_WazaAri = 0;
			uint16_t m_Yuko = 0;
			uint16_t m_Koka = 0;

			uint16_t m_Shido = 0;
			uint16_t m_MedicalExamination = 0;

			DisqualificationState m_Disqualification = DisqualificationState::Unknown;//Did the judoka get disqualified

			bool m_HansokuMake = false;

			bool m_Hantei = false;

			bool m_Gachi = false;//Fusen or kiken gachi
		};


		Mat(uint32_t ID, const Application* App = nullptr);
		Mat(Mat&) = delete;
		Mat(const Mat&) = delete;
		~Mat() { m_State = State::Waiting; Close(); }

		//Virtuals
		Type GetType() const override { return IMat::Type::LocalMat; };
		bool IsOpen()  const override { return m_Window.IsRunning(); }
		bool Open()  override;
		bool Close() override;

		//void ClearNextMatches() override;
		//void AddNextMatch(const Match* NextMatch) override;
		bool HasConcluded() const override;

		const std::vector<OsaekomiEntry>& GetOsaekomiList() const override { return m_OsaekomiList; }

		bool RequestScreenshot() const override;

		//Basics
		const Judoka& GetFighter(Fighter Who) const { if (Who == Fighter::White) return m_White; return m_Blue; }
		std::string GetTime2String() const { return m_HajimeTimer.ToString(); }
		uint32_t GetTimeElapsed() const { return m_HajimeTimer.GetElapsedTime(); }
		uint32_t GetTime2Display() const;
		bool IsIppon() const { return GetScoreboard(Fighter::White).m_Ippon >= 1 || GetScoreboard(Fighter::Blue).m_Ippon >= 1; }
		bool IsHajime() const override { return m_HajimeTimer.IsRunning(); }
		bool IsOsaekomiRunning() const override { return m_OsaekomiTimer[0].IsRunning() || m_OsaekomiTimer[1].IsRunning(); }//Returns true during an osaekomi situation
		bool IsOsaekomi() const override { return IsOsaekomiRunning() || m_OsaekomiTimer[0].GetElapsedTime() > 0 || m_OsaekomiTimer[1].GetElapsedTime() > 0; }//Returns true during an osaekomi situation (even during yoshi!)
		Fighter GetOsaekomiHolder() const { if (m_OsaekomiTimer[(int)Fighter::White].GetElapsedTime() > 0) return Fighter::White; return Fighter::Blue; }
		bool CanNextFightStart() const override { return m_State == State::Waiting; }
		bool IsDoingAnimation() const { return m_State == State::TransitionToMatch || m_State == State::TransitionToWaiting; }
		bool AreFightersOnMat() const override { return m_State == State::TransitionToMatch || m_State == State::Running; }
		const Match* GetMatch() const override { return m_pMatch; }
		const std::vector<const Match*> GetNextMatches() const override;

		uint32_t EndTimeOfOsaekomi() const;//Returns the number of seconds the osaekomi end the match

		bool IsOutOfTime() const override;

		bool IsGoldenScore() const override { return m_GoldenScore; }
		bool EnableGoldenScore(bool GoldenScore = true) override;

		const Scoreboard& GetScoreboard(Fighter Whom) const;

		const Window& GetWindow() const { return m_Window; }

		//Commands by judge
		virtual bool StartMatch(Match& NewMatch) override;
		virtual bool EndMatch() override;

		virtual void Hajime() override;
		virtual void Mate() override;
		virtual void Sonomama() override;

		virtual void AddIppon(Fighter Whom) override;
		virtual void RemoveIppon(Fighter Whom) override;

		virtual void AddWazaAri(Fighter Whom) override;
		virtual void RemoveWazaAri(Fighter Whom) override;

		virtual void AddYuko(Fighter Whom) override;
		virtual void RemoveYuko(Fighter Whom) override;

		virtual void AddKoka(Fighter Whom) override;
		virtual void RemoveKoka(Fighter Whom) override;

		virtual void Hantei(Fighter Whom) override;
		virtual void SetAsDraw(bool Enable = true) override;

		virtual void AddShido(Fighter Whom) override;
		virtual void RemoveShido(Fighter Whom) override;

		virtual void AddHansokuMake(Fighter Whom, bool Direct = true) override;
		virtual void RemoveHansokuMake(Fighter Whom) override;

		virtual void AddDisqualification(Fighter Whom) override;
		virtual void AddNotDisqualification(Fighter Whom) override;
		virtual void RemoveDisqualification(Fighter Whom) override;

		virtual void AddMedicalExamination(Fighter Whom) override;
		virtual void RemoveMedicalExamination(Fighter Whom) override;

		virtual void AddGachi(Fighter Whom) override;
		virtual void RemoveGachi(Fighter Whom) override;

		virtual void Osaekomi(Fighter Whom) override;
		virtual void Tokeda() override;

		//Serialization
		ZED::CSV Scoreboard2String() const override;
		ZED::CSV Osaekomi2String(Fighter Who) const override;

	private:
		Scoreboard m_Scoreboards[2];


		enum class State
		{
			StartUp,//Showing logo
			Waiting,//Waiting for the next fight to be issued
			TransitionToMatch,
			Running,//Running a match
			TransitionToWaiting
		} mutable m_State = State::StartUp;


		struct GraphicElement;

		struct Animation
		{
			enum class Type
			{
				MoveConstantVelocity
			};

			Animation(double vx, double vy, double va = 0.0f, std::function<bool(const GraphicElement&)> AnimateTill = nullptr) : m_AnimateTill(AnimateTill)
			{
				m_Type = Type::MoveConstantVelocity;
				m_vx = vx;
				m_vy = vy;
				m_va = va;
			}

			bool Process(GraphicElement& Graphic, double dt);

			Type m_Type;
			std::function<bool(const GraphicElement&)> m_AnimateTill;
			std::function<void(Mat&)> m_onEnd = nullptr;//Will be executed when the animation ends

			uint32_t m_sx = 0, m_sy = 0;
			uint32_t m_ex = 0, m_ey = 0;

			double m_vx, m_vy, m_va;
		};


		struct GraphicElement
		{
			GraphicElement() = default;
			//GraphicElement(ZED::Texture* Tex) : m_Texture(Tex) {}

			void Clear()
			{
				m_Name.clear();

				if (m_Texture)
					m_Texture->Unload();
			}

			void UpdateTexture(const ZED::Renderer& Renderer, const std::string& Text, ZED::Color Color, ZED::FontSize Size = ZED::FontSize::Huge)
			{
				if (m_Name == Text) return;

				m_Texture = Renderer.RenderFont(Size, Text, Color);
				m_Size = Size;
				m_Name = Text;
				m_Color = Color;
			}

			void Shorten(const ZED::Renderer& Renderer)//Shortens the text by one character
			{
				if (m_Name.back() == '.')
					m_Name = m_Name.substr(0, m_Name.length()-2) + ".";
				else
					m_Name = m_Name.substr(0, m_Name.length()-1) + ".";

				m_Texture = Renderer.RenderFont(m_Size, m_Name, m_Color);
			}

			void Render(const ZED::Renderer& Renderer, double dt)
			{
				if (m_Animations.size() > 0)
					if (m_Animations.front().Process(*this, dt))
						m_Animations.erase(m_Animations.begin());

				if (m_CustomRenderRoutine)
					m_CustomRenderRoutine(Renderer);

				else if (!m_Texture)//Render quad
				{
					if (m_width > 0 && m_height > 0 && m_a > 0.0)
					{
						ZED::Rect rect((int)m_x, (int)m_y, m_width, m_height);
						Renderer.FillRect(rect, m_color.r, m_color.g, m_color.b, (int)m_a);
					}
				}

				else//Render texture
				{
					if (m_a > 255.0)
						m_Texture->SetAlpha(255);
					else if (m_a <= 0.0)
					{
						m_Texture->SetAlpha(0);
						return;
					}
					else
						m_Texture->SetAlpha((unsigned char)m_a);

					if (m_Aligment == Aligment::Left)
						Renderer.RenderTransformed(*m_Texture.data, (int)m_x, (int)m_y);
					else if (m_Aligment == Aligment::Center)
						Renderer.RenderTransformed(*m_Texture.data, (int)m_x - m_Texture->GetWidth()/2, (int)m_y-m_Texture->GetHeight()/2);
					else if (m_Aligment == Aligment::Right)
						Renderer.RenderTransformed(*m_Texture.data, (int)m_x - m_Texture->GetWidth(), (int)m_y);
				}
			}

			GraphicElement& SetPosition(int X, int Y, int A)
			{
				m_x = X;
				m_y = Y;
				m_a = A;
				return *this;
			}

			GraphicElement& SetPosition(int X, int Y)
			{
				m_x = X;
				m_y = Y;
				return *this;
			}

			GraphicElement& SetAlpha(double a)
			{
				m_a = a;
				return *this;
			}

			GraphicElement& AddAnimation(const Animation& NewAnimation)
			{
				m_Animations.emplace_back(NewAnimation);
				return *this;
			}

			GraphicElement& StopAllAnimations()
			{
				m_Animations.clear();
				return *this;
			}

			GraphicElement& Left()
			{
				m_Aligment = Aligment::Left;
				return *this;
			}

			GraphicElement& Center()
			{
				m_Aligment = Aligment::Center;
				return *this;
			}

			GraphicElement& Right()
			{
				m_Aligment = Aligment::Right;
				return *this;
			}

			ZED::Texture* operator->() { return m_Texture; }
			const ZED::Texture* operator->() const { return m_Texture; }
			operator ZED::Texture* () { return m_Texture; }

			ZED::Ref<ZED::Texture> m_Texture;

			ZED::FontSize m_Size = ZED::FontSize::Huge;
			std::string m_Name;
			ZED::Color m_Color{0, 0, 0};

			std::vector<Animation> m_Animations;
			std::function<void(const ZED::Renderer& Renderer)> m_CustomRenderRoutine;

			double m_x = 0.0, m_y = 0.0, m_a = 0.0;
			int m_width = 0, m_height = 0;
			ZED::Color m_color{ 0, 0, 0, 0 };

			enum class Aligment
			{
				Left, Center, Right
			} m_Aligment = Aligment::Left;
		};


		void SetName(const std::string& NewName) override
		{
			m_mutex.lock();
			IMat::SetName(NewName);
			m_mutex.unlock();
		}

		//Processing
		void Process();
		void UpdateGraphics() const;
		void RenderScore(double dt) const;
		void RenderTimer(double dt) const;
		void RenderShidos(double dt) const;
		bool Render(double dt) const;

		bool Mainloop();

		void RenderBackgroundEffect(const float alpha) const;

		bool Reset();
		Scoreboard& SetScoreboard(Fighter Whom);
		void AddEvent(MatchLog::NeutralEvent NewEvent);
		void AddEvent(Fighter Whom, MatchLog::BiasedEvent NewEvent);
		Match::Result GetResult() const;

		void NextState(State NextState) const;


		Window m_Window;

		Judoka m_White;
		Judoka m_Blue;

		mutable uint32_t m_CurrentStateStarted = 0;//Timestamp when the current state started

		Timer m_HajimeTimer;
		Timer m_OsaekomiTimer[2];
		std::vector<OsaekomiEntry> m_OsaekomiList;

		bool m_GoldenScore = false;
		bool m_IsDraw = false;

		Match* m_pMatch = nullptr;//Current match (if the current fight is associated with a match)

		const Application* m_Application;

		mutable volatile bool m_RequestScreenshot = false;//True if a screenshot has been requested. Will be false after the screenshot has been saved
		uint32_t m_LastFrameTime = 40;

		std::thread m_Thread;//Thread for running the main loop
		mutable std::mutex m_mutex;

		//Graphics
		mutable std::unordered_map<std::string, GraphicElement> m_Graphics;

		mutable ZED::Ref<ZED::Texture> m_Background;
		mutable ZED::Ref<ZED::Texture> m_Logo;

		double m_ScalingFactor = 1.0;//Should be 1.0 for 1080p screen, smaller for smaller screen and > 1.0 for larger screens
	};
}