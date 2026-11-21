USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPVPLadderScoreByJob]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-02-07
description : 1:1 ???? ?? ???? ??? ?? ??? ?????.
result set : JobCode/tinyint, Win/int, Lose/int, Draw/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ??? ??? ??? ???? ?? at 2011-02-11
**/

CREATE PROCEDURE [dbo].[P_GetListPVPLadderScoreByJob]
	  @inbCharacterID bigint --// ??? ID
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
	SELECT JobCode, Win, Lose, Draw
	FROM dbo.PVPLadderScoresByJob WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID
	ORDER BY JobCode;
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
