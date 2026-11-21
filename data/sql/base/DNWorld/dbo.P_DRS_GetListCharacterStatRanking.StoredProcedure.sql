USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListCharacterStatRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-12-17
description : ??? ?? ?? ??? ?????.
result set :
		Ranking/int, CharacterID/bigint, CharacterName/nvarchar, CharacterLevel/tinyint, CharacterClassCode/tinyint, AbilityPoint/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000019 = ??? ??? ??? ???? ?? ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListCharacterStatRanking]
	  @inbCharacterID bigint --// ???ID
	, @inyCharacterClassCode tinyint = NULL --// ??? ??? ?? (1=???, 2=??, 3=????, 4=???)
	, @inyCharacterAbilityCode tinyint = NULL --// ??? ?? ?? (5=??,7=??,8=??,9=??,12=????,16=??????)
    , @intCharacterRanking int = NULL OUTPUT --// ??? ??
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

	IF @inyCharacterAbilityCode IS NULL AND @inyCharacterClassCode IS NULL
	BEGIN
		SET @intReturnValue = 1030000019;
		GOTO ErrorHandler;
	END

	/**_# [CharacterStatRanking] ???? SELECT???.*/
	SELECT CASE WHEN @inyCharacterClassCode IS NULL THEN AbilityRank ELSE ClassAbilityRank END AS Ranking, 
		   CharacterID, CharacterName, CharacterLevel, CharacterClassCode, AbilityPoint
	  FROM dbo.CharacterStatRanking WITH (NOLOCK)
	 WHERE CharacterClassCode = COALESCE(@inyCharacterClassCode, CharacterClassCode) 
	   AND CharacterAbilityCode = COALESCE(@inyCharacterAbilityCode, CharacterAbilityCode)
	   AND CASE WHEN @inyCharacterClassCode IS NULL THEN AbilityRank ELSE ClassAbilityRank END <= 100
	 ORDER BY CASE WHEN @inyCharacterClassCode IS NULL THEN AbilityRank ELSE ClassAbilityRank END;

	SELECT @intCharacterRanking = ISNULL(CASE WHEN @inyCharacterClassCode IS NULL THEN AbilityRank ELSE ClassAbilityRank END, 0)
      FROM dbo.CharacterStatRanking WITH (NOLOCK)
	 WHERE CharacterID = @inbCharacterID 
	   AND CharacterClassCode = COALESCE(@inyCharacterClassCode, CharacterClassCode) 
	   AND CharacterAbilityCode = COALESCE(@inyCharacterAbilityCode, CharacterAbilityCode);

	IF @@ROWCOUNT = 0
		SET @intCharacterRanking = 0;

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
