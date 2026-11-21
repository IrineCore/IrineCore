USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacterForNexon]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-10
description : ?? ??, ??? ?? ??? ??? ?????.
result set : AccountName, CharacterID, CharacterName, CharacterLevel, JobCode, GuildName, WorldID, WorldName
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = DNWorld DB? ?? ????? ?????. at 2011-01-28
**/

CREATE PROCEDURE [dbo].[P_GetListCharacterForNexon]
	  @nvcAccountName nvarchar(50) --// ??? ?? ??
	, @intWorldID int --// ?? ID
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @nvcStmt nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF NOT EXISTS (SELECT * FROM sys.servers WHERE name = N'World3')
		RETURN 0;

	SET @nvcStmt = N'
SELECT CH.AccountName, CH.CharacterID, CH.CharacterName, CHS.CharacterLevel, CHS.JobCode, G.GuildName, CH.WorldID, DW.WorldName
FROM World3.DNWorld.dbo.Characters CH WITH (NOLOCK)
	INNER JOIN World3.DNWorld.dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
	INNER JOIN dbo.DimWorlds DW WITH (NOLOCK) ON CH.WorldID = DW.WorldID
	LEFT OUTER JOIN World3.DNWorld.dbo.GuildMembers GM WITH (NOLOCK) ON CH.CharacterID = GM.CharacterID
	LEFT OUTER JOIN World3.DNWorld.dbo.Guilds G WITH (NOLOCK) ON GM.GuildID = G.GuildID
WHERE CH.AccountName = @nvcAccountName AND CH.WorldID = @intWorldID AND CH.DeleteFlag = CAST(0 AS bit);'

	EXEC sp_executesql @nvcStmt
		, N'@nvcAccountName nvarchar(50), @intWorldID int'
		, @nvcAccountName = @nvcAccountName
		, @intWorldID = @intWorldID;
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
