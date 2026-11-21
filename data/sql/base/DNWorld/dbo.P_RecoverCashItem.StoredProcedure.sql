USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_RecoverCashItem]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-01-26
description : ??? ?????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_RecoverCashItem]
	  @inbItemSerial bigint --// ??? ?? ??
	, @insItemRemainCount smallint --// ??? ??
	, @bitEternityFlag bit OUTPUT --// 0=???? ?? ??, 1=?? ???
	, @sdtItemExpireDate smalldatetime OUTPUT --// ??? ?? ??
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
	UPDATE dbo.MaterializedItems
	SET ExpireCompleteFlag = 0
		, @bitEternityFlag = EternityFlag =
			CASE ItemLifespan
				WHEN 2628000 THEN 1
				ELSE 0
			END
		, @sdtItemExpireDate = ItemExpireDate = DATEADD(MINUTE, ItemLifespan, ItemMaterializeDate)
	WHERE ItemSerial = @inbItemSerial
		AND ItemLocationCode = 5
		AND ItemRemainCount =  @insItemRemainCount
		AND ExpireCompleteFlag = CAST(1 AS bit);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
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
