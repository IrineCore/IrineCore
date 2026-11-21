USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuildRewards]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-29
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetGuildRewards]
	  @intGuildID int --// ??ID
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
	/**_# ??? ??? ???? DELETE???.*/
	DELETE dbo.GuildRewards 
	WHERE GuildID = @intGuildID AND EternityFlag = 0
	  AND ItemExpireDate <= GETDATE();
		
	/**_# [GuildRewards] ???? SELECT???.*/
	SELECT GuildItemID, EternityFlag, RegisterDate, ItemExpireDate
	FROM dbo.GuildRewards WITH(NOLOCK, INDEX(PK_GuildRewards), FORCESEEK)
	WHERE GuildID = @intGuildID
	OPTION (KEEPFIXED PLAN)
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
