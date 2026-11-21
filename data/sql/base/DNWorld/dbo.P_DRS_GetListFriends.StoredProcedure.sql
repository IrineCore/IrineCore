USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListFriends]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-04-26
description : ?????? ?????.
result set : FriendGroupID/int, FriendGroupName/nvarchar, FriendAccountID/int, FriendCharacterID/bigint, FriendCharacterName/nvarchar, CharacterLevel/tinyin, JobCode/tinyint, MobileAuthenticationFlag/bit, LastVillageMapID/int, LastLogoutDate/datetime2, LastMapID/int, LastServerType/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103133 = ?? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListFriends]
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
	/**_# ?? ??? ?????.*/
	;WITH C1 (FriendGroupID, FriendAccountID, FriendCharacterID, FriendCharacterName, FriendMemo, CharacterLevel, JobCode, MobileAuthenticationFlag, LastVillageMapID, LastLogoutDate, LastMapID, LastServerType) AS (
		SELECT FR.FriendGroupID, CH.AccountID, CH.CharacterID, CH.CharacterName, FR.FriendMemo, CS.CharacterLevel, CS.JobCode, CH.MobileAuthenticationFlag, CS.LastVillageMapID, CS.LastLogoutDate, CS.LastMapID, CS.LastServerType
		FROM dbo.Friends FR WITH (NOLOCK)
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON FR.FriendCharacterID = CH.CharacterID
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON FR.FriendCharacterID = CS.CharacterID
		WHERE FR.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
	), C2 AS (
		SELECT FriendGroupID, FriendGroupName
		FROM dbo.FriendGroups FG WITH (NOLOCK)
		WHERE FG.CharacterID = @inbCharacterID
	)
	SELECT C2.FriendGroupID, C2.FriendGroupName, C1.FriendAccountID, C1.FriendCharacterID, C1.FriendCharacterName, C1.CharacterLevel, C1.JobCode, C1.MobileAuthenticationFlag, C1.LastVillageMapID, C1.LastLogoutDate, C1.LastMapID, C1.LastServerType
	FROM C1
		FULL OUTER JOIN C2 ON C1.FriendGroupID = C2.FriendGroupID
	ORDER BY C2.FriendGroupID;

	/**_# ?? ??? ??? 103133? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103133;
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
