USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetListDWCTeamPointPlayHistory]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-03-08
description : DWC ? ??? ?? ??? ?????.
result set : vsDWCTeamID/int, vsDWCTeamName/nvarchar(50), DWCPointChangeCode/tinyint, DWCPointBefore/int, DWCPoint/int, RegisterDate/datetime2(0) 
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_GetListDWCTeamPointPlayHistory]
	  @intDWCTeamID int = 0 --// DWC ?ID
	, @nvcDWCTeamName nvarchar(50) = NULL--// DWC ???
	, @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ?? ??
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
	/**_# [DWCTeamPointPlayHistory] ?????*/
	IF @intDWCTeamID > 0
		SELECT vsDWCTeamID, B.DWCTeamName AS vsDWCTeamName, A.DWCPointChangeCode, A.DWCPointBefore, A.DWCPoint, A.RegisterDate 
		FROM dbo.DWCTeamPointPlayHistory A WITH(NOLOCK, INDEX(PK_DWCTeamPointPlayHistory), FORCESEEK)
			INNER JOIN dbo.DWCTeams B WITH(NOLOCK, INDEX(PK_DWCTeams), FORCESEEK) ON B.DWCTeamID = A.vsDWCTeamID
		WHERE A.DWCTeamID = @intDWCTeamID
          AND A.RegisterDate BETWEEN @dt2StartDate AND @dt2EndDate
		OPTION(KEEPFIXED PLAN);
	ELSE IF @nvcDWCTeamName IS NOT NULL
		WITH CTE AS(
			SELECT DWCTeamID	
			FROM dbo.DWCTeams WITH(NOLOCK, INDEX(IX_UNF_DWCTeams_DWCTeamName), FORCESEEK)
			WHERE DWCTeamName = @nvcDWCTeamName AND CloseFlag = 0
		)
		SELECT B.vsDWCTeamID, C.DWCTeamName AS vsDWCTeamName, B.DWCPointChangeCode, B.DWCPointBefore, B.DWCPoint, B.RegisterDate 
		FROM CTE A
			INNER JOIN dbo.DWCTeamPointPlayHistory B WITH(NOLOCK, INDEX(PK_DWCTeamPointPlayHistory), FORCESEEK) ON B.DWCTeamID = A.DWCTeamID
			INNER JOIN dbo.DWCTeams C WITH(NOLOCK, INDEX(PK_DWCTeams), FORCESEEK) ON C.DWCTeamID = B.vsDWCTeamID
		WHERE B.RegisterDate BETWEEN @dt2StartDate AND @dt2EndDate
		OPTION(KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103361;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
