USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListWeeklyLike]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-13
description : ?? ??? ??? ?????.
result set : CharacterName/nvarchar(30)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListWeeklyLike]
	  @inbCharacterID bigint --// ???ID	
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
	/**_# [WeeklyLikes] ???? SELECT???.*/
	SELECT TOP 20 CH.CharacterName
	FROM dbo.WeeklyLikes WL WITH (NOLOCK) INNER JOIN dbo.Characters CH WITH (NOLOCK) ON WL.CharacterID = CH.CharacterID
	WHERE WL.LikeCharacterID = @inbCharacterID
	ORDER BY WL.LikeDate DESC;

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
