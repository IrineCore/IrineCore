USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListTradeByCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-18

description :
	?? ???? ???? ??? ??? ??? ?????.
result set :
	TradeID	int
	ItemSerial	bigint
	ItemID	int
	ItemLevel	tinyint
	ItemCount	smallint
	Price	int
	RegisterDate	datetime2(0)
	TradeExpireDate	datetime2(0)
	TradeStatusCode	tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103172 = ?? ??? ??? ???? ????.
history :
	2 = ???? ???? ?? ????? ???? ??     by 20111013.wgkim
**/

CREATE PROCEDURE [dbo].[P_GetListTradeByCharacter]
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
	SELECT TradeID, ItemSerial, ItemID, ItemLevel, ItemCount, Price, RegisterDate, TradeExpireDate, TradeStatusCode
	FROM dbo.Trade WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID AND TradeStatusCode IN (0, 1)
	ORDER BY TradeID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103172;
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
