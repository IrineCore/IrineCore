USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModAdditiveItemID]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-19
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103292 = ???? ???? ?? ??????.
history :
3 = (44640) input param. @inySealCountAfter ?? by ??? at 2011-11-10
**/

CREATE PROCEDURE [dbo].[P_ModAdditiveItemID]
	  @inbItemSerial bigint --// ??? ?? ??
	, @intAdditiveItemID int --// ???? ??? ??? ??? ID
	, @inbCharacterID bigint --// ??? ID
	, @inySealCountAfter tinyint = NULL --// ?? ? ?? ?? ??? ??? ?????. NULL? ???? ???? ????.
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
	/**_# ???? ???? ?? ????? 103292? ?????.*/
	IF EXISTS (
		SELECT *
		FROM dbo.Trade TR WITH (NOLOCK)
		WHERE TR.TradeStatusCode = 0 AND TR.ItemSerial = @inbItemSerial
	)
	BEGIN
		SET @intReturnValue = 103292;
		GOTO ErrorHandler;
	END

	UPDATE dbo.MaterializedItems
	SET AdditiveItemID = @intAdditiveItemID
		, SealCount = CASE WHEN @inySealCountAfter IS NULL THEN SealCount ELSE @inySealCountAfter END
	WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbCharacterID;
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
