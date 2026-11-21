USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModDarkLairRanking]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : sj-yang
e-mail : therye@eyedentitygames.com
created date : 2010-10-07
description : ModDarkLairRanking ?? ??? ?? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103268 = Update ? ?? ????. (DarkLairRanking)
**/

CREATE PROCEDURE [dbo].[P_ModDarkLairRanking]
	  @inbDarkLairRankingID bigint --// ???? ?? ID
	, @bitBlockFlag bit --// ?? ??(0:??, 1:??) : ?? 1 ??? ??? ???? ???.
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
		/**_# ?? UPDATE ???. */
		UPDATE dbo.DarkLairRanking
		SET   BestFlag = 0
			, BlockFlag = @bitBlockFlag
		WHERE DarkLairRankingID = @inbDarkLairRankingID;

		/**_## UPDATE? ?? ??? 103268 ?(?) ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103268;
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
