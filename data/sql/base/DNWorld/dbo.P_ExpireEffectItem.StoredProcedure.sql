USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ExpireEffectItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-30
description : ??? ??? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history : 
2~3 = ????? ?? ??? ?? by ??? at 2011-12-23
4 = Hint ?? ? ???? ?? by ??? at 2013-01-05
**/

CREATE PROCEDURE [dbo].[P_ExpireEffectItem]
	  @inbCharacterID bigint --// ??? ID
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
	DELETE dbo.EffectItems
	FROM dbo.EffectItems WITH (INDEX(IX_NC_EffectItems_CharacterID), FORCESEEK)
	WHERE CharacterID = @inbCharacterID 
		AND EternityFlag = CAST(0 AS bit) 
		AND EffectItemExpireDate <= CAST(GETDATE() AS smalldatetime)
	OPTION (KEEPFIXED PLAN);
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
