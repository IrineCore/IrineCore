USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetDailyLike]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-13
description : ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetDailyLike]
	  @inbCharacterID bigint --// ???ID
	, @inbLikeCharacterID bigint --// ???? ?? ???ID
	, @bitLikeFlag bit = NULL OUTPUT --// ??? ??
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
	/**_# [DailyLikes] ???? SELECT???.*/
	IF EXISTS (SELECT * FROM dbo.DailyLikes WITH (NOLOCK) WHERE CharacterID = @inbCharacterID AND LikeCharacterID = @inbLikeCharacterID)
		SET @bitLikeFlag = CAST(1 AS bit);
	ELSE
		SET @bitLikeFlag = CAST(0 AS bit);

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
