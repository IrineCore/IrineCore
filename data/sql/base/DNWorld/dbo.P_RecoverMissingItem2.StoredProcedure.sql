USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_RecoverMissingItem2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-21
description : ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103217 = ??? ? ?? ??????. - ???? ??? ???? ???? ??? ?? ??? 0????.
history :
3 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
**/

CREATE PROCEDURE [dbo].[P_RecoverMissingItem2]
	  @inbItemSerial bigint --// ??? ???? ??? ???
	, @inbCharacterID bigint --// ??? ID
	, @inyItemLocationCode tinyint --// 2=????, 4=??? ????, 5=?? ????, 8=??, 9=??, 10=?
	, @inyItemLocationIndex tinyint --// ???? ??? ??? ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbMaterializedItemID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	UPDATE dbo.MaterializedItems
	SET ItemLocationCode = @inyItemLocationCode
		, ItemLocationIndex = @inyItemLocationIndex
	WHERE ItemSerial = @inbItemSerial
		AND OwnerCharacterID = @inbCharacterID
		AND (
			ItemLocationCode IS NULL OR (ItemLocationCode = 12 AND NOT EXISTS (
				SELECT *
				FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_ItemSerial), FORCESEEK)
				WHERE ItemSerial = @inbItemSerial AND TradeStatusCode = 0
				)
			)
		)
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103217;
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
