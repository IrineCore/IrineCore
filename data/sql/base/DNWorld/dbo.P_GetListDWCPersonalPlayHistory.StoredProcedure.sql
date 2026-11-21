USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDWCPersonalPlayHistory]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-15
description : DWC ?? ??? ?????.
result set : DWCTeamName/nvarchar(50), ResultCode/tinyint, GainDWCPoint/int, RegisterDate/datetime2(0)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103357 = ?? ???? DWC???? ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListDWCPersonalPlayHistory]
	  @inbCharacterID bigint = NULL --// ???ID
	, @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ?? ??
	, @nvcCharacterName nvarchar(50) = NULL --//??? ??
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
	IF @nvcCharacterName IS NOT NULL
		SELECT @inbCharacterID = CharacterID 
		FROM dbo.Characters WITH(NOLOCK, INDEX(IX_UNF_Characters_CharacterName), FORCESEEK) 
		WHERE CharacterName = @nvcCharacterName AND DeleteFlag = 0

	/**_# [DWCPersonalPlayHistory] ???? ?????.*/
	SELECT B.DWCTeamName, A.ResultCode, A.GainDWCPoint, A.RegisterDate
	FROM dbo.DWCPersonalPlayHistory A WITH(NOLOCK, INDEX(PK_DWCPersonalPlayHistory), FORCESEEK)
		INNER JOIN dbo.DWCTeams B WITH(NOLOCK, INDEX(PK_DWCTeams), FORCESEEK) ON A.DWCTeamID = B.DWCTeamID
	WHERE A.CharacterID = @inbCharacterID
	  AND A.RegisterDate BETWEEN @dt2StartDate AND @dt2EndDate
	OPTION(KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103357;
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
