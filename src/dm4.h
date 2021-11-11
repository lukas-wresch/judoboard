#pragma once
#include <string>



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
			int ID = -1;

			int ClubID = -1;//< 0 if no value is known
			Club* Club = nullptr;

			std::string Firstname;
			std::string Lastname;

			int Weight    = -1;//< 0 if no weight is known
			int Birthyear = -1;//< 0 if no value is known
		};


		DM4(const std::string& Filename);

		const std::vector<Club>& GetClubs() const { return m_Clubs; }
		const std::vector<Participant>& GetParticipants() const { return m_Participants; }

		std::string GetTournamentName()  const { return m_TournamentName; }
		std::string GetTournamentPlace() const { return m_TournamentPlace; }
		std::string GetTournamentDate()  const { return m_TournamentDate; }		
		std::string GetAgeGroup()        const { return m_AgeGroup; }

	private:
		const std::vector<Club> m_Clubs;
		const std::vector<Participant> m_Participants;

		std::string m_TournamentName;
		std::string m_TournamentPlace;
		std::string m_TournamentDate;
		std::string m_AgeGroup;
	};
}