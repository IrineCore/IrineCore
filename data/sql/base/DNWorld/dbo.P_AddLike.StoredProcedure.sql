USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddLike]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-13
description : ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103402 = ?? ?? ??? ? ??????.
103405 = ?? ??? ??? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_AddLike]
	  @inbCharacterID bigint --// ??? ID		
	, @inbLikeCharacterID bigint --// ??? ? ??? ID
	, @intMaxLikeCount int --// ?? ?? ??? ??
	, @intLikeCount int OUTPUT --// ?? ? LikeCount
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2 = GETDATE()
	  , @intTodayLikeCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ?? ???? ????? ?????.*/
	IF EXISTS (SELECT * FROM dbo.DailyLikes WITH (NOLOCK) WHERE CharacterID = @inbCharacterID AND LikeCharacterID = @inbLikeCharacterID)
	BEGIN
		SET @intReturnValue = 103402;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ??? ????? ?????.*/
	SELECT @intTodayLikeCount = LikeCount
	FROM dbo.VIX_DailyLikeCount WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;
	
	IF @intTodayLikeCount >= @intMaxLikeCount
	BEGIN
		SET @intReturnValue = 103405;
		GOTO ErrorHandler;
	END

	/**_# [CharacterStatus] ???? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @intLikeCount = LikeCount = LikeCount + 1
	WHERE CharacterID = @inbLikeCharacterID;

	/**_# ?? ??? ??? ?????.*/
	INSERT dbo.DailyLikes (CharacterID, LikeCharacterID, LikeDate)
	SELECT @inbCharacterID, @inbLikeCharacterID, @dt2Now;

	/**_# ?? ??? ??? ?????.*/
	INSERT dbo.WeeklyLikes (CharacterID, LikeCharacterID, LikeDate)
	SELECT @inbCharacterID, @inbLikeCharacterID, @dt2Now;

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
