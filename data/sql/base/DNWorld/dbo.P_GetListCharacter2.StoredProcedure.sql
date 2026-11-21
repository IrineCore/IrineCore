USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacter2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 21
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ??? ??? ?????.
result set : 
	version = 10 : CharacterID/bigint, CharacterIndex/tinyint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint, LastVillageMapID/int, LastDarkLairVillageMapID/int, LastMapID/int, DefaultBody/int, DefaultLeg/int, DefaultHand/int, DefaultFoot/int, HairColor/int, EyeColor/int, SkinColor/int, VillageFirstVisitFlag/bit, CheckSumBin/binary, DeleteFlag/bit, DeleteDueDate/datetime2
	version = 11 : CharacterID/bigint, CharacterIndex/tinyint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint, LastVillageMapID/int, LastDarkLairVillageMapID/int, LastMapID/int, DefaultBody/int, DefaultLeg/int, DefaultHand/int, DefaultFoot/int, HairColor/int, EyeColor/int, SkinColor/int, VillageFirstVisitFlag/bit, CheckSumBin/binary, DeleteFlag/bit, DeleteDueDate/datetime2, LastLoginDate/datetime2, CreateDate/datetime2
	version = 12 : CharacterID/bigint, CharacterIndex/tinyint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint, LastVillageMapID/int, LastDarkLairVillageMapID/int, LastMapID/int, DefaultBody/int, DefaultLeg/int, DefaultHand/int, DefaultFoot/int, HairColor/int, EyeColor/int, SkinColor/int, VillageFirstVisitFlag/bit, CheckSumBin/binary, DeleteFlag/bit, DeleteDueDate/datetime2, AccountLevelCode/tinyint
	version = 13 : CharacterID/bigint, CharacterIndex/tinyint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint, LastVillageMapID/int, LastDarkLairVillageMapID/int, LastMapID/int, DefaultBody/int, DefaultLeg/int, DefaultHand/int, DefaultFoot/int, HairColor/int, EyeColor/int, SkinColor/int, VillageFirstVisitFlag/bit, CheckSumBin/binary, DeleteFlag/bit, DeleteDueDate/datetime2, LastLoginDate/datetime2, CreateDate/datetime2, AccountLevelCode/tinyint
return value : 
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
5 = ??? ???? ?????? ?? ???? wake? ? ????? ?? by ??? at 2011-10-21
6 = Hint ???? Plan? ?? by ??? at 2011-11-07
8 = DormantCharacterStatus ??? ?? ?? by ??? at 2011-11-30
9 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
10 = ?? ??? ??? ?? ?? by ??? at 2012-07-18
14 = ??? ???? ???? ??? ?? by ??? at 2013-01-05
15 =  14 ?? ?? ?? by ??? at 2013-01-05
16 = AccountLevelCode ?? ? ?? by ??? at 2013-01-08
18 = 16? ?? ??  by ??? at 2013-01-11 
**/

CREATE PROCEDURE [dbo].[P_GetListCharacter2]
	  @intAccountID int --// ??? ?? ID
	, @intDeleteWaitingTime int --// ??? ?? ?? ??. ?? : minute
	, @intWorldID int = NULL --// ?? ID
	, @intVersion int = 10 --// ??? SP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @i int = 1
	, @j int
	, @inbCharacterID bigint
	, @dt2Now datetime2(0) = GETDATE()
	, @dt2EventFatigueGainDate datetime2(0)
	, @insEventFatigue smallint;

