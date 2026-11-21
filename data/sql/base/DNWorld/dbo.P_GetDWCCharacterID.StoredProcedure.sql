USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetDWCCharacterID]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-08
description : ???? DWCCharacterID? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103356 = ?? ??? DWC???? ?? ?? ????.
**/

CREATE PROCEDURE [dbo].[P_GetDWCCharacterID]
	  @intAccountID bigint --// ?? ID
	, @intDeleteWaitingTime int --// ?? ?? ??(?)..!!!????? ??? ???? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP 1 CharacterID
	FROM dbo.Characters WITH(NOLOCK, INDEX(IX_NNI_Characters_AccountID), FORCESEEK)
	WHERE AccountID = @intAccountID 
      AND AccountLevelCode = 110 
      AND DeleteFlag = 0
	OPTION(KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103356;
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
