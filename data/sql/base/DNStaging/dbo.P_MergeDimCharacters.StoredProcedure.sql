USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_MergeDimCharacters]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-20
description : DimCharacters ???? MERGE???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = TempCharacters2? Buffer Table? ??? ?? INSERT ??? by ??? at (2011-11-07)
3 = ???? ??? ?? ?? 1? ??? ??? by ??? at (2011-11-07)
4 = AccountLevelCode ?? ?? by ??? at (2012-07-13)
5 = PCRoomRebirthCoin, RebirthCoin ?? ?? by ??? at (2013-08-22)
**/

CREATE PROCEDURE [dbo].[P_MergeDimCharacters]
	  @dtmBasisDate datetime --// ?? ??
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
	MERGE dbo.DimCharacters AS T
	USING (
		SELECT CharacterID, AccountID, CharacterName, WorldID, CharacterClassCode, CharacterIndex, VillageFirstVisitFlag, SkillResetFlag, CharacterLevel, CharacterExp, MissionScore, JobCode
			, Coin, WarehouseCoin, SkillPoint, LastLoginDate, CreateDate, DeleteFlag, DeleteDate, DeleteCharacterName, DeletePrivateIP, DeletePublicIP, AccountLevelCode, PCRoomRebirthCoin, RebirthCoin
		FROM dbo.V_DimCharacters
		WHERE CreateDate < (@dtmBasisDate + 1)
	) AS S
	ON T.CharacterID = S.CharacterID
	WHEN MATCHED THEN
		UPDATE SET CharacterName = S.CharacterName
			, VillageFirstVisitFlag = S.VillageFirstVisitFlag
			, SkillResetFlag = S.SkillResetFlag
			, CharacterLevel = S.CharacterLevel
			, CharacterExp = S.CharacterExp
			, MissionScore = S.MissionScore
			, JobCode = S.JobCode
			, Coin = S.Coin
			, WarehouseCoin = S.WarehouseCoin
			, SkillPoint = S.SkillPoint
			, LastLoginDate = S.LastLoginDate
			, DeleteFlag = S.DeleteFlag
			, DeleteDate = S.DeleteDate
			, DeleteCharacterName = S.DeleteCharacterName
			, DeletePrivateIP = S.DeletePrivateIP
			, DeletePublicIP = S.DeletePublicIP
			, AccountLevelCode = S.AccountLevelCode
			, PCRoomRebirthCoin = S.PCRoomRebirthCoin
			, RebirthCoin = S.RebirthCoin
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (CharacterID, AccountID, CharacterName, WorldID, CharacterClassCode, CharacterIndex, VillageFirstVisitFlag, SkillResetFlag, CharacterLevel, CharacterExp, MissionScore, JobCode, Coin, WarehouseCoin
			, SkillPoint, LastLoginDate, CreateDate, DeleteFlag, DeleteDate, DeleteCharacterName, DeletePrivateIP, DeletePublicIP, AccountLevelCode, PCRoomRebirthCoin, RebirthCoin)
		VALUES (S.CharacterID, S.AccountID, S.CharacterName, S.WorldID, S.CharacterClassCode, S.CharacterIndex, S.VillageFirstVisitFlag, S.SkillResetFlag, S.CharacterLevel, S.CharacterExp, S.MissionScore, S.JobCode, S.Coin, S.WarehouseCoin
			, S.SkillPoint, S.LastLoginDate, S.CreateDate, S.DeleteFlag, S.DeleteDate, S.DeleteCharacterName, S.DeletePrivateIP, S.DeletePublicIP, S.AccountLevelCode, S.PCRoomRebirthCoin, S.RebirthCoin);

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
