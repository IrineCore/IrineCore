USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetCharacterStatRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-12-17
description : ???? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000019 = ??? ??? ??? ???? ?? ???? ?????.
1030000020 = ??? ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetCharacterStatRanking]
	  @inbCharacterID bigint --// ???ID	
	, @inyCharacterClassCode tinyint = NULL --// ??? ??? ??
	, @inyCharacterAbilityCode tinyint = NULL --// ??? ?? ??	
	, @intRanking int = NULL OUTPUT --// ??
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
	SELECT @intRanking = CASE WHEN @inyCharacterClassCode IS NULL THEN AbilityRank ELSE ClassAbilityRank END 
	  FROM dbo.CharacterStatRanking WITH (NOLOCK)
	 WHERE CharacterID = @inbCharacterID 
       AND CharacterClassCode = COALESCE(@inyCharacterClassCode, CharacterClassCode)
       AND CharacterAbilityCode = COALESCE(@inyCharacterAbilityCode, CharacterAbilityCode);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000020;
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
