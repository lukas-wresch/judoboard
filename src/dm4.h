#pragma once
#include <string>
#include "judoboard.h"
#include "md5.h"
#include "../ZED/include/blob.h"



namespace Judoboard
{
	//A *.dm4 is a "message" file from one (or multiple) clubs about participants of a tournament

	class DM4
	{
	public:
		struct Club
		{
			int ID = -1;
			std::string Name;
			std::string Representative_Firstname;
			std::string Representative_Lastname;
		};

		struct Participant
		{
			Participant() = default;
			Participant(const MD5::Participant& Judoka)
			{
				Firstname = Judoka.Firstname;
				Lastname  = Judoka.Lastname;

				if (Judoka.AgeGroup)
					Gender = Judoka.AgeGroup->Gender;

				if (Judoka.WeightInGrams > 0)
					WeightInGrams = Judoka.WeightInGrams;
			}

			int ID = -1;

			int ClubID = -1;//< 0 if no value is known
			const Club* Club = nullptr;

			std::string Firstname;
			std::string Lastname;

			Gender Gender;

			int WeightInGrams = -1;//< 0 if no weight is known
			int Birthyear = -1;//< 0 if no value is known
		};


		DM4(const std::string& Filename);
		DM4(ZED::Blob&& Data) { Parse(std::move(Data)); }
		~DM4();

		const Club* FindClubByID(int ClubID) const;

		const std::vector<Club*>& GetClubs() const { return m_Clubs; }
		const std::vector<Participant>& GetParticipants() const { return m_Participants; }

		std::string GetSenderClubName()  const { return m_Sender_ClubName; }
		std::string GetSenderName()      const { return m_Sender_Name; }

		std::string GetTournamentName()  const { return m_TournamentName; }
		std::string GetTournamentPlace() const { return m_TournamentPlace; }
		std::string GetTournamentDate()  const { return m_TournamentDate; }
		std::string GetAgeGroup()        const { return m_AgeGroup; }
		Gender      GetGender()          const { return m_Gender; }

		operator bool() const { return m_IsValid; }

	private:
		bool Parse(ZED::Blob&& Data);

		bool ParseLine(const std::string& Line);//Parse a single line of the dm4 file
		bool ParseStartOfChunk(const std::string& Line);//Parses the start of a new chunk, i.e. [Identifikation] or [Absender]. Returns false if Line is not the start of a new chunk
		bool GetValue(const std::string& Line, const std::string& Key, std::string& Result) const;
		std::string RemoveCharFromString(std::string& Str, char CharacterToRemove) const;

		//Flags used during parsing
		enum class Chunk
		{
			Unknow = 0,
			Identification,
			Sender,
			Report,
			Clubs,
			Participants
		} m_CurrentChunk = Chunk::Unknow;


		std::vector<Club*> m_Clubs;
		std::vector<Participant> m_Participants;

		std::string m_Sender_ClubName;
		std::string m_Sender_Name;

		std::string m_TournamentName;
		std::string m_TournamentDate;
		std::string m_TournamentPlace;
		std::string m_AgeGroup;
		Gender      m_Gender = Gender::Male;

		bool m_IsValid = false;
	};
}