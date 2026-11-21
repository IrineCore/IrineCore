USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPVPLadderScoresByJobForSnapShot]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-11-06
description : ?? PVP ??? ?? ??? ?????.(DailySnapshot)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListPVPLadderScoresByJobForSnapShot]
	  @dtmBasisDate datetime 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intWorldID int

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP 1 @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID

	SELECT @intWorldID AS WorldID
		 , CAST(DATEADD(DAY, -1, @dtmBasisDate) AS date) AS BasisDate
		 , CharacterID
		 , JobCode
		 , Win
		 , Lose
		 , Draw
	FROM dbo.PVPLadderScoresByJob WITH (NOLOCK)
	OPTION (MAXDOP 1);
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
