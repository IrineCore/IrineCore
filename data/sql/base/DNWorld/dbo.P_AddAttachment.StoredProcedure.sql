USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddAttachment]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : sj-yang
e-mail : therye@eyedentitygames.com
created date : 2010-09-30
description : <????? ?????.>
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. DBA ?? ?? ???.
**/

CREATE PROCEDURE [dbo].[P_AddAttachment]
	  @intBoardSeq int --// ??? ????
	, @nvcRealFileName nvarchar(100) --// ?? ???
	, @nvcChangedFileName nvarchar(100) --// ???? ???? ???
	, @nvcFileDescription nvarchar(200) = NULL --// ?? ??
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
	/**_# [BoardAttachment] ???? INSERT???.*/
	INSERT dbo.BoardAttachment (BoardSeq, RealFileName, ChangedFileName, AttachementDate, FileDescription)
	VALUES (@intBoardSeq, @nvcRealFileName, @nvcChangedFileName, GETDATE(), @nvcFileDescription);
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
