USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_DelEvent]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-03
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
104102 = ???? ????.
104103 = ?? ???? ??? ??????. ???? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_DelEvent]
	  @intEventID int --// ??? ID
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
	/**_# ?? ??????.*/
	DELETE dbo.Events WHERE EventID = @intEventID;

	/**_## ??? ?? ???, 104102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 104102;
		GOTO ErrorHandler;
	END
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Events_EventResults_EventID%'
		SET @intReturnValue = 104103;

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
