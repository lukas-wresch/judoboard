#pragma once
#include <string>
#include <functional>
#include "judoboard.h"
#include "../ZED/include/blob.h"



namespace Judoboard
{
	class ITournament;
	class Tournament;


	//A *.md5 is a proprietary file format for storing tournament information

	class MD5
	{
	public:
		struct Weightclass;


		struct Association
		{
			int ID   = -1;
			int Tier = -1;//International starts at 2, everything below increases the tier level

			std::string Description;
			std::string ShortName;

			std::string Number;//"Number" of the association, can also include characters like '.'
			int NextAsscociationID = -1;//Association of the parent
			const Association* NextAsscociation = nullptr;

			bool Active = false;
		};

		struct Club
		{
			int ID = -1;

			int AssociationID = -1;
			const Association* Association = nullptr;

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

			int OfficialClubNo = 0;//Official no. of the club
			int StatusChanged  = -1;

			mutable void* pUserData = nullptr;
		};

		struct RelationClubAssociation//Relational table to connect clubs to associations
		{
			int ClubID = -1;
			int Tier   = -1;//International starts at 2, everything below increases the tier level
			int AssociationID = -1;
		};

		struct AgeGroup
		{
			int ID = -1;

			std::string Name;

			int MinBirthyear = -1;
			int MaxBirthyear = -1;

			Gender Gender = Gender::Male;

			int LotterySchemaID = 0;

			bool MoveUp = true;
			bool ChangeWeightAtScale = false;

			bool AllCategories = false;
			bool Kata = false;

			bool PoolSystem = false;
			bool AllParticipantsInResultTable = true;

			int Money = 0;
			int MoneyKata = 0;
			int MoneyAllCategories = 0;
			int MoneyIncreased = 0;
			int MoneyKataIncreased = 0;
			int MoneyAllCategoriesIncreased = 0;

			int Tolerance = 0;

			int Team = 0;

			mutable void* pUserData = nullptr;
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

			int Graduation = 0;//Kyu/Dan, 0 for unknown, 1=9. Kyu, 9=9. Kyu, 10=1. Dan, 21=12. Dan

			int  WeightclassID = -1;//Remember these IDs are NOT unique!
			const Weightclass* Weightclass = nullptr;
			bool HasBeenWeighted = false;

			int Birthyear = -1;//< 0 if no value is known

			int StartNo = -1;
			int QualificationRank = 99;//Rank for qualification (place), 9 for placed on 1
			//1 for place 1, 2 for place 2 (till place 8)
			//9 = seeded 1 (till 16 = seeded 8)
			//17 = nachruecker 1, (till 20 nachruecker 4)
			//99 = no place

			bool StatusChanged = false;

			std::string ClubFullname;
			std::string AssociationShortname;

			int AllCategoriesParticipantID = -1;
			int KataParticipantID = -1;
			int GKParticipantID   = -1;//Weightclass participant id???

			bool MoneyIncreased = false;
			int  WeightInGrams  = -1;

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

			std::string Description;
			int Status = 0;
			//0 = input phase
			//2 = lottery done
			//3 = schedule complete
			//4 = completed

			int FightSystemID = 16;
			//1  = double elimination (7 participants, 16 system)
			//13 = 1 participant
			//14 = 2 participants (best of 3?)
			//15 = round robin?
			//16 = round robin (5 participants)
			//19 = Single elimination (single consulation bracket, 16 system)
			//20 = Single elimination (single consulation bracket, 32 system)
			//24 = pooled (6 participants) 3+3 pool
			int FightSystemTypeID = 1;//Unknown field, always 1

			bool MatchForThirdPlace = false;
			bool MatchForFifthPlace = false;

			std::string Date;

			int Relay     = 1;
			int MaxJGJ    =  6;//Maximum participant for round robin system
			int MaxPooled = 10;//Maximum participant for pooled system

			std::string Identifier;
			std::string ForReference = "T";


			//Version 7
			bool BestOfThree = false;

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

			int MatchNo = 1;

			int StartNoRed = -1;
			int RedID = -1;
			int RedFromMatch = -1;
			int RedTyp = 0;//Unknown field (likely related to RedFromMatch)
			const Participant* Red = nullptr;

			int StartNoWhite = -1;
			int WhiteID = -1;
			int WhiteFromMatch = -1;
			int WhiteTyp = 0;//Unknown field (likely related to WhiteFromMatch)
			const Participant* White = nullptr;

			int WinnerID      = -1;
			int WinnerMatchNo = 0;
			int WinnerColor   = 0;

			int LoserID      = -1;
			int LoserMatchNo = 0;
			int LoserColor   = 0;

			int WaitingForWinnerFromMatch = 0;

			int Time   = -1;
			int Result = -1;//Could be: result available

			int ScoreWinner = -1;
			int ScoreLoser  = -1;

			int Status = 1;
			//0 = Not ready (unresolved dependencies) OR completely empty match (both fighters are empty)
			//1 = GO (ready), could also be invalid match (RedID == WhiteID)
			//2 = Half empty match (one of the fighters is empty)
			//3 = completed match
			//5 = completed match (is elimination match)
			//0, 2, 4, 5 = ???

			int RedOutMatchID   = -1;
			int WhiteOutMatchID = -1;

			int Pool = 0;//ID of the pool
			//1 for round robin (sometimes 0!?)
			//1-4 for pool matches in pool match table, 0 for final round

