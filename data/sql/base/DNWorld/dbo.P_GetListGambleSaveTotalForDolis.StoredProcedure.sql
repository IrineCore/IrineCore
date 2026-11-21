USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGambleSaveTotalForDolis]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-24
description : ?? ???? ? ??? ??? ?????.
resultset : GambleFeeCode/tinyint, PlayCount, SaveTotal
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListGambleSaveTotalForDolis]
	  @dt2GambelStartDate datetime2(0) --// ??????
	, @dt2GambelEndDate datetime2(0) --// ??????
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
	/**_# ?? ???? ??? ??, ? ???? ?????.*/
	SELECT GambleFeeCode, COUNT(*) AS PlayCount, SUM( (A.EntranceFee * B.GambleParticipantesCount) - A.WinPrize - A.SemiWinPrize - ((A.EntranceFee * B.GambleParticipantesCount)/100*10) ) AS SaveTotal
	FROM dbo.GambleIDs A WITH(NOLOCK, INDEX(IX_NC_GambleIDs_GambelStartDate), FORCESEEK)
		INNER JOIN dbo.VIX_GambleParticipantesTotal B WITH(NOLOCK, NOEXPAND) ON A.GambleID = B.GambleID
	WHERE GambelStartDate BETWEEN @dt2GambelStartDate AND @dt2GambelEndDate
	  AND RewardFlag = 0
      AND GambelEndDate IS NOT NULL
	GROUP BY GambleFeeCode;

	
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
