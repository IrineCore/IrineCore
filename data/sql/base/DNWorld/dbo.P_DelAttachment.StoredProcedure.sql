USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelAttachment]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : sj-yang
e-mail : therye@eyedentitygames.com
created date : 2010-09-30
description : <?? ??? ?????.>
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. DBA ?? ?????.
102113 = ??? ?? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DelAttachment]
	  @intAttachmentSeq int --// ??? ?? ??? ?? ??
WITH EXECUTE AS 'SPExecutor'
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
	/**_# ?? ????, ?? ? ???? ??? ??? ?? ?????.*/
	DELETE dbo.BoardAttachment		
	WHERE AttachmentSeq = @intAttachmentSeq;

	/**_## ??? ?? ??? 102113 ?(?) ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 102113;
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
	SET @intReturnValue = 100;

RETURN @intReturnValue;
GO