			int ThirdMatchNo = 0;//Unknown field
			int ThirdColor   = 0;//Unknown field
			int AreaID       = 1;//Unknown field
			//Typical 1
			//2 = first round elimination system
			//9 = final match elimination system
			//4 = third place match
		};

		struct Result
		{
			int AgeGroupID = -1;
			const AgeGroup* AgeGroup = nullptr;
			int WeightclassID = -1;
			const Weightclass* Weightclass = nullptr;

			int RankID = -1;
			int Pool   = 0;

			int RankNo  = -1;
			int MatchNo = -1;//Unknown field

			int RankType = 1;//Unknown field
			int ParticipantID = -1;
			const Participant* Participant = nullptr;

			int PointsPlus  = 0;//Number of won matches
			int PointsMinus = 0;//Number of lost matches

			int ScorePlus  = 0;//Sum of score
			int ScoreMinus = 0;//Sum of score of enemies

			bool Relay    = false;
			bool FromPool = false;
		};

		struct RankToPoints
		{
			int RankID = -1;
			int Points = -1;
		};


		MD5(const std::string& Filename);
		MD5(ZED::Blob&& Data) { Parse(std::move(Data)); }
		MD5(const Tournament& Tournament);
		~MD5();

		bool Save(const std::string& Filename) const;

		[[nodiscard]] Association*  FindAssociation(int AssociationID);
		[[nodiscard]] Association*  FindAssociation(int AssociationID) const;
		[[nodiscard]] Association*  FindAssociationByName(const std::string& Name) const;
		[[nodiscard]] Club*         FindClub(int ClubID);
		[[nodiscard]] Club*         FindClubByName(const std::string& Name) const;
		[[nodiscard]] Participant*  FindParticipant(int ParticipantID);
		[[nodiscard]] AgeGroup*     FindAgeGroup(int AgeGroupID);
		[[nodiscard]] Weightclass*  FindWeightclass(int AgeGroupID, int WeightclassID);
		[[nodiscard]] const Weightclass* FindWeightclass(const std::string& AgeGroup, const std::string& Weightclass) const;
		[[nodiscard]] const Result* FindResult(int AgeGroupID, int WeightclassID, int Rank) const;
		[[nodiscard]] const Result* FindResult(const std::string& AgeGroup, const std::string& Weightclass, int Rank) const;
		[[nodiscard]] std::vector<const Result*> FindResults(int AgeGroupID, int WeightclassID) const;
		[[nodiscard]] int FindStartNo(int AgeGroupID, int WeightclassID, int ParticipantID) const;
		[[nodiscard]] int FindLotOfAssociation(int AssociationID) const;

		void Dump() const;

		const std::vector<Association*>& GetAssociations()  const { return m_Associations; }
		const std::vector<Club*>&        GetClubs()		    const { return m_Clubs; }
		const std::vector<AgeGroup*>&    GetAgeGroups()     const { return m_AgeGroups; }
		const std::vector<Weightclass*>& GetWeightclasses() const { return m_Weightclasses; }
		const std::vector<Participant*>& GetParticipants()  const { return m_Participants; }
		const std::vector<Match>&		 GetMatches()	    const { return m_Matches; }
		const std::vector<Result>&       GetResults()	    const { return m_Results; }
		const std::vector<Lottery>&      GetLottery()       const { return m_Lottery; }

		int GetNumAssociations()  const { return m_NumAssociations; }//Returns the number of associations that should be in the file according to the header, not the actual number of associations read
		int GetNumClubs()		  const { return m_NumClubs; }//Returns the number of clubs that should be in the file according to the header, not the actual number of clubs read
		int GetNumParticipants()  const { return m_NumParticipants; }//Returns the number of participants that should be in the file according to the header, not the actual number of participants read

		std::string GetFileDate()      const { return m_FileDate; }
		std::string GetDateStart()     const { return m_DateStart; }
		std::string GetDateEnd()       const { return m_DateEnd; }
		std::string GetDescription()   const { return m_Description; }
		auto        GetLotteryTierID() const { return m_LotteryTierID; }

		const Association* GetOrganizer() const {
			return FindAssociation(m_AssociationID);
		}

		operator bool() const { return m_IsValid; }

	private:
		bool Parse(ZED::Blob&& Data);
		std::string ReadLine(ZED::Blob& Data, bool* pNewLine = nullptr);
		std::string RemoveControlCharacters(std::string& Str);//Removes all ASCII and Latin1 control characters
		int ReadInt(ZED::Blob& Data, bool* pNewLine = nullptr);

		std::vector<std::string> ReadHeader(ZED::Blob& Data, int& DataCount);

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
		bool ReadLog(ZED::Blob& Data);

		bool ReadTable(ZED::Blob& Data, std::function<bool(std::vector<std::string>&&, std::vector<std::string>&&)> Parse = nullptr);


		std::vector<Association*> m_Associations;
		std::vector<Club*>        m_Clubs;
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

		int m_SchemaID = 1;
		int m_LotteryTierID = -1;//Lottery is conducted on which tier?
		int m_AssociationID = -1;//Association that is conducting the tournament
		int m_AssociationLevelID = -1;//Tier of m_AssociationID + 1
		int m_TierToDisplay = -1;//Tier that should be displayed on lists
		int m_MAXJGJ = 5;//Maximum participants for round robin
		int m_LotteryProcess = 0;

		int m_NumOfRelays = 2;

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

		int m_Money = 0;
		int m_MoneyIncreased = 0;

		bool m_IsValid = false;
	};
}