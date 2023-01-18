#pragma once
#include "match.h"
#include "localizer.h"
#include "../ZED/include/csv.h"
#include "../ZED/include/image.h"



namespace Judoboard
{
	class IMat
	{
		friend class Application;

	public:
		enum class Type
		{
			Unknown = 0, LocalMat, VirtualMat, RemoteMat
		};


		enum class TimerStyle
		{
			HundredsMS,//Show time like:  1:23.5
			OnlySeconds,//Show time like: 1:23
			Full,//Show time like:        1:23.456
		};

		enum class IpponStyle
		{
			DoubleDigit,//Show score like: 0 0, 0 1, 1 0
			SingleDigit,//Show score like: 0, 1, 2
			SpelledOut,//  Show score like: *none*, 1 Wazaari, 2 Wazaari, Ippon
		};


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
				m_HansokuMake_Direct = false;
				m_Disqualification = DisqualificationState::Unknown;

				m_MedicalExamination = 0;
				m_Hantei = false;
				m_Gachi  = false;
			}

			bool IsUnknownDisqualification() const { return m_HansokuMake && m_Disqualification == DisqualificationState::Unknown; }
			bool IsDisqualified()            const { return m_HansokuMake && m_Disqualification == DisqualificationState::Disqualified; }
			bool IsNotDisqualified()         const { return m_HansokuMake && m_Disqualification == DisqualificationState::NotDisqualified; }

			uint16_t m_Ippon = 0;
			uint16_t m_WazaAri = 0;
			uint16_t m_Yuko = 0;
			uint16_t m_Koka = 0;

			uint16_t m_Shido = 0;
			uint16_t m_MedicalExamination = 0;

			DisqualificationState m_Disqualification = DisqualificationState::Unknown;//Did the judoka get disqualified

			bool m_HansokuMake = false;
			bool m_HansokuMake_Direct = false;

			bool m_Hantei = false;