DECLARE @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dt2Now);
DECLARE @tblCharacters table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	CharacterID bigint, 
	CharacterIndex tinyint, 
	CharacterName nvarchar(30), 
	CharacterLevel tinyint, 
	JobCode tinyint, 
	LastVillageMapID int, 
	LastDarkLairVillageMapID int, 
	LastMapID int, 
	DefaultBody int, 
	DefaultLeg int, 
	DefaultHand int, 
	DefaultFoot int, 
	HairColor int, 
	EyeColor int, 
	SkinColor int, 
	VillageFirstVisitFlag bit, 
	CheckSumBin binary(4), 
	DeleteFlag bit, 
	DeleteDueDate datetime2(0),
	LastLoginDate datetime2(0),
	CreateDate datetime2(0),
	AccountLevelCode tinyint
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ??? ??? ?????.*/
	INSERT @tblCharacters (CharacterID, CharacterIndex, CharacterName, CharacterLevel, JobCode, LastVillageMapID, LastDarkLairVillageMapID, LastMapID, DefaultBody, DefaultLeg, DefaultHand, DefaultFoot, HairColor, EyeColor, SkinColor, VillageFirstVisitFlag, CheckSumBin, DeleteFlag, DeleteDueDate, LastLoginDate, CreateDate, AccountLevelCode)
	SELECT CH.CharacterID, CH.CharacterIndex, CH.CharacterName, CHS.CharacterLevel, CHS.JobCode, CHS.LastVillageMapID, CHS.LastDarkLairVillageMapID, CHS.LastMapID
		, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot, CHS.HairColor, CHS.EyeColor, CHS.SkinColor, CH.VillageFirstVisitFlag, CHS.CheckSumBin, CH.DeleteFlag
		, CASE WHEN (CH.DeleteFlag = CAST(1 AS bit)
				AND CH.DeleteByAdminFlag = CAST(0 AS bit)
				AND CH.DeleteDate > DATEADD(MINUTE, -@intDeleteWaitingTime, @dt2Now)
				AND CHS.CharacterLevel >= 10) THEN DATEADD(MINUTE, @intDeleteWaitingTime, CH.DeleteDate) ELSE NULL END AS DeleteDueDate
		, CHS.LastLoginDate, CH.CreateDate, CH.AccountLevelCode
	FROM dbo.Characters CH WITH (NOLOCK, INDEX(IX_NNI_Characters_AccountID), FORCESEEK)
		INNER LOOP JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
	WHERE CH.AccountID = @intAccountID
		AND CH.WorldID = COALESCE(@intWorldID, CH.WorldID)
		AND ( 
			CH.DeleteFlag = CAST(0 AS bit) OR
			( 
				CH.DeleteFlag = CAST(1 AS bit)
				AND CH.DeleteByAdminFlag = CAST(0 AS bit)
				AND CH.DeleteDate > DATEADD(MINUTE, -@intDeleteWaitingTime, @dt2Now)
				AND CHS.CharacterLevel >= 10
			)
		)
		AND NOT EXISTS (
			SELECT * 
			FROM dbo.HiddenCharacters WITH (NOLOCK, INDEX(PK_HiddenCharacters), FORCESEEK) 
			WHERE CharacterID = CH.CharacterID
		)	
	OPTION (KEEPFIXED PLAN);

	IF @intVersion = 10
		SELECT CharacterID, CharacterIndex, CharacterName, CharacterLevel, JobCode, LastVillageMapID, LastDarkLairVillageMapID, LastMapID, DefaultBody, DefaultLeg, DefaultHand, DefaultFoot, HairColor, EyeColor, SkinColor, VillageFirstVisitFlag, CheckSumBin, DeleteFlag, DeleteDueDate
		FROM @tblCharacters;

	ELSE IF @intVersion = 11
		SELECT CharacterID, CharacterIndex, CharacterName, CharacterLevel, JobCode, LastVillageMapID, LastDarkLairVillageMapID, LastMapID, DefaultBody, DefaultLeg, DefaultHand, DefaultFoot, HairColor, EyeColor, SkinColor, VillageFirstVisitFlag, CheckSumBin, DeleteFlag, DeleteDueDate, LastLoginDate, CreateDate
		FROM @tblCharacters;
	
	ELSE IF @intVersion = 12
		SELECT CharacterID, CharacterIndex, CharacterName, CharacterLevel, JobCode, LastVillageMapID, LastDarkLairVillageMapID, LastMapID, DefaultBody, DefaultLeg, DefaultHand, DefaultFoot, HairColor, EyeColor, SkinColor, VillageFirstVisitFlag, CheckSumBin, DeleteFlag, DeleteDueDate, AccountLevelCode
		FROM @tblCharacters;

	ELSE IF @intVersion = 13
		SELECT CharacterID, CharacterIndex, CharacterName, CharacterLevel, JobCode, LastVillageMapID, LastDarkLairVillageMapID, LastMapID, DefaultBody, DefaultLeg, DefaultHand, DefaultFoot, HairColor, EyeColor, SkinColor, VillageFirstVisitFlag, CheckSumBin, DeleteFlag, DeleteDueDate, LastLoginDate, CreateDate, AccountLevelCode
		FROM @tblCharacters;

--nexonz begin
	ELSE IF @intVersion = 14
		SELECT CharacterID, CharacterIndex, CharacterName, CharacterLevel, JobCode, LastVillageMapID, LastDarkLairVillageMapID, LastMapID, DefaultBody, DefaultLeg, DefaultHand, DefaultFoot, HairColor, EyeColor, SkinColor, VillageFirstVisitFlag, CheckSumBin, DeleteFlag, DeleteDueDate, LastLoginDate, CreateDate, AccountLevelCode
		FROM @tblCharacters;
--nexonz end
	SET @j = @@ROWCOUNT;

	/**_# ???? ???? ??? 103102? ?????.*/
	IF @j = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END
	/**_# ???? ??? ??? ? ??? ???? ??? ?????.*/
	WHILE @i <= @j
	BEGIN
		SELECT @inbCharacterID = CharacterID FROM @tblCharacters WHERE Seq = @i;

		EXEC dbo.P_ExpireMaterializedItem2 @inbCharacterID = @inbCharacterID;

		EXEC dbo.P_ExpireEffectItem @inbCharacterID = @inbCharacterID;

		SET @i += 1;
	END
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;
IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
