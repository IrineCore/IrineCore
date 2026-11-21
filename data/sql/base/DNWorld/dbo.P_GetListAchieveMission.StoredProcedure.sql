USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAchieveMission]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-15
description : ??? ?? ??? ?????.
result set : 
	version 1 = MissionID/int, AchieveDate/datetime2, MissionTypeCode/tinyint
	version 2 = MissionID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = (59407) intput param. @intVersion ?? by ??? at 2012-06-12
**/

CREATE PROCEDURE [dbo].[P_GetListAchieveMission]
	  @inbCharacterID bigint --// ??? ID
	, @intVersion int = 1 --// ??? SP? ??
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
	IF @intVersion = 1
		SELECT MissionID, AchieveDate, MissionTypeCode
		FROM dbo.AchieveMissions WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID
		ORDER BY AchieveDate;

	ELSE IF @intVersion = 2
		SELECT MissionID
		FROM dbo.AchieveMissions WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID;

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
