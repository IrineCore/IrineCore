USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelComment]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : sj-yang
e-mail : therye@eyedentitygames.com
created date : 2010-09-30
description : <Description,, ?????.>
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
102112 = ???? ? ?? ????.
**/

CREATE PROCEDURE [dbo].[P_DelComment]
	@intBoardSeq int --// ??? ??
	,@intCommentsSeq int --// ??? ???
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

	BEGIN TRANSACTION;
		/**_# DeleteFlag ? 1 ? ???????. */
		UPDATE dbo.BoardComments SET DeleteFlag = 1
		WHERE CommentsSeq = @intCommentsSeq;

		/**_## Update ? ?? ??? 102112 ?(?) ?????. */
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 102112;
			GOTO ErrorHandler;
		END
		ELSE
		BEGIN
		/**_## ?? ????? ???? ???? CommentCount ? 1 ?? ????. */
			UPDATE dbo.BoardMain SET CommentCount = CommentCount - 1 WHERE BoardSeq = @intBoardSeq;
		END
	COMMIT TRANSACTION;
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
