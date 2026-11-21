USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDWCChannel]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-15
description : ???? ?? DWC ??? ?????.
result set : DWCChannelID/int, DWCStartTime/datetime2(0), DWCEndTime/datetime2(0), EnableFlag/bit, StatusCode/tinyint, LadderTime1/int, LadderTime2/int, LimitCharacterLevel/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListDWCChannel]
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
	/**_# [DWCChannels] ???? ?????.*/
	SELECT DWCSequence AS DWCChannelID, DWCStartTime, DWCEndTime, EnableFlag, StatusCode, LadderTime1, LadderTime2, LimitCharacterLevel
	FROM dbo.DWCChannels WITH(NOLOCK);

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
