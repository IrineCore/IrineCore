USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetUnifiedLevel]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-08-14
description : ??? ?? ?? ????? ??(????)? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = DWC ??? ?? ????? ?? at 2013-02-18 by ???
**/

CREATE PROCEDURE [dbo].[P_GetUnifiedLevel]
	  @intAccountID int --// ?? ID
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
	SELECT SUM(CAST(CharacterLevel AS int)) AS UnifiedLevel
	FROM dbo.Characters A WITH(NOLOCK, INDEX(IX_NNI_Characters_AccountID), FORCESEEK) 
		INNER JOIN dbo.CharacterStatus B WITH(NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK)  ON A.CharacterID = B.CharacterID 
	WHERE A.AccountID = @intAccountID AND A.DeleteFlag = 0 AND A.AccountLevelCode <> 110
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

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
