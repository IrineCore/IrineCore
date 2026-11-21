USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddFieldAttachment]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-13
description : ??? ?? ???? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103281 = ???? ????.
**/

CREATE PROCEDURE [dbo].[P_AddFieldAttachment]
	  @intFarmID int --// ?? ID
	, @insFieldIndex smallint --// ?? ID
	, @intAttachedItemID int --// ??? ??? ID
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
	/**_# [FieldAttachments] ???? INSERT???.*/
	/**_## ???? ??? 103281? ?????.*/
	INSERT dbo.FieldAttachments (FarmID, FieldIndex, AttachedItemID)
	VALUES (@intFarmID, @insFieldIndex, @intAttachedItemID);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Fields_FieldAttachments_FarmID_FieldIndex%'
		SET @intReturnValue = 103281;

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
