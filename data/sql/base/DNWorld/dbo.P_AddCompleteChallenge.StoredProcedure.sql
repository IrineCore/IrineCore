USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCompleteChallenge]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-11-23
description : ??? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103352 = ?? ??? ???????.
**/

CREATE PROCEDURE [dbo].[P_AddCompleteChallenge]
	   @inbCharacterID bigint --// ???ID
	 , @inyChallengeID tinyint --// ????ID
	 , @intWeekday int --// ??	(?=1,?=2,?=3,?=4,?=5,?=6,?=7)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# [CompleteChallenges] ???? INSERT???.*/
	INSERT dbo.CompleteChallenges (CharacterID, ChallengeID, [Weekday], CompleteDate)
	VALUES (@inbCharacterID, @inyChallengeID, @intWeekday, @dt2Now);
	
	/**_# [CharacterStatus] ???? Update???.*/
	UPDATE dbo.CharacterStatus
	   SET LastStampDate = @dt2Now
	 WHERE CharacterID = @inbCharacterID;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_%'
		SET @intReturnValue = 103352;
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
