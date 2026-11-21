USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPlayAlteia]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-10-22
description : ???? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddPlayAlteia]
	  @inbCharacterID bigint --// ???ID
	, @intAcquireGoldKeyCount int --// ?????? ??
	, @intPlayTime int --// ?????
	, @intGuildID int = NULL --// ??ID 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2Now datetime2(0) = GETDATE();
DECLARE @dt2Today datetime2(0) = CAST(GETDATE() AS date);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [PlayAlteia] ???? MERGE???.*/
	MERGE dbo.PlayAlteia AS T
	USING (SELECT @inbCharacterID AS CharacterID 
			   , @intAcquireGoldKeyCount AS AcquireGoldKeyCount
			   , @intPlayTime AS PlayTime)S ON T.CharacterID = S.CharacterID
	WHEN MATCHED THEN
		UPDATE SET T.AcquireGoldKeyCount = CASE WHEN T.AcquireGoldKeyCount >= S.AcquireGoldKeyCount THEN T.AcquireGoldKeyCount ELSE S.AcquireGoldKeyCount END
				 , T.GoldKeyRegisterDate = CASE WHEN T.AcquireGoldKeyCount >= S.AcquireGoldKeyCount THEN T.GoldKeyRegisterDate ELSE @dt2Now END
				 , T.PlayTime = CASE WHEN T.PlayTime <= S.PlayTime THEN T.PlayTime ELSE S.PlayTime END
				 , T.PlayTimeRegisterDate = CASE WHEN T.PlayTime <= S.PlayTime THEN T.PlayTimeRegisterDate ELSE @dt2Now END
				 , T.DailyPlayCount = T.DailyPlayCount + 1
				 , T.WeeklyPlayCount = T.WeeklyPlayCount + 1
				 , T.LastPlayingDate = @dt2Now
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (CharacterID, AcquireGoldKeyCount, PlayTime, GoldKeyRegisterDate, PlayTimeRegisterDate, DailyPlayCount, WeeklyPlayCount, LastPlayingDate)
		VALUES (S.CharacterID, S.AcquireGoldKeyCount, S.PlayTime, @dt2Now, @dt2Now, 1, 1, @dt2Now);

	/**_# ?? ??? ?? ??? ?? AlteiaGuildRank MERGE???.*/
	IF @intGuildID IS NOT NULL AND EXISTS(SELECT * FROM dbo.GuildMembers WITH(NOLOCK) WHERE CharacterID = @inbCharacterID)
	BEGIN
		MERGE dbo.AlteiaGuildRank AS T
		USING (SELECT @intGuildID AS GuildID
				    , @intAcquireGoldKeyCount AS AcquireGoldKeyCount) S ON T.GuildID = S.GuildID
		WHEN MATCHED THEN
			UPDATE SET T.AcquireGoldKeyCount += S.AcquireGoldKeyCount
					 , T.UpdateDate = @dt2Now
		WHEN NOT MATCHED BY TARGET THEN
			INSERT (GuildID, AcquireGoldKeyCount, UpdateDate)
			VALUES (S.GuildID, S.AcquireGoldKeyCount, @dt2Now);
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
