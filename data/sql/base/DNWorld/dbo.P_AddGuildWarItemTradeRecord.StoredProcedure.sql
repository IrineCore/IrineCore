USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarItemTradeRecord]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-09-23

description :
	??? ?? ???? ??? ??? ???? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarItemTradeRecord]
	  @inbCharacterID bigint --// ???ID
	, @intItemID int --// ???ID
	, @intItemCount int --// ????
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

	/**_# ??? ?? ???? ??? ??? ????? ???? ???. */
	UPDATE dbo.GuildWarItemTradeRecord
	   SET ItemCount = ItemCount + @intItemCount
	 WHERE CharacterID = @inbCharacterID
	   AND ItemID = @intItemID;

	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.GuildWarItemTradeRecord (CharacterID, ItemID, ItemCount)
		VALUES (@inbCharacterID, @intItemID, @intItemCount);
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
