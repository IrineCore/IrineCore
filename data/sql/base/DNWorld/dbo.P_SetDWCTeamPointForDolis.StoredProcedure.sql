USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SetDWCTeamPointForDolis]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-30
description : DWC ? ????? ??? ??????.(??? ?? ???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_SetDWCTeamPointForDolis]
	  @intDWCTeamID int = NULL --// DWC ?ID
	, @nvcDWCTeamName nvarchar(50) = NULL --// DWC ? ??
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
	IF @intDWCTeamID > 0
		UPDATE dbo.DWCTeams
		SET @intDWCPointBefore = DWCPoint
		  , Win = 0
		  , Lose = 0
		  , Draw = 0
		  , DWCPoint = 0
		  , TodayWin = 0
		  , TodayLose = 0
		  , TodayDraw = 0
		  , ConsecutiveWin = 0
		  , ConsecutiveLose = 0
		  , DWCHiddenPoint = 0
		WHERE DWCTeamID = @intDWCTeamID
	ELSE IF @nvcDWCTeamName > N''
		UPDATE dbo.DWCTeams
		SET @intDWCTeamID = DWCTeamID 
		  , @intDWCPointBefore = DWCPoint
		  ,	Win = 0
		  , Lose = 0
		  , Draw = 0
		  , DWCPoint = 0
		  , TodayWin = 0
		  , TodayLose = 0
		  , TodayDraw = 0
		  , ConsecutiveWin = 0
		  , ConsecutiveLose = 0
		  , DWCHiddenPoint = 0
		WHERE DWCTeamName = @nvcDWCTeamName AND CloseFlag = 0

	IF @@ROWCOUNT > 0
		INSERT dbo.DWCTeamPointPlayHistory(DWCTeamID, DWCPointChangeCode, DWCPointBefore, DWCPoint, RegisterDate)
		VALUES (@intDWCTeamID, 6, ISNULL(@intDWCPointBefore, 0), 0, GETDATE())

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
