#pragma once
#include "imat.h"



namespace Judoboard
{
	class VirtualMat : public IMat
	{
	public:
		VirtualMat(uint32_t ID) : IMat(ID) {}

		//Virtuals
		Type GetType() const override { return IMat::Type::VirtualMat; };
		bool IsOpen() const override { return true; }
		bool Open()  override { return true; }
		bool Close() override { return true; }

		virtual const std::vector<OsaekomiEntry>& GetOsaekomiList() const { return m_OsaekomiList; };

		virtual bool AreFightersOnMat() const { return false; }

		virtual bool CanNextMatchStart() const override { return false; };
		virtual bool StartMatch(Match& NewMatch) { return false; }
		virtual bool HasConcluded() const override { return false; }
		virtual bool EndMatch() = 0;

		virtual uint32_t GetTimeElapsed() const { return 0; }

		virtual bool IsHajime()      const override { return false; }
		virtual bool IsOutOfTime()   const override { return false; }
		virtual bool IsGoldenScore() const override { return false; }
		virtual bool EnableGoldenScore(bool GoldenScore = true) override { return false; }

		//Commands by judge
		virtual void Hajime() {}
		virtual void Mate() {}

		virtual void AddIppon(Fighter Whom) override {}
		virtual void RemoveIppon(Fighter Whom) override {}

		virtual void AddWazaAri(Fighter Whom) override {}
		virtual void RemoveWazaAri(Fighter Whom) override {}

		virtual void AddYuko(Fighter Whom) override {}
		virtual void RemoveYuko(Fighter Whom) override {}

		virtual void AddKoka(Fighter Whom) override {}
		virtual void RemoveKoka(Fighter Whom) override {}

		virtual void Hantei(Fighter Whom) override {}
		virtual void SetAsDraw(bool Enable = true) override {}

		virtual void AddShido(Fighter Whom) override {}
		virtual void RemoveShido(Fighter Whom) override {}

		virtual void AddHansokuMake(Fighter Whom, bool Direct = true) override {}
		virtual void RemoveHansokuMake(Fighter Whom) override {}

		virtual void AddMedicalExamination(Fighter Whom) override {}
		virtual void RemoveMedicalExamination(Fighter Whom) override {}

		virtual void Osaekomi(Fighter Whom) override {}
		virtual void Tokeda() override {}

		//Output
		virtual Match::Result GetResult() const { Match::Result ret; return ret; };

		//Serialization
		virtual ZED::CSV Scoreboard2String() const override { return ""; }
		virtual ZED::CSV Osaekomi2String(Fighter Who) const  override { return ""; }

	private:
		std::vector<OsaekomiEntry> m_OsaekomiList;
	};
}