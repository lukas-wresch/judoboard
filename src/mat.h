#pragma once
#include <string>
#include <thread>
#include "../ZED/include/read_write_mutex.h"
#include <functional>
#include "judoka.h"
#include "timer.h"
#include "match.h"
#include "imat.h"
#include "rule_set.h"
#include "window.h"
#include "../ZED/include/sound.h"



namespace Judoboard
{
	class Application;



	class Mat final : public IMat
	{
		friend class Application;

	public:
		Mat(uint32_t ID, const Application* App = nullptr);
		Mat(Mat&) = delete;
		Mat(const Mat&) = delete;
		~Mat() { m_State = State::Waiting; Close(); }

		//Virtuals
		virtual Type GetType() const override { return IMat::Type::LocalMat; };
		virtual bool IsOpen()  const override { return m_Window.IsRunning(); }
		virtual bool Open()  override;
		virtual bool Close() override;
		virtual bool Pause(bool Enable = true) override {
			if (!IsPaused() && Enable)
			{
				if (m_State == State::StartUp || m_State == State::Waiting)
				{
					NextState(State::Paused);
					return true;
				}
			}
			else if (IsPaused() && !Enable)
			{
				NextState(State::Waiting);
				return true;
			}

			return false;
		}
		virtual bool IsPaused() const override { return m_State == State::Paused; }

		virtual bool HasConcluded() const override;

		virtual std::vector<OsaekomiEntry> GetOsaekomiList() const override {
			m_mutex.LockRead();
			auto copy = m_OsaekomiList;
			m_mutex.UnlockRead();
			return copy;
		}

		virtual ZED::Blob RequestScreenshot() const override;

		//Basics
		const Judoka& GetFighter(Fighter Who) const { if (Who == Fighter::White) return m_White; return m_Blue; }
		std::string GetTime2String() const { return m_HajimeTimer.ToString(); }
		uint32_t GetTimeElapsed() const { return m_HajimeTimer.GetElapsedTime(); }
		uint32_t GetTime2Display() const;
		bool IsIppon() const { return GetScoreboard(Fighter::White).m_Ippon >= 1 || GetScoreboard(Fighter::Blue).m_Ippon >= 1; }
		bool IsHajime() const override { return m_HajimeTimer.IsRunning(); }
		bool IsOsaekomiRunning() const override { return m_OsaekomiTimer[0].IsRunning() || m_OsaekomiTimer[1].IsRunning(); }//Returns true during an osaekomi situation
		bool IsOsaekomi() const override { return m_IsOsaekomi; }//Returns true during an osaekomi situation (even during sonomama!)
		bool IsSonomama() const { return !IsHajime() && IsOsaekomi(); }
		Fighter GetOsaekomiHolder() const { return m_OsaekomiHolder; }
		bool CanNextMatchStart() const override { return m_State == State::Waiting; }
		bool IsDoingAnimation() const { return m_State == State::TransitionToMatch || m_State == State::TransitionToWaiting; }
		bool AreFightersOnMat() const override { return m_State == State::TransitionToMatch || m_State == State::Running; }
		const Match* GetMatch() const override { return m_pMatch; }
		const std::vector<Match> GetNextMatches() const override;

		uint32_t EndTimeOfOsaekomi() const;//Returns the number of seconds the osaekomi end the match

		virtual bool IsOutOfTime() const override;

		virtual bool IsGoldenScore() const override { return m_GoldenScore; }
		virtual bool EnableGoldenScore(bool GoldenScore = true) override;

		const Window& GetWindow() const { return m_Window; }

		//Commands by judge
		virtual bool StartMatch(Match* NewMatch, bool UseForce = false) override;
		virtual bool EndMatch() override;

		virtual void Hajime() override;
		virtual void Mate() override;
		virtual bool WasMateRecent() const override { return AreFightersOnMat() && Timer::GetTimestamp() - m_MateTimestamp < 3000; }
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
		virtual void RevokeHantei() override;
		virtual void SetAsDraw(bool Enable = true) override;

		virtual void AddShido(Fighter Whom) override;
		virtual void RemoveShido(Fighter Whom) override;

		virtual void AddHansokuMake(Fighter Whom, bool Direct = true) override;
		virtual void RemoveHansokuMake(Fighter Whom) override;

		virtual void AddDisqualification(Fighter Whom) override;
		virtual void AddNoDisqualification(Fighter Whom) override;
		virtual void RemoveDisqualification(Fighter Whom) override;
		virtual void RemoveNoDisqualification(Fighter Whom) override;

		virtual void AddMedicalExamination(Fighter Whom) override;
		virtual void RemoveMedicalExamination(Fighter Whom) override;

		virtual void AddGachi(Fighter Whom) override;
		virtual void RemoveGachi(Fighter Whom) override;

		virtual void Osaekomi(Fighter Whom) override;
		virtual void Tokeda() override;

		uint32_t GetMaxMatchTime();

		//Serialization
		virtual void ToString(YAML::Emitter& Yaml) const override;

		virtual const Scoreboard& GetScoreboard(Fighter Whom) const override
		{
			if (Whom == Fighter::White)
				return m_Scoreboards[0];
			return m_Scoreboards[1];
		}

