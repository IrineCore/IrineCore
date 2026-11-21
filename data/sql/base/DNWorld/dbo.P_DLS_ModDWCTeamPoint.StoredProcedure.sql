USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_ModDWCTeamPoint]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-02-22
description : DWC ? ????? ?? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103361 = ???? ?? DWC????.
**/

CREATE PROCEDURE [dbo].[P_DLS_ModDWCTeamPoint]
	  @intDWCTeamID int = NULL --// DWC ?ID
	, @nvcDWCTeamName nvarchar(50) = NULL --// DWC ? ??
	, @intDWCPoint int --// DWC??? (+/- ??? ??? ?)
	, @intAfterDWCPoint int OUTPUT --// ??? DWC ??? ? 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @intDWCPointBefore int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @intDWCTeamID IS NULL OR @intDWCTeamID < 0
		SELECT @intDWCTeamID = DWCTeamID FROM dbo.DWCTeams WHERE DWCTeamName = @nvcDWCTeamName AND CloseFlag = 0

	UPDATE dbo.DWCTeams
	SET @intDWCPointBefore = DWCPoint
	  , @intAfterDWCPoint = DWCPoint += @intDWCPoint
	WHERE DWCTeamID = @intDWCTeamID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103361;
		GOTO ErrorHandler;
	END

	INSERT dbo.DWCTeamPointPlayHistory(DWCTeamID, DWCPointChangeCode, DWCPointBefore, DWCPoint, RegisterDate)
	VALUES (@intDWCTeamID, 5, @intDWCPointBefore, @intAfterDWCPoint, GETDATE())
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
