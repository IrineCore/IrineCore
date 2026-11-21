USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGuildDepotSize]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-05
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103252 = ??? ???? ??? ?? ?? ??? ?? ?/? ??? ?????.
**/

CREATE PROCEDURE [dbo].[P_ModGuildDepotSize]
	  @intGuildID int --// ?? ID
	, @insGuildDepotSize smallint --// ?? ?? ??
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
	/**_# [Guilds] ???? UPDATE???.*/
	UPDATE dbo.Guilds
	SET GuildDepotSize = @insGuildDepotSize
	WHERE GuildID = @intGuildID AND GuildDepotSize <> @insGuildDepotSize;

	/**_# ??? ???? ??? ?? ?? ??? ??? ??? 103252? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103252;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ?????.*/
	INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, RegisterDate, GuildDepotSize)
	VALUES (@intGuildID, 10, GETDATE(), @insGuildDepotSize);
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
