USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFriendRequest]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-03
description : ???? ??? ?????.
result set : FriendCharacterID/bigint, FriendCharacterName/nvarchar, RequestCode/tinyint(1=?? ?? ??? ???, 2=??? ?? ??? ???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListFriendRequest]
	  @inbCharacterID bigint --// ??? ID	
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
	/**_# [FriendRequests] ???? SELECT???.*/
	;WITH A AS (
		SELECT FriendCharacterID, CAST(1 AS tinyint) AS RequestCode
		FROM dbo.FriendRequests WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID
		UNION ALL
		SELECT CharacterID AS FriendCharacterID, CAST(2 AS tinyint) AS RequestCode
		FROM dbo.FriendRequests WITH (NOLOCK)
		WHERE FriendCharacterID = @inbCharacterID
	)
	SELECT A.FriendCharacterID, CH.CharacterName AS FriendCharacterName, A.RequestCode
	FROM A INNER JOIN dbo.Characters CH WITH (NOLOCK) ON A.FriendCharacterID = CH.CharacterID
	WHERE CH.DeleteFlag = 0;

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
