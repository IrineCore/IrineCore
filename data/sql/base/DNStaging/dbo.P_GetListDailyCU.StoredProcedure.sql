USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDailyCU]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-19
description : ? ?? ?? ??? ?????. WorldID, WorldName ??? NULL? ???? ?? ?? ??? ?????.
result set : BasisDate/smalldatetime, WorldID/int, WorldName/nvarchar, CUCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
History
20101021 = ????? ??? ??? ?? ??? ??
**/

CREATE PROCEDURE [dbo].[P_GetListDailyCU]
	  @sdtStartDate smalldatetime --// ?? ?? ??
	, @sdtEndDate smalldatetime --// ?? ? ??
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
	SELECT L.BasisDate, L.WorldID, W.WorldName, CUCount, L.LogDate
	FROM dbo.ST_DailyMaxCU L WITH (NOLOCK)
		LEFT OUTER JOIN dbo.DimWorlds W ON L.WorldID = W.WorldID
	WHERE BasisDate BETWEEN @sdtStartDate AND @sdtEndDate
	  AND ChannelPartnerCode IS NULL
	ORDER BY L.WorldID, BasisDate;
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
