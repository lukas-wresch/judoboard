#pragma once
#include "imat.h"
#include "../ZED/include/http_client.h"



namespace Judoboard
{
	class RemoteMat : public IMat
	{
	public:
		RemoteMat(uint32_t ID, const std::string& Host, uint16_t Port, const std::string& Token);

		//Virtuals
		virtual Type GetType() const override { return IMat::Type::RemoteMat; };
		virtual bool IsOpen() const override;
		virtual bool Open()  override;
		virtual bool Close() override;
		virtual bool Pause(bool Enable = true) override { return false; }
		virtual bool IsPaused() const override { return false; }

		virtual bool IsConnected() const override;

		virtual std::vector<OsaekomiEntry> GetOsaekomiList() const override;

		virtual bool AreFightersOnMat() const override;

		virtual bool CanNextMatchStart() const override;
		virtual bool StartMatch(Match* NewMatch, bool UseForce = false) override;
		virtual bool HasConcluded() const override;
		virtual bool EndMatch() override;

		virtual uint32_t GetTimeElapsed()  const override { return 0; }
		virtual uint32_t GetTime2Display() const override { return 0; }

		virtual bool IsHajime()      const override { return GetState().hajime; }
		virtual bool IsOutOfTime()   const override { return GetState().isoutoftime; }
		virtual bool IsGoldenScore() const override { return GetState().isgoldenscore; }
		virtual bool EnableGoldenScore(bool GoldenScore = true) override;

		virtual bool IsOsaekomiRunning() const override {//Returns true if one of the osaekomi clocks are running
			auto state = GetState();
			return state.white_osaekomi || state.blue_osaekomi;
		}
		virtual bool IsOsaekomi() const override {//Returns true during an osaekomi situation (even during yoshi!)
			auto state = GetState();
			return state.IsOsaekomi;
		};//Returns true during an osaekomi situation
		virtual Fighter GetOsaekomiHolder() const override {
			if (GetState().white_osaekomi_time > 0)
				return Fighter::White;
			return Fighter::Blue;
		}

		//Commands by judge
		virtual void Hajime() override;
		virtual void Mate() override;
		virtual bool WasMateRecent() const override { return AreFightersOnMat() && Timer::GetTimestamp() - /*m_MateTimestamp*/0 < 3000; }
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
		virtual void RevokeHantei() override {}
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

		//Output
		virtual Match::Result GetResult() const { assert(false); Match::Result ret; return ret; };//NOT IMPLEMENTED
		virtual ZED::Blob RequestScreenshot() const;

		//Serialization
		virtual void ToString(YAML::Emitter& Yaml) const override {}

		//Config
		virtual void SetFullscreen(bool Enabled = true, int Monitor = -1) override
		{
			SendCommand("/ajax/config/set_mat?id=" + std::to_string(GetMatID()), std::string("fullscreen=") + (Enabled ? "true" : "false") + "&monitor=" + std::to_string(Monitor));
			IMat::SetIsFullscreen(Enabled);
			IMat::SetMonitorIndex(Monitor);
		}

		virtual int GetMonitor() const override
		{
			std::string response = SendRequest("/ajax/config/get_mats");
			auto yaml = YAML::Load(response);

			if (yaml["mats"] && yaml["mats"][GetMatID()] && yaml["mats"][GetMatID()]["monitor"])
				return yaml["mats"][GetMatID()]["monitor"].as<int>();
			return -1;
		}

		virtual void SetAudio(bool Enabled, const std::string& NewFilename, int DeviceID) override
		{
			assert(false);//NOT IMPLEMENTED
		}

		std::string GetHostname() const { return m_Hostname; }
		uint16_t GetPort() const { return m_Port; }

	private:
		bool SendCommand(const std::string& URL) const;
		bool SendCommand(const std::string& URL, const std::string& Body) const;
		ZED::HttpClient::Packet SendRequest(const std::string& URL) const;
		ZED::HttpClient::Packet SendRequest(const std::string& URL, const std::string& Body) const;
		bool PostData(const std::string& URL, const YAML::Emitter& Data) const;

		struct InternalState
		{
			//int white_ippon, white_wazaari, white_shidos, white_medical;
			//int blue_ippon, blue_wazaari, blue_shidos, blue_medical;
			uint32_t display_time;
			uint32_t white_osaekomi_time, blue_osaekomi_time;

			bool white_osaekomi, blue_osaekomi;
			bool hajime, IsOsaekomi, cannextmatchstart, hasconcluded, isoutoftime, NoWinnerYet, isgoldenscore, arefightersonmat;
		};

		virtual const Scoreboard& GetScoreboard(Fighter Whom) const override
		{
			GetState();

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

		InternalState GetState(bool* pSuccess = nullptr) const;


		mutable Scoreboard m_Scoreboards[2];
		Match* m_pMatch = nullptr;//Reference to the current match used for caching

		mutable std::vector<OsaekomiEntry> m_OsaekomiList;

		std::string m_Hostname;
		uint16_t m_Port;
		std::string m_SecurityToken;
	};
}