USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPVPTournamentWinner]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-11-01
description : PVP ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListPVPTournamentWinner]
	  @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ?? ??
	, @inyPlayerCount tinyint = 0 --// (optional)?? ?? ?
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

	SELECT A.PVPTournamentResultDate, A.PlayerCount, A.PVPTournamentID, B.PVPTournamentSeqence, B.CharacterID, C.CharacterName, B.FinalRound, B.ResultFlag
	FROM dbo.PVPTournamentIDs A WITH(NOLOCK, INDEX(IX_NC_PVPTournamentIDs_PVPTournamentResultDate), FORCESEEK)
		INNER JOIN dbo.PVPTournamentResults B WITH(NOLOCK, INDEX(PK_PVPTournamentResults), FORCESEEK) ON A.PVPTournamentID = B.PVPTournamentID
		INNER JOIN dbo.Characters C WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK) ON B.CharacterID = C.CharacterID
	WHERE A.PVPTournamentResultDate BETWEEN @dt2StartDate AND @dt2EndDate
	  AND B.ResultFlag = 1
      AND A.PlayerCount >= @inyPlayerCount
	ORDER BY A.PVPTournamentID, B.PVPTournamentSeqence
	OPTION(KEEPFIXED PLAN);

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
