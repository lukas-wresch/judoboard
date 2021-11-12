#pragma once
#include <string>
#include "../ZED/include/blob.h"



namespace Judoboard
{
	//A *.md5 is a proprietary file format for storing tournament information

	class MD5
	{
	public:
		struct Club
		{
			int ID = -1;
			std::string Name_ForSorting;
			std::string Name;

			std::string Representative_Firstname;
			std::string Representative_Lastname;
			std::string Representative_Street;
			std::string Representative_Place;
			std::string Representative_ZipCode;

			std::string Representative_TelPrivate;
			std::string Representative_TelProfessional;
			std::string Representative_TelMobil;
			std::string Representative_Email;
			std::string Representative_Fax;
			std::string Representative_ClubNo;//Official no. of the club
			int StatusChanged = -1;//Could also be a boolean
		};

		struct RelationClubAssociation//Relational table to connect clubs to associations
		{
			int ClubID = -1;
			int TierID = -1;
			int AssociationID = -1;
		};

		struct Participant
		{
			int ID = -1;

			int AgeGroupID = -1;
			int ClubID = -1;
			const Club* Club = nullptr;

			int Graduation = -1;

			int WeightclassID = -1;
			bool HasBeenWeighted = false;

			int Birthyear = -1;//< 0 if no value is known

			int StartNo = -1;
			int RankID  = -1;

			bool StatusChanged = false;

			std::string ClubFull;
			std::string ClubShort;

			int AllCategoriesParticipantID = -1;
			int KataParticipantID = -1;
			int GKParticipantID = -1;//GK = ???

			bool MoneyIncreased = false;
			int WeightInGramm = -1;
		};

		struct Lottery
		{
			int AssociationID = -1;
			int StartNo = -1;
		};

		struct RelationParticipantMatchTable
		{
			int AgeGroupID = -1;
			int WeightclassID = -1;
			int StartNo = -1;
			int ParticipantID = -1;
		};

		struct Match
		{
			int AgeGroupID = -1;
			int WeightclassID = -1;
			int MatchNo = -1;

			int StartNoRed = -1;
			int RedID = -1;
			int RedFromMatch = -1;
			int RedTyp = -1;

			int StartNoWhite = -1;
			int WhiteID = -1;
			int WhiteFromMatch = -1;
			int WhiteTyp = -1;

			int WinnerID = -1;
			int WinnerMatchNo = -1;
			int WinnerColor = -1;

			int LoserID = -1;
			int LoserMatchNo = -1;
			int LoserColor = -1;

			int WaitingForWinnerFromMatch = -1;

			int Time = -1;
			int Result = -1;

			int ScoreWinner = -1;
			int ScoreLoser = -1;

			int Status = -1;

			int RedOutMatchID = -1;
			int WhiteOutMatchID = -1;

			int Pool = -1;

			int ThirdMatchNo = -1;
			int ThirdColor = -1;
			int AreaID = -1;
		};


		MD5(const std::string& Filename);
		MD5(ZED::Blob& Data) { Parse(std::move(Data)); }
		//~MD5();

		const std::vector<Club>& GetClubs() const { return m_Clubs; }
		const std::vector<Participant>& GetParticipants() const { return m_Participants; }

		std::string GetFileDate()  const { return m_FileDate; }

		operator bool() const { return m_IsValid; }

	private:
		bool Parse(ZED::Blob&& Data);
		std::string ReadLine(ZED::Blob& Data, bool* pStartOfHeading = nullptr, bool* pNewLine = nullptr, bool* pDoubleZero = nullptr);
		std::string RemoveControlCharacters(std::string& Str);

		bool ReadTournamentData(ZED::Blob& Data);
		bool ReadRankScore(ZED::Blob& Data);
		bool ReadAgeGroups(ZED::Blob& Data);
		bool ReadWeightclasses(ZED::Blob& Data);
		bool ReadRelationClubAssociation(ZED::Blob& Data);
		bool ReadLotteryScheme(ZED::Blob& Data);
		bool ReadLotterySchemaLine(ZED::Blob& Data);
		bool ReadParticipants(ZED::Blob& Data);
		bool ReadAssociation(ZED::Blob& Data);
		bool ReadClubs(ZED::Blob& Data);


		std::vector<Club> m_Clubs;
		std::vector<Participant> m_Participants;

		//Meta data can be found at the start of a MD5 file
		std::string m_FileDate;
		std::string m_Description;
		int m_SchemaID = -1;
		std::string m_Place;

		bool m_IsValid = false;
	};
}