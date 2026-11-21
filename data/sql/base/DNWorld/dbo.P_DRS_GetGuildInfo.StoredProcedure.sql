USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetGuildInfo]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-04-26
description : ????? ?????.
result set : GuildName/int, GuildLevel/tinyint, NoticeContent/nvarchar(50), MemberCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103238 = ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetGuildInfo]	   
	@intGuildID int  --// ?? ID
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
	/**_# [Guilds] ???? Select ???.*/
	SELECT GuildName, GuildLevel, NoticeContent, (SELECT COUNT(*) FROM dbo.GuildMembers WITH (NOLOCK) WHERE GuildID = @intGuildID) AS MemberCount
	FROM dbo.Guilds WITH (NOLOCK) 
	WHERE GuildID = @intGuildID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103238;
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
