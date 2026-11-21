USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_AddCharacterStatRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-12-17
description : ??? ?? ??? ?????. (??max, ??max, ??, ??, ????, ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_AddCharacterStatRanking]	 
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

	/**_# ?? ?? ???? TRUNCATE ?? ??? ?????? INSERT ???.*/
	TRUNCATE TABLE dbo.CharacterStatRanking;

	WITH CA AS (
		SELECT CA.CharacterID, CH.CharacterName, CH.CharacterClassCode, CS.CharacterLevel, CA.CharacterAbilityCode, ISNULL(CA.AbilityPoint, 0) AS AbilityPoint
		FROM dbo.CharacterAbilities CA WITH (NOLOCK)
			 INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON CA.CharacterID = CH.CharacterID
			 INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CS.CharacterID
		WHERE CA.CharacterAbilityCode IN (5,7,8,9,12,16) AND CH.DeleteFlag = 0 AND CH.AccountLevelCode < 99
	), R AS (
		SELECT CharacterID, CharacterName, CharacterClassCode, CharacterAbilityCode, AbilityPoint, CharacterLevel,
			   RANK() OVER(PARTITION BY CharacterAbilityCode  ORDER BY AbilityPoint DESC, CharacterLevel ASC, CharacterID DESC) AS AbilityRank,
			   RANK() OVER(PARTITION BY CharacterAbilityCode, CharacterClassCode  ORDER BY AbilityPoint DESC, CharacterLevel ASC, CharacterID DESC) AS ClassAbilityRank	   
		FROM CA
	)
	INSERT dbo.CharacterStatRanking (CharacterID, CharacterName, CharacterClassCode, CharacterAbilityCode, AbilityPoint, AbilityRank, ClassAbilityRank, CharacterLevel)
	SELECT CharacterID, CharacterName, CharacterClassCode, CharacterAbilityCode, AbilityPoint, AbilityRank, ClassAbilityRank, CharacterLevel	
	FROM R
	WHERE AbilityRank <= 1000 OR ClassAbilityRank <= 1000;

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
