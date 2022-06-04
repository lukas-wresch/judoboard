#pragma once
#include <string>
#include "judoboard.h"
#include "../ZED/include/blob.h"



namespace Judoboard
{
	//A *.md5 is a proprietary file format for storing tournament information

	class MD5
	{
	public:
		struct Weightclass;


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

			int OfficialClubNo = -1;//Official no. of the club
			int StatusChanged  = -1;//Could also be a boolean
		};

		struct Association
		{
			int ID = -1;
			int TierID = -1;

			std::string Description;
			std::string ShortName;

			int Number = -1;
			int NextAsscociationID = -1;
			const Association* NextAsscociation = nullptr;

			bool Active = false;
		};

		struct RelationClubAssociation//Relational table to connect clubs to associations
		{
			int ClubID = -1;
			int TierID = -1;
			int AssociationID = -1;
		};

		struct AgeGroup
		{
			int ID = -1;

			std::string Name;

			int MinBirthyear = -1;
			int MaxBirthyear = -1;

			Gender Gender = Gender::Male;

			int LotterySchemaID = -1;

			bool MoveUp = false;
			bool ChangeWeightAtScale = false;

			bool AllCategories = false;
			bool Kata = false;

			bool PoolSystem = false;
			bool AllParticipantsInResultTable = false;

			int Money = -1;
			int MoneyKata = -1;
			int MoneyAllCategories = -1;
			int MoneyIncreased = -1;
			int MoneyKataIncreased = -1;
			int MoneyAllCategoriesIncreased = -1;

			int Tolerance = -1;

			std::string Team;//Format is unclear
		};

		struct Participant
		{
			int ID = -1;

			int AgeGroupID = -1;
			const AgeGroup* AgeGroup = nullptr;

			int ClubID = -1;
			const Club* Club = nullptr;

			std::string Firstname;
			std::string Lastname;

			int Graduation = -1;

			int  WeightclassID = -1;//Remeber these IDs are NOT unique!
			const Weightclass* Weightclass = nullptr;
			bool HasBeenWeighted = false;

			int Birthyear = -1;//< 0 if no value is known

			int StartNo = -1;
			int RankID  = -1;

			bool StatusChanged = false;

			std::string ClubFullname;
			std::string ClubShortname;

			int AllCategoriesParticipantID = -1;
			int KataParticipantID = -1;
			int GKParticipantID = -1;//GK = ???

			bool MoneyIncreased = false;
			int  WeightInGramm = -1;

			mutable void* pUserData = nullptr;
		};

		struct Weightclass
		{
			int ID = -1;//These are IDs are NOT unique!! Weightclasses for differen age groups have the SAME ID!!
			int AgeGroupID = -1;
			const AgeGroup* AgeGroup = nullptr;//Don't link like this, because this is messed up in md5 files!

			int WeightLargerThan  = -1;
			int WeightSmallerThan = -1;

			int WeightInGrammsLargerThan  = -1;
			int WeightInGrammsSmallerThan = -1;

			int MaxPooled = -1;

			std::string Description;
			int Status = -1;

			int FightSystemID = -1;
			int FightSystemTypeID = -1;

			bool MatchForThirdPlace = false;
			bool MatchForFifthPlace = false;

			std::string Date;

			int Relay = -1;
			int MaxJGJ = -1;

			std::string Identifier;
			std::string ForReference;

			mutable void* pUserData = nullptr;
		};

		struct Lottery
		{
			int AssociationID = -1;
			int StartNo = -1;
		};

		struct LotterySchema
		{
			int ID = -1;
			std::string Description;

			int AssociationID = -1;
			int TierID = -1;
		};

		struct LotterySchemaLine
		{
			int LotterySchemaID = -1;
			int PositionID = -1;

			int AssociationID = -1;
			int RankID = -1;
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
			const AgeGroup* AgeGroup = nullptr;

			int WeightclassID = -1;
			const Weightclass* Weightclass = nullptr;

			int MatchNo = -1;

			int StartNoRed = -1;
			int RedID = -1;
			int RedFromMatch = -1;
			int RedTyp = -1;
			const Participant* Red = nullptr;

			int StartNoWhite = -1;
			int WhiteID = -1;
			int WhiteFromMatch = -1;
			int WhiteTyp = -1;
			const Participant* White = nullptr;

			int WinnerID = -1;
			int WinnerMatchNo = -1;
			int WinnerColor = -1;

			int LoserID = -1;
			int LoserMatchNo = -1;
			int LoserColor = -1;

			int WaitingForWinnerFromMatch = -1;

			int Time   = -1;
			int Result = -1;

			int ScoreWinner = -1;
			int ScoreLoser  = -1;

			int Status = -1;

			int RedOutMatchID = -1;
			int WhiteOutMatchID = -1;

			int Pool = -1;

