USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListCharacterDetails]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-03-21
description : ??? ???? ?????. (???? ?? ???)
			- @intWorldID = NULL ? ???? ?? ??? ?? ??? ?? ?? (??? ??? ??)
result set : 
	CharacterID/bigint, CharacterIndex/tinyint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint, LastVillageMapID/int, LastDarkLairVillageMapID/int, LastMapID/int, DefaultBody/int, DefaultLeg/int, DefaultHand/int, DefaultFoot/int, HairColor/int, EyeColor/int, SkinColor/int, VillageFirstVisitFlag/bit, CheckSumBin/binary, DeleteFlag/bit, GuildID/int, Coin/bigint, WarehouseCoin/bigint, GuildName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListCharacterDetails]	  
	  @intAccountID int --// ??? ?? ID	
	, @intWorldID int --// ?? ID	
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
	/**_# dbo.Characters ????? ?????.*/

	SELECT CH.CharacterID, CH.CharacterIndex, CH.CharacterName, CHS.CharacterLevel, CHS.JobCode, CHS.LastVillageMapID, CHS.LastDarkLairVillageMapID, CHS.LastMapID
		, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot, CHS.HairColor, CHS.EyeColor, CHS.SkinColor, CH.VillageFirstVisitFlag, CHS.CheckSumBin, CH.DeleteFlag, ISNULL(GM.GuildID, 0) AS GuildID, CHS.Coin, CHS.WarehouseCoin
	    , G.GuildName
	FROM dbo.Characters CH WITH (NOLOCK, INDEX(IX_NNI_Characters_AccountID), FORCESEEK)
		INNER LOOP JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
		LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CH.CharacterID = GM.CharacterID
		LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
	WHERE CH.AccountID = @intAccountID
		AND CH.AccountLevelCode <> 110
		AND CH.WorldID = COALESCE(@intWorldID, CH.WorldID)
		AND CH.DeleteFlag = CAST(0 AS bit)
		AND NOT EXISTS (
			SELECT * 
			FROM dbo.HiddenCharacters WITH (NOLOCK, INDEX(PK_HiddenCharacters), FORCESEEK) 
			WHERE CharacterID = CH.CharacterID
		)

	/**_# ???? ???? ??? 103102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;		
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
