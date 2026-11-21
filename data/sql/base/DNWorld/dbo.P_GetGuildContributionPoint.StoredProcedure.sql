USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuildContributionPoint]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-01-11
description : ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103242 = ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetGuildContributionPoint]
	  @inbCharacterID bigint --// ??? ID	
	, @intGuildContributionPoint int OUTPUT --// ?????
	, @intWeeklyGuildContributionPoint int OUTPUT --// ???????
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
	/**_# [GuildMembers] ???? SELECT???.*/
	SELECT @intGuildContributionPoint = GM.GuildContributionPoint, @intWeeklyGuildContributionPoint = WG.WeeklyGuildContributionPoint
	FROM dbo.GuildMembers GM WITH (NOLOCK) 
		 LEFT OUTER JOIN dbo.WeeklyGuildContributionPoint WG WITH (NOLOCK, INDEX(PK_WeeklyGuildContributionPoint), FORCESEEK) ON GM.CharacterID = WG.CharacterID
	WHERE GM.CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103242;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
