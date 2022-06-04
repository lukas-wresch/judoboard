#pragma once
#include <string>
#include "judoboard.h"
#include "md5.h"
#include "../ZED/include/blob.h"



namespace Judoboard
{
	//A *.dmf is a "message" file from one (or multiple) clubs about participants of a tournament (Disk Melder Data File)

	class DMF
	{
	public:
		struct Club
		{
			std::string Name;
			std::string Kreis;
			std::string Bezirk;
			std::string Country;
		};

		struct Participant
		{
			Participant() = default;
			Participant(const MD5::Participant& Judoka)
			{
				Firstname = Judoka.Firstname;
				Lastname  = Judoka.Lastname;

				if (Judoka.WeightInGrams > 0)
					WeightInGrams = Judoka.WeightInGrams;
			}

			std::string Firstname;
			std::string Lastname;

			int WeightInGrams = -1;//< 0 if no weight is known
			int Birthyear = -1;//< 0 if no value is known
		};


		DMF(const std::string& Filename);
		DMF(ZED::Blob&& Data) { Parse(std::move(Data)); }

		const Club& GetClub() const { return m_Club; }
		const std::vector<Participant>& GetParticipants() const { return m_Participants; }

		std::string GetSenderName()      const { return m_Sender_Name; }
		std::string GetSenderStreet()    const { return m_Sender_Street; }
		std::string GetSenderPlace()     const { return m_Sender_Place; }
		std::string GetSenderTel()       const { return m_Sender_Tel; }

		std::string GetTournamentName()  const { return m_TournamentName; }
		std::string GetTournamentPlace() const { return m_TournamentPlace; }
		std::string GetTournamentDate()  const { return m_TournamentDate; }
		std::string GetAgeGroup()        const { return m_AgeGroup; }
		Gender      GetGender()          const { return m_Gender; }

		operator bool() const { return m_IsValid; }

	private:
		bool Parse(ZED::Blob&& Data);
		std::string ReadLine(ZED::Blob& Data);


		Club m_Club;
		std::vector<Participant> m_Participants;

		std::string m_Sender_Name;
		std::string m_Sender_Street;
		std::string m_Sender_Place;
		std::string m_Sender_Tel;

		std::string m_TournamentName;
		std::string m_TournamentDate;
		std::string m_TournamentPlace;

		std::string m_AgeGroup;
		Gender      m_Gender = Gender::Male;

		bool m_IsValid = false;
	};
}