			int ThirdMatchNo = -1;
			int ThirdColor   = -1;
			int AreaID = -1;
		};

		struct Result
		{
			int AgeGroupID = -1;
			const AgeGroup* AgeGroup = nullptr;
			int WeightclassID = -1;
			const Weightclass* Weightclass = nullptr;

			int RankID = -1;
			int Pool = -1;

			int RankNo = -1;
			int MatchNo = -1;

			int RankType = -1;
			int ParticipantID = -1;
			Participant* Participant = nullptr;

			int PointsPlus  = -1;
			int PointsMinus = -1;

			int ScorePlus  = -1;
			int ScoreMinus = -1;

			bool Relay = false;
			bool FromPool = false;
		};

		struct RankToPoints
		{
			int Rank = -1;
			int Points = -1;
		};


		MD5(const std::string& Filename);
		MD5(ZED::Blob&& Data) { Parse(std::move(Data)); }
		~MD5();

		bool Save(const std::string& Filename) const;

		Association* FindAssociation(int AssociationID);
		Club*        FindClub(int ClubID);
		Participant* FindParticipant(int ParticipantID);
		AgeGroup*    FindAgeGroup(int AgeGroupID);
		Weightclass* FindWeightclass(int AgeGroupID, int WeightclassID);
		const Result* FindResult(int AgeGroupID, int WeightclassID, int Rank) const;
		const Result* FindResult(const std::string& AgeGroup, const std::string& Weightclass, int Rank) const;
		std::vector<const Result*> FindResults(int AgeGroupID, int WeightclassID) const;

		void Dump() const;

		const std::vector<Association*>& GetAssociations()  const { return m_Associations; }
		const std::vector<Club*>&        GetClubs()		    const { return m_Clubs; }
		const std::vector<Weightclass*>& GetWeightclasses() const { return m_Weightclasses; }
		const std::vector<Participant*>& GetParticipants()  const { return m_Participants; }
		const std::vector<Match>&		 GetMatches()	    const { return m_Matches; }
		const std::vector<Result>&       GetResults()	    const { return m_Results; }

		int GetNumAssociations()  const { return m_NumAssociations; }//Returns the number of associations that should be in the file according to the header, not the actual number of associations read
		int GetNumClubs()		  const { return m_NumClubs; }//Returns the number of clubs that should be in the file according to the header, not the actual number of clubs read
		int GetNumParticipants()  const { return m_NumParticipants; }//Returns the number of participants that should be in the file according to the header, not the actual number of participants read

		std::string GetFileDate()    const { return m_FileDate; }
		std::string GetDescription() const { return m_Description; }

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
		bool ReadLottery(ZED::Blob& Data);
		bool ReadLotteryScheme(ZED::Blob& Data);
		bool ReadLotterySchemaLine(ZED::Blob& Data);
		bool ReadParticipants(ZED::Blob& Data);
		bool ReadAssociation(ZED::Blob& Data);
		bool ReadClubs(ZED::Blob& Data);
		bool ReadRelationParticipantMatchTable(ZED::Blob& Data);
		bool ReadMatchData(ZED::Blob& Data);
		bool ReadResult(ZED::Blob& Data);


		std::vector<Association*> m_Associations;
		std::vector<Club*> m_Clubs;
		std::vector<Participant*> m_Participants;
		std::vector<AgeGroup*>    m_AgeGroups;
		std::vector<Weightclass*> m_Weightclasses;

		std::vector<RelationParticipantMatchTable> m_Relations;
		std::vector<RelationClubAssociation> m_ClubRelations;

		std::vector<Lottery> m_Lottery;
		std::vector<LotterySchema> m_LotterySchemas;
		std::vector<LotterySchemaLine> m_LotterySchemaLines;

		std::vector<Match>  m_Matches;
		std::vector<Result> m_Results;
		std::vector<RankToPoints> m_RankToPoints;

		//Meta data can be found at the start of a MD5 file
		std::string m_FileDate;//Date the file was saved
		std::string m_Description;

		int m_SchemaID = -1;
		int m_LotteryLevelID = -1;
		int m_AssociationID  = -1;
		int m_AssociationLevelID = -1;
		int m_LevelShortID = -1;
		int m_MAXJGJ = -1;
		int m_LotteryProcess = -1;

		int m_NumOfRelays = -1;

		std::string m_Place;
		std::string m_DateStart;
		std::string m_DateEnd;
		std::string m_SportAdministrator;

		bool m_ThirdPlaceMatch = false;
		bool m_FifthPlaceMatch = false;
		bool m_IgnoreNegativeScores = false;

		int m_NumClubs = -1;//Number of clubs in the md5 file
		int m_NumParticipants = -1;//Number of participants in the md5 file
		int m_NumAssociations = -1;//Number of associations in the md5 file

		int m_Money = -1;
		int m_MoneyIncreased = -1;

		bool m_IsValid = false;
	};
}