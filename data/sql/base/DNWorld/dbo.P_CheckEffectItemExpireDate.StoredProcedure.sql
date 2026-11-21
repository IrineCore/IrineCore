USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_CheckEffectItemExpireDate]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-10
description : ?? ???? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103195 = ?? ???? ?? ??? ???????.
history :
2 = ??? ?? ?? by ??? at 2012-01-16
**/

CREATE PROCEDURE [dbo].[P_CheckEffectItemExpireDate]
	  @inbCharacterID bigint --// ??? ID
	, @intItemID int --// ??? ID
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
	IF NOT EXISTS (
		SELECT *
		FROM dbo.EffectItems WITH (NOLOCK, INDEX(IX_NC_EffectItems_CharacterID), FORCESEEK)
		WHERE CharacterID = @inbCharacterID AND ItemID = @intItemID AND EffectItemExpireDate > CAST(GETDATE() AS smalldatetime)
	)
	BEGIN
		SET @intReturnValue = 103195;
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