			bool m_Gachi = false;//Fusen or kiken gachi
		};


		struct OsaekomiEntry
		{
			OsaekomiEntry(Fighter Who, uint32_t Time) : m_Who(Who), m_Time(Time) {}

			Fighter m_Who;//Fighter who archieved the osaekomi
			uint32_t m_Time;//Length of the osaekomi in millseconds
		};


		IMat(uint32_t ID) { m_ID = ID; IMat::SetName(Localizer::Translate("Mat") + " " + std::to_string(ID)); }
		virtual ~IMat() {}

		IMat(IMat&) = delete;
		IMat(const IMat&) = delete;

		uint32_t GetMatID() const { return m_ID; }
		void SetMatID(uint32_t NewID) { m_ID = NewID; }

		virtual const std::string& GetName() const { return m_Name; }

		virtual const std::vector<OsaekomiEntry>& GetOsaekomiList() const = 0;

		virtual Type GetType() const = 0;//Returns the type of the mat (local, remote)
		virtual bool IsOpen()  const = 0;//Returns true if and only if the mat is open (connected and ready to receive commands)
		virtual bool Open() = 0;//Opens the mat. Returns true if successful
		virtual bool Close() = 0;//Tries to close the mat. Returns true if successful

		virtual bool IsConnected() const { return true; };//Only relevant for remote mats. Retuns false if and only the connection to the slave server is lost

		//Basics
		virtual bool AreFightersOnMat() const = 0;

		virtual const Match* GetMatch() const { return nullptr; }
		virtual const std::vector<Match> GetNextMatches() const { return m_NextMatches; }

		virtual bool CanNextMatchStart() const = 0;
		virtual bool StartMatch(Match* NewMatch) = 0;//Creates a new match. Both judoka are copied to the mat. Returns false when a match is still progressing and hence a new match can not be started
		virtual bool HasConcluded() const = 0;//Returns true if and only if the match has finished and hence EndMatch() can be called
		virtual bool EndMatch() = 0;//Closes the match that is currently on the mat and resets the scoreboard

		virtual uint32_t GetTimeElapsed()  const = 0;
		virtual uint32_t GetTime2Display() const = 0;

		virtual bool IsHajime() const = 0;//Returns true if and only the match is running and not paused i.e. the hajime timer is running
		virtual bool IsOutOfTime() const = 0;//Returns true if and only if the match has run out of time
		virtual bool IsGoldenScore() const = 0;
		virtual bool EnableGoldenScore(bool GoldenScore = true) = 0;

		virtual bool IsOsaekomiRunning() const = 0;//Returns true if one of the osaekomi clocks are running
		virtual bool IsOsaekomi() const = 0;//Returns true during an osaekomi situation

		//Commands by judge
		virtual void Hajime() = 0;//Starts/continues the match. If an osaekomi is running this commands acts as an yoshi
		virtual void Mate() = 0;//Pauses the match. If an osaekomi is running this commands will stop the osaekomi
		virtual void Sonomama() = 0;//Sonomama. Pauses the match and osaekomi

		virtual void AddIppon(Fighter Whom) = 0;//Adds an ippon on the scoreaboard for fighter 'Whom'
		virtual void RemoveIppon(Fighter Whom) = 0;//Removes an ippon on the scoreaboard for fighter 'Whom'

		virtual void AddWazaAri(Fighter Whom) = 0;
		virtual void RemoveWazaAri(Fighter Whom) = 0;

		virtual void AddYuko(Fighter Whom) = 0;
		virtual void RemoveYuko(Fighter Whom) = 0;

		virtual void AddKoka(Fighter Whom) = 0;
		virtual void RemoveKoka(Fighter Whom) = 0;

		virtual void Hantei(Fighter Whom) = 0;
		virtual void RevokeHantei() = 0;
		virtual void SetAsDraw(bool Enable = true) = 0;

		virtual void AddShido(Fighter Whom) = 0;
		virtual void RemoveShido(Fighter Whom) = 0;

		virtual void AddHansokuMake(Fighter Whom, bool Direct = true) = 0;
		virtual void RemoveHansokuMake(Fighter Whom) = 0;

		virtual void AddDisqualification(Fighter Whom) = 0;
		virtual void AddNoDisqualification(Fighter Whom) = 0;
		virtual void RemoveDisqualification(Fighter Whom) = 0;
		virtual void RemoveNoDisqualification(Fighter Whom) = 0;

		virtual void AddMedicalExamination(Fighter Whom) = 0;
		virtual void RemoveMedicalExamination(Fighter Whom) = 0;

		virtual void AddGachi(Fighter Whom) = 0;
		virtual void RemoveGachi(Fighter Whom) = 0;

		virtual void Osaekomi(Fighter Whom) = 0;//Starts an osaekomi for fighter 'Whom'
		virtual void Tokeda() = 0;//Stops an osaekomi

		//Output
		virtual const Scoreboard& GetScoreboard(Fighter Whom) const = 0;
		virtual Match::Result GetResult() const = 0;
		virtual ZED::Blob RequestScreenshot() const = 0;

		//Serialization
		virtual void ToString(YAML::Emitter& Yaml) const = 0;

		//Config
		IpponStyle GetIpponStyle() const { return m_IpponStyle; }
		TimerStyle GetTimerStyle() const { return m_TimerStyle; }
		NameStyle  GetNameStyle()  const { return m_NameStyle; }
		bool IsFullscreen() const { return m_IsFullscreen; }
		virtual void SetFullscreen(bool Enabled = true) = 0;

		virtual void SetName(const std::string& NewName) { m_Name = NewName; }
		virtual void SetIpponStyle(IpponStyle NewStyle) { m_IpponStyle = NewStyle; }
		virtual void SetTimerStyle(TimerStyle NewStyle) { m_TimerStyle = NewStyle; }
		virtual void SetNameStyle(NameStyle NewStyle) { m_NameStyle = NewStyle; }
		virtual void SetIsFullscreen(bool Enabled) { m_IsFullscreen = Enabled; }

	protected:
		std::vector<Match> m_NextMatches;

	private:
		std::string m_Name;

		uint32_t m_ID = 1;

		//Configuration
		TimerStyle m_TimerStyle = TimerStyle::OnlySeconds;
		IpponStyle m_IpponStyle = IpponStyle::DoubleDigit;
		NameStyle m_NameStyle   = NameStyle::FamilyName;
		bool m_IsFullscreen = true;
	};
}