#pragma once
#include "imat.h"
#include "../ZED/include/http_client.h"



namespace Judoboard
{
	class RemoteMat : public IMat
	{
	public:
		RemoteMat(uint32_t ID, const std::string& Host, uint16_t Port);

		//Virtuals
		virtual Type GetType() const override { return IMat::Type::RemoteMat; };
		virtual bool IsOpen() const override;
		virtual bool Open()  override;
		virtual bool Close() override;

		virtual bool IsConnected() const override;

		virtual const std::vector<OsaekomiEntry>& GetOsaekomiList() const override { return m_OsaekomiList; };

		virtual bool AreFightersOnMat() const override;

		virtual bool CanNextMatchStart() const override;
		virtual bool StartMatch(Match* NewMatch) override;
		virtual bool HasConcluded() const override;
		virtual bool EndMatch() override;

		virtual uint32_t GetTimeElapsed()  const override { return 0; }
		virtual uint32_t GetTime2Display() const override { return 0; }

		virtual bool IsHajime()      const override { return false; }
		virtual bool IsOutOfTime()   const override { return false; }
		virtual bool IsGoldenScore() const override { return false; }
		virtual bool EnableGoldenScore(bool GoldenScore = true) override { return false; }

		virtual bool IsOsaekomiRunning() const override { return false; };//Returns true if one of the osaekomi clocks are running
		virtual bool IsOsaekomi() const override { return false; };//Returns true during an osaekomi situation
		virtual Fighter GetOsaekomiHolder() const override { return Fighter::White; }

		//Commands by judge
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
		virtual void RemoveDisqualification(Fighter Whom) override {}

		virtual void AddMedicalExamination(Fighter Whom) override {}
		virtual void RemoveMedicalExamination(Fighter Whom) override {}

		virtual void AddGachi(Fighter Whom) override {}
		virtual void RemoveGachi(Fighter Whom) override {}

		virtual void Osaekomi(Fighter Whom) override {}
		virtual void Tokeda() override {}

		//Output
		virtual Match::Result GetResult() const { Match::Result ret; return ret; };
		virtual ZED::Blob RequestScreenshot() const;

		//Serialization
		virtual ZED::CSV Scoreboard2String() const override { return ""; }
		virtual ZED::CSV Osaekomi2String(Fighter Who) const  override { return ""; }

		//Config
		virtual void SetFullscreen(bool Enabled = true) override
		{
			//TODO
			IMat::SetIsFullscreen(Enabled);
		}

	private:
		bool SendCommand(const std::string& URL) const;
		ZED::HttpClient::Packet SendRequest(const std::string& URL) const;
		bool PostData(const std::string& URL, const ZED::CSV& Data) const;

		struct InternalState
		{
			//int white_ippon, white_wazaari, white_shidos, white_medical;
			//int blue_ippon, blue_wazaari, blue_shidos, blue_medical;
			uint32_t display_time;
			uint32_t white_osaekomi_time, blue_osaekomi_time;

			bool white_osaekomi, blue_osaekomi;
			bool hajime, cannextmatchstart, hasconcluded, isoutoftime_and_draw, isgoldenscore, arefightersonmat;
		};

		virtual const Scoreboard& GetScoreboard(Fighter Whom) const override
		{
			bool success;
			GetState(success);

			if (Whom == Fighter::White)
				return m_Scoreboards[0];
			return m_Scoreboards[1];
		}
		Scoreboard& SetScoreboard(Fighter Whom) const
		{
			if (Whom == Fighter::White)
				return m_Scoreboards[0];
			return m_Scoreboards[1];
		}

		InternalState GetState(bool& Success) const;

		mutable Scoreboard m_Scoreboards[2];

		std::vector<OsaekomiEntry> m_OsaekomiList;

		std::string m_Hostname;
		uint16_t m_Port;
	};
}