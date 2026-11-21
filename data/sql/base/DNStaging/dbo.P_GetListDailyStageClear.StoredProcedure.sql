USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDailyStageClear]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-12-01
description : ??? ???? ??? ??? ?????.
result set : BasisDate, StageID, DifficultyCode, ChallengeCount, ClearCount, AvgClearTime, MinClearTime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListDailyStageClear]
	  @datBasisDate date --// ?? ?? ??	
	, @intStageID int = NULL --// ???? ID
	, @inyDifficultyCode tinyint = NULL --// 1=??, 2=??, 3=???, 4=???, 5=???	
WITH EXECUTE AS 'SPExecutor'
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
	/**_# [ST_DailyStageClear] ????? ???? ??? ??? Select ???..*/
	SELECT BasisDate, StageID, DifficultyCode, ChallengeCount, ClearCount, AvgClearTime, MinClearTime
      FROM dbo.ST_DailyStageClear WITH (NOLOCK)
     WHERE BasisDate = @datBasisDate
       AND StageID = COALESCE(@intStageID, StageID)
	   AND DifficultyCode = COALESCE(@inyDifficultyCode, DifficultyCode)
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
