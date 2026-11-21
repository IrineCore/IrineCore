USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModWishProducts]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-06-08

description :
	??? ?? ?? ?? ?? (??? ?? ?? ID ??)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103340 = ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ModWishProducts]
	  @inbMailID bigint --// ??ID
	, @inbPurchaseOrderID bigint --// ??? ?? ?? ID
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

	/**_# ??? ?? ?? ID? ?????.*/
	UPDATE dbo.WishProducts
	   SET PurchaseOrderID = @inbPurchaseOrderID
	 WHERE MailID = @inbMailID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103340; -- ??? ???? ????.
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
