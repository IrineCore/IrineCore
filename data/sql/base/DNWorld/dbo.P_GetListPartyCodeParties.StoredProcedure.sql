USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPartyCodeParties]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-05-02
description : PartyCode?? ??? ?????.
result set : version1 : PartyID/bigint, ServerID/int, RoomID/int, ServerType/tinyint, ServerCommonID/int, PartyCode/tinyint, PartyName/nvarchar, PartyMaxCount/smallint, GoalMapID/int, DifficultyCode/tinyint, PartyOptionFlagBitmap/int, PassWordValue/int, MinLevel/tinyint, DefaultDistributionCode/tinyint, RandomDistributionCode/tinyint, UpkeepCount/int, PartyOwnerCharacterID/bigint, PartyMemberCount/int
		     version2 : PartyID/bigint, ServerID/int, RoomID/int, ServerType/tinyint, ServerCommonID/int, PartyCode/tinyint, PartyName/nvarchar, PartyMaxCount/smallint, GoalMapID/int, DifficultyCode/tinyint, PartyOptionFlagBitmap/int, PassWordValue/int, MinLevel/tinyint, DefaultDistributionCode/tinyint, RandomDistributionCode/tinyint, UpkeepCount/int, PartyOwnerCharacterID/bigint, PartyMemberCount/int, CharacterTypeCode/tinyint
   		     version3 : PartyID/bigint, ServerID/int, RoomID/int, ServerType/tinyint, ServerCommonID/int, PartyCode/tinyint, PartyName/nvarchar, PartyMaxCount/smallint, GoalMapID/int, DifficultyCode/tinyint, PartyOptionFlagBitmap/int, PassWordValue/int, MinLevel/tinyint, DefaultDistributionCode/tinyint, RandomDistributionCode/tinyint, UpkeepCount/int, PartyOwnerCharacterID/bigint, PartyMemberCount/int, CharacterTypeCode/tinyint, DNGirlFlag/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = ????? ???? ?? ??? ResultSet?? ?? by ??? at 2012-05-11
4 = ????? ???? ??? ? ??? ?? by ??? at 2012-05-15
5 = ??? ?? by ??? at 2012-08-24
7 = ??? ?? ?? by ??? at 2013-05-13
9 = ??????? ???????? ?? by ??? at 2013-07-01
**/

CREATE PROCEDURE [dbo].[P_GetListPartyCodeParties]
	  @inyPartyCode tinyint --//?? ??
	, @intWorldID int --//??ID
	, @intVersion int = 1 --// ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF @intVersion = 1
		/**_# [Parties] ???? Select???.*/
		SELECT P.PartyID, P.ServerID, P.RoomID, P.ServerType, P.ServerCommonID, P.PartyCode, P.PartyName, P.PartyMaxCount, P.GoalMapID, P.DifficultyCode, P.PartyOptionFlagBitmap, P.PassWordValue, P.MinLevel, P.DefaultDistributionCode, P.RandomDistributionCode, P.UpkeepCount, ISNULL(PJ.CharacterID, 0) AS PartyOwnerCharacterID, ISNULL(PMC.PartyMemberCount, 0) AS PartyMemberCount
		FROM dbo.Parties P WITH(NOLOCK, INDEX(PK_Parties)) 
			LEFT OUTER JOIN dbo.PartyJoins PJ WITH(NOLOCK) ON P.PartyID = PJ.PartyID AND PJ.PartyOwnerFlag = 1
			LEFT OUTER JOIN dbo.VIX_PartyMemberCount PMC WITH (NOLOCK) ON P.PartyID = PMC.PartyID
		WHERE PartyCode = @inyPartyCode 
		  AND WorldID = @intWorldID;

	ELSE IF @intVersion = 2
		/**_# [Parties] ???? Select???.*/
		SELECT P.PartyID, P.ServerID, P.RoomID, P.ServerType, P.ServerCommonID, P.PartyCode, P.PartyName, P.PartyMaxCount, P.GoalMapID, P.DifficultyCode, P.PartyOptionFlagBitmap, P.PassWordValue, P.MinLevel, P.DefaultDistributionCode, P.RandomDistributionCode, P.UpkeepCount, ISNULL(PJ.CharacterID, 0) AS PartyOwnerCharacterID, ISNULL(PMC.PartyMemberCount, 0) AS PartyMemberCount, PMC.CharacterTypeCode
		FROM dbo.Parties P WITH(NOLOCK, INDEX(PK_Parties)) 
			LEFT OUTER JOIN dbo.PartyJoins PJ WITH(NOLOCK) ON P.PartyID = PJ.PartyID AND PJ.PartyOwnerFlag = 1
			LEFT OUTER JOIN dbo.VIX_PartyMemberCount PMC WITH (NOLOCK) ON P.PartyID = PMC.PartyID
		WHERE PartyCode = @inyPartyCode 
		  AND WorldID = @intWorldID;
		  
	ELSE IF @intVersion = 3
		/**_# [Parties] ???? Select???.*/
		SELECT P.PartyID, P.ServerID, P.RoomID, P.ServerType, P.ServerCommonID, P.PartyCode, P.PartyName, P.PartyMaxCount, P.GoalMapID, P.DifficultyCode, P.PartyOptionFlagBitmap, P.PassWordValue, P.MinLevel, P.DefaultDistributionCode, P.RandomDistributionCode, P.UpkeepCount, ISNULL(PJ.CharacterID, 0) AS PartyOwnerCharacterID, ISNULL(PMC.PartyMemberCount, 0) AS PartyMemberCount, PMC.CharacterTypeCode, CASE WHEN PGC.PartyDNGirlCount > 0 THEN CAST (1 AS bit) ELSE CAST (0 AS bit) END AS DNGirlFlag
		FROM dbo.Parties P WITH(NOLOCK, INDEX(PK_Parties)) 
			LEFT OUTER JOIN dbo.PartyJoins PJ WITH(NOLOCK) ON P.PartyID = PJ.PartyID AND PJ.PartyOwnerFlag = 1
			LEFT OUTER JOIN dbo.VIX_PartyDNGirlCount PGC WITH(NOLOCK) ON P.PartyID = PGC.PartyID
			LEFT OUTER JOIN dbo.VIX_PartyMemberCount PMC WITH (NOLOCK) ON P.PartyID = PMC.PartyID
		WHERE PartyCode = @inyPartyCode 
		  AND P.WorldID = @intWorldID;

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
