USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddComment]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : sj-yang
e-mail : therye@eyedentitygames.com
created date : 2010-09-30
description : <??? ??>
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddComment]
	  @intBoardSeq int --// ??? ??
	, @intUserIndex int --// ?? ?? ????
	, @nvcUserID nvarchar(50) --// ?? ?? ???
	, @nvcComments nvarchar(max) --// ??? ??
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
	BEGIN TRAN
		/**_# [BoardComments] ???? INSERT???.*/
		INSERT dbo.BoardComments (BoardSeq, UserIndex, UserID, Comments, WriteDate, UpdateDate, DeleteDate, DeleteFlag)
		VALUES (@intBoardSeq, @intUserIndex, @nvcUserID, @nvcComments, GETDATE(), NULL, NULL, 0);
		/**_# CommentCount ? 1 ?? ????.*/		
		UPDATE dbo.BoardMain SET CommentCount = CommentCount + 1 WHERE BoardSeq = @intBoardSeq;
	COMMIT;
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
