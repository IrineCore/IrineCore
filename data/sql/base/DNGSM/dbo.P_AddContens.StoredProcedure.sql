USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_AddContens]    Script Date: 11/14/2026 8:47:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
version : 11
author : sj-yang
e-mail : therye@eyedentitygames.com
created date : 2010-09-30
description : <???? ?????.>
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. DBA ?? ?? ???.
**/

CREATE PROCEDURE [dbo].[P_AddContens]
	  @intUserIndex int --// ?? ????
	, @nvcUserID nvarchar(50) --// ?? ???
	, @nvcSubject nvarchar(200) --// ? ??
	, @nvcContents nvarchar(max) --// ? ??
	, @intAttachedFlag tinyint --// ???? ??(0:???? ??, 1:???? ??)
	, @intBoardSeq int output --// ??? ???? ??
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
	/**_# [BoardMain] ???? INSERT???.*/
	INSERT dbo.BoardMain (UserIndex, UserID, [Subject], Contents, WriteDate, UpdateDate, DeleteDate, DeleteFlag, AttachedFlag, CommentCount)
	VALUES (@intUserIndex, @nvcUserID, @nvcSubject, @nvcContents, GETDATE(), NULL, NULL, 0, @intAttachedFlag, 0);
	SET @intBoardSeq = SCOPE_IDENTITY();
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	SET @intReturnValue =100;

RETURN @intReturnValue;
GO
