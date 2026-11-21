USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListNestRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-08-16
description : ??? ??? ??? ?????.
result set : CharacterID/bigint, CharacterName/nvarchar, JobCode/tinyiny, CharacterLevel/tinyint, GuildName/nvarchar, ClearTime/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListNestRanking]
	  @intMapID int --// ?ID	
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
	/**_# [NestClearRecord] ???? SELECT???.*/
	;WITH CR AS (
		SELECT CharacterID, ClearTime, MIN(ClearDate) AS ClearDate
		FROM dbo.NestClearRecord WITH (NOLOCK)
		WHERE MapID = @intMapID
		GROUP BY CharacterID, ClearTime
	), NR AS (
		SELECT RANK() OVER(ORDER BY ClearTime) AS Ranking, CharacterID, ClearTime, ClearDate
		FROM CR
	)
	SELECT NR.CharacterID, CH.CharacterName, CS.JobCode, CS.CharacterLevel, G.GuildName, NR.ClearTime
	FROM NR INNER JOIN dbo.Characters CH WITH (NOLOCK) ON NR.CharacterID = CH.CharacterID
	     INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID
		 LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON CH.CharacterID = GM.CharacterID
		 LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK) ON GM.GuildID = G.GuildID	
	WHERE Ranking <= 5;
	
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
