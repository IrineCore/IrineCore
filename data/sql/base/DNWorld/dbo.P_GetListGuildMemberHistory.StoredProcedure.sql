USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildMemberHistory]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-11-12
description : ??? ??(??/??/??) ??? ?????.
result set : 
		RegisterDate/datetime2, --// ?? ?? ?? ??
		GuildHistoryCode/tinyint, --// 3=??? ??, 4=??? ??, 5=??? ??, 6=??? ?? ??
		AccountID/int, --// ??ID
		AccountName/nvarchar(50), --// ???
		CharacterID/bigint, --// ??? ID
		CharacterName/nvarchar(30), --// ????
		JobCode/tinyint, --// ????
		CharacterLevel/tinyint, --// ??? ??
		GuildMemberRoleCode/tinyint--// 0=?? ???, 1=?? ??, 2=?? ???, 3=?? ???, 4=?? ???
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103353 = ????? ??ID? ??? ??? ???? ???.
**/

CREATE PROCEDURE [dbo].[P_GetListGuildMemberHistory]
	    @dt2StartDate datetime2(0) --// ?? ????
	  , @dt2EndDate datetime2(0) --// ?? ????
	  , @nvcGuildName nvarchar(50) = NULL --// ?? ??	
	  , @intGuildID int = NULL --// ?? ID	
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
	/**_# [GuildHistory] ???? SELECT???.*/
	IF @nvcGuildName IS NOT NULL AND @intGuildID IS NULL
		SELECT GH.RegisterDate, GH.GuildHistoryCode, CH.AccountID, CH.AccountName, GH.CharacterID, CH.CharacterName, CS.JobCode, GH.CharacterLevel, GH.GuildMemberRoleCode
		  FROM dbo.GuildHistory GH WITH (NOLOCK) 
			   LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON GH.CharacterID = CH.CharacterID
			   LEFT OUTER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON GH.CharacterID = CS.CharacterID
			   INNER JOIN dbo.Guilds GD WITH (NOLOCK) ON GH.GuildID = GD.GuildID
		 WHERE GH.RegisterDate >= @dt2StartDate AND GH.RegisterDate <= @dt2EndDate	       
			   AND GD.GuildName = @nvcGuildName
			   AND GH.GuildHistoryCode IN (3,4,5,6)
		 ORDER BY GH.RegisterDate DESC;
	ELSE IF @nvcGuildName IS NULL AND @intGuildID IS NOT NULL
		SELECT GH.RegisterDate, GH.GuildHistoryCode, CH.AccountID, CH.AccountName, GH.CharacterID, CH.CharacterName, CS.JobCode, GH.CharacterLevel, GH.GuildMemberRoleCode
		  FROM dbo.GuildHistory GH WITH (NOLOCK) 
			   LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON GH.CharacterID = CH.CharacterID
			   LEFT OUTER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON GH.CharacterID = CS.CharacterID
			   INNER JOIN dbo.Guilds GD WITH (NOLOCK) ON GH.GuildID = GD.GuildID
		 WHERE GH.RegisterDate >= @dt2StartDate AND GH.RegisterDate <= @dt2EndDate	       
			   AND GD.GuildID = @intGuildID
			   AND GH.GuildHistoryCode IN (3,4,5,6)
		 ORDER BY GH.RegisterDate DESC;
	ELSE
	BEGIN
		SET @intReturnValue = 103353;
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
