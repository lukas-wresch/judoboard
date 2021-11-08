#pragma once
#include "match.h"
#include "localizer.h"
#include <ZED/include/csv.h>



namespace Judoboard
{
	class IMat
	{
		friend class Application;

	public:
		enum class Type
		{
			Unknown, LocalMat, VirtualMat, RemoteMat
		};


		enum class IpponDisplayStyle
		{
			DoubleDigit,//Show score like: 0 0, 0 1, 1 0
			SingleDigit,//Show score like: 0, 1, 2
			SpelledOut//  Show score like: *none*, 1 Wazaari, 2 Wazaari, Ippon
		};


		struct OsaekomiEntry
		{
			OsaekomiEntry(Fighter Who, uint32_t Time) : m_Who(Who), m_Time(Time) {}

			Fighter m_Who;//Fighter who archieved the osaekomi
			uint32_t m_Time;//Length of the osaekomi in millseconds
		};


		IMat(uint32_t ID) { m_ID = ID; IMat::SetName(Localizer::Translate("Mat") + " " + std::to_string(ID)); }
		virtual ~IMat() {}

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
		virtual const std::vector<const Match*> GetNextMatches() const { return m_NextMatches; }

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
		virtual void SetAsDraw(bool Enable = true) = 0;

		virtual void AddShido(Fighter Whom) = 0;
		virtual void RemoveShido(Fighter Whom) = 0;

		virtual void AddHansokuMake(Fighter Whom, bool Direct = true) = 0;
		virtual void RemoveHansokuMake(Fighter Whom) = 0;

		virtual void AddDisqualification(Fighter Whom) = 0;
		virtual void AddNotDisqualification(Fighter Whom) = 0;
		virtual void RemoveDisqualification(Fighter Whom) = 0;

		virtual void AddMedicalExamination(Fighter Whom) = 0;
		virtual void RemoveMedicalExamination(Fighter Whom) = 0;

		virtual void AddGachi(Fighter Whom) = 0;
		virtual void RemoveGachi(Fighter Whom) = 0;

		virtual void Osaekomi(Fighter Whom) = 0;//Starts an osaekomi for fighter 'Whom'
		virtual void Tokeda() = 0;//Stops an osaekomi

		//Output
		virtual Match::Result GetResult() const = 0;
		virtual bool RequestScreenshot() const { return false; }

		//Serialization
		virtual ZED::CSV Scoreboard2String() const = 0;
		virtual ZED::CSV Osaekomi2String(Fighter Who) const = 0;

		//Config
		IpponDisplayStyle GetIpponDisplayStyle() const { return m_DisplayStyle; }

	protected:
		virtual void SetName(const std::string& NewName) { m_Name = NewName; }
		virtual void SetIpponDisplayStyle(IpponDisplayStyle NewStyle) { m_DisplayStyle = NewStyle; }

		std::vector<const Match*> m_NextMatches;

	private:
		std::string m_Name;

		uint32_t m_ID = 1;

		//Configuration
		IpponDisplayStyle m_DisplayStyle = IpponDisplayStyle::DoubleDigit;
		//IpponDisplayStyle m_DisplayStyle = IpponDisplayStyle::SpelledOut;
		//bool m_ShowIpponAsTwoWazaari = true;//If true an ippon will be displayed as 2 wazari instead of 1 ippon & 0 wazarri
	};
}