		//Config
		virtual void SetFullscreen(bool Enabled = true, int Monitor = -1) override
		{
			m_Window.Fullscreen(Enabled, Monitor);
			IMat::SetIsFullscreen(Enabled);
			IMat::SetMonitorIndex(Monitor);
		}

		virtual int GetMonitor() const override
		{
			return GetMonitorIndex();
		}

		virtual void SetName(const std::string& NewName) override
		{
			m_mutex.LockWrite();
			IMat::SetName(NewName);
			m_mutex.UnlockWrite();
		}

		virtual void SetSoundFilename(const std::string& NewFilename) override
		{
			IMat::SetSoundFilename(NewFilename);

			m_Sound = std::move(ZED::Sound("assets/sounds/" + GetSoundFilename() + ".wav"));
			if (m_Sound)
				ZED::Log::Info("Sound file loaded");
			else
				ZED::Log::Warn("Could not load sound file");
		}


	private:
		virtual void PlaySoundFile() const override { m_Sound.Play(); }
		virtual void StopSoundFile() const override { m_Sound.Stop(); }


		Scoreboard& SetScoreboard(Fighter Whom)
		{
			if (Whom == Fighter::White)
				return m_Scoreboards[0];
			return m_Scoreboards[1];
		}

		Scoreboard m_Scoreboards[2];

		enum class State
		{
			StartUp,//Showing logo
			Waiting,//Waiting for the next fight to be issued
			TransitionToMatch,
			Running,//Running a match
			TransitionToWaiting,
			Paused,//Mat is paused
		} mutable m_State = State::StartUp;


		struct GraphicElement;

		class Animation
		{
		public:
			enum class Type
			{
				MoveConstantVelocity,
				ScaleSinus
			};

			static Animation CreateLinear(double vx, double vy, double va = 0.0f, std::function<bool(const GraphicElement&)> AnimateTill = nullptr)
			{
				Animation ret;
				ret.m_Type = Type::MoveConstantVelocity;
				ret.m_AnimateTill = AnimateTill;
				ret.m_vx = vx;
				ret.m_vy = vy;
				ret.m_va = va;
				return ret;
			}

			static Animation CreateScaleSinus(double Amplitude, double Frequency, double BaseSize = 1.0, double DampingLinear = 0.0, double DampingQuadratic = 0.0, std::function<bool(const GraphicElement&)> AnimateTill = nullptr)
			{
				Animation ret;
				ret.m_Type = Type::ScaleSinus;
				ret.m_AnimateTill = AnimateTill;
				ret.m_Amplitude = Amplitude;
				ret.m_Frequency = Frequency;
				ret.m_BaseSize  = BaseSize;
				ret.m_DampingLinear    = DampingLinear;
				ret.m_DampingQuadratic = DampingQuadratic;
				return ret;
			}

			bool Process(GraphicElement& Graphic, double dt);//Returns true when the animation ends

			bool IsRunInParallel() const { return m_RunInParallelToNextAnimation; }
			void RunInParallel(bool Enable = true) { m_RunInParallelToNextAnimation = Enable; }

		private:
			Animation() = default;

			Type m_Type = Type::MoveConstantVelocity;
			std::function<bool(const GraphicElement&)> m_AnimateTill;
			std::function<void(Mat&)> m_onEnd = nullptr;//Will be executed when the animation ends

			bool m_RunInParallelToNextAnimation = false;//If true the next animation will be executed at the same time

			union
			{
				struct {//For linear move animation
					double m_vx, m_vy, m_va;
				};

				struct {//For scale animation
					double m_Amplitude, m_Frequency, m_BaseSize, m_DampingLinear, m_DampingQuadratic;
				};
			};
		};


		struct GraphicElement
		{
			GraphicElement() = default;
			//GraphicElement(ZED::Texture* Tex) : m_Texture(Tex) {}

			void Reset()
			{
				m_Texture.Reset();
			}

			void Clear()
			{
				m_Name.clear();

				if (m_Texture)
					m_Texture->Unload();
			}

			GraphicElement& UpdateTexture(const ZED::Renderer& Renderer, const std::string& Text, ZED::Color Color, ZED::FontSize Size = ZED::FontSize::Huge)
			{
				if (m_Name == Text) return *this;

				m_Texture = Renderer.RenderFont(Size, Text, Color);
				m_Size = Size;
				m_Name = Text;
				m_Color = Color;
				return *this;
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
				for (size_t i = 0; i < m_Animations.size(); ++i)
				{
					if (m_Animations[i].Process(*this, dt))
					{
						//Animation has finished
						m_Animations.erase(m_Animations.begin() + i);
						i--;
					}

					else if (!m_Animations[i].IsRunInParallel())//Run next animations as well?
						break;
				}

				if (m_CustomRenderRoutine)
					m_CustomRenderRoutine(Renderer);

				else if (!m_Texture)//Render quad
				{
					if (m_width > 0 && m_height > 0 && m_a > 0.0)
					{
						ZED::Rect rect((int)m_x, (int)m_y, m_width, m_height);
						if (m_a > 255.0)
							Renderer.FillRect(rect, m_color.r, m_color.g, m_color.b, 255);
						else
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

					if (m_Alignment == Alignment::Left)
						Renderer.RenderTransformed(m_Texture, (int)m_x, (int)m_y);
					else if (m_Alignment == Alignment::Center)
						Renderer.RenderTransformed(m_Texture, (int)m_x - (int)(m_Texture->GetWidth()*m_Texture->GetSizeX()/2.0f), (int)m_y - (int)(m_Texture->GetHeight()*m_Texture->GetSizeY()/2.0f));
					else if (m_Alignment == Alignment::Right)
						Renderer.RenderTransformed(m_Texture, (int)m_x - (int)(m_Texture->GetWidth()*m_Texture->GetSizeX()), (int)m_y);
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

			GraphicElement& SetWidth(int Width)
			{
				m_width = Width;
				return *this;
			}

			GraphicElement& SetHeight(int Height)
			{
				m_height = Height;
				return *this;
			}

			GraphicElement& SetSize(int Width, int Height)
			{
				m_width  = Width;
				m_height = Height;
				return *this;
			}

			GraphicElement& SetTexture(ZED::Texture* NewTexture)
			{
				m_Texture = NewTexture;
				return *this;
			}

			GraphicElement& SetAlpha(double a)
			{
				m_a = a;
				return *this;
			}

			GraphicElement& SetColor(ZED::Color Color)
			{
				m_color = Color;
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
				m_Alignment = Alignment::Left;
				return *this;
			}

			GraphicElement& Center()
			{
				m_Alignment = Alignment::Center;
				return *this;
			}

			GraphicElement& Right()
			{
				m_Alignment = Alignment::Right;
				return *this;
			}

			GraphicElement& Centralize()//Change alignment to center without changing the position of the graphics element
			{
				if (m_Alignment == Alignment::Left)
				{
					m_x += m_Texture->GetWidth()  * m_Texture->GetSizeX() / 2;
					m_y += m_Texture->GetHeight() * m_Texture->GetSizeY() / 2;
				}
				else if (m_Alignment == Alignment::Right)
				{
					m_x -= m_Texture->GetWidth()  * m_Texture->GetSizeX() / 2;
					m_y -= m_Texture->GetHeight() * m_Texture->GetSizeY() / 2;
				}
				m_Alignment = Alignment::Center;
				return *this;
			}

			auto& GetAnimations() { return m_Animations; }

			ZED::Texture* operator->() { return m_Texture; }
			const ZED::Texture* operator->() const { return m_Texture; }
			//operator ZED::Texture* () { return m_Texture; }
			operator bool () { return m_Texture && m_Texture->IsLoaded(); }

			ZED::Ref<ZED::Texture> m_Texture;
			//ZED::Texture* m_Texture = nullptr;

			ZED::FontSize m_Size = ZED::FontSize::Huge;
			std::string m_Name;
			ZED::Color m_Color{0, 0, 0};

			std::vector<Animation> m_Animations;
			std::function<void(const ZED::Renderer& Renderer)> m_CustomRenderRoutine;

			double m_x = 0.0, m_y = 0.0, m_a = 0.0;
			int m_width = 0, m_height = 0;
			ZED::Color m_color{ 0, 0, 0, 0 };

			enum class Alignment
			{
				Left, Center, Right
			} m_Alignment = Alignment::Left;
		};


		//Processing
		void Process();
		void UpdateGraphics() const;
		void UpdateOsaekomiGraphics() const;
		void RenderScore(double dt) const;
		void RenderTimer(double dt) const;
		void RenderShidos(double dt) const;
		bool Render(double dt) const;

		bool Mainloop();

		void RenderBackgroundEffect(const float alpha) const;

		bool Reset();
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
		uint32_t m_MateTimestamp = 0;//Timestam of last mate command

		std::vector<OsaekomiEntry> m_OsaekomiList;
		bool m_IsOsaekomi = false;
		Fighter m_OsaekomiHolder = Fighter::White;

		bool m_GoldenScore = false;
		bool m_IsDraw = false;

		Match* m_pMatch = nullptr;//Current match (if the current fight is associated with a match)

		const Application* m_Application;

		mutable volatile bool m_RequestScreenshot = false;//True if a screenshot has been requested. Will be false after the screenshot has been saved
		mutable ZED::Blob m_Screenshot;
		uint32_t m_LastFrameTime = 40;

		std::thread m_Thread;//Thread for running the main loop
		//mutable std::recursive_mutex m_mutex;
		mutable ZED::RecursiveReadWriteMutex m_mutex;

		//Graphics
		mutable std::unordered_map<std::string, GraphicElement> m_Graphics;

		mutable ZED::Ref<ZED::Texture> m_Background;
		mutable ZED::Ref<ZED::Texture> m_Logo;
		mutable ZED::Ref<ZED::Texture> m_Winner;
    
		mutable ZED::Sound m_Sound;//Sound signal

		double m_ScalingFactor = 1.0;//Should be 1.0 for 1080p screen, smaller for smaller screen and > 1.0 for larger screens

		mutable volatile uint32_t m_FrameCount = 0;
	};
}