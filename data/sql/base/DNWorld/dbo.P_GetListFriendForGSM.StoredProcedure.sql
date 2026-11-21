USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFriendForGSM]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-17
description : ?? ??? ?????. (?? ???? ??? ??? ??? & ?? ???? ??? ??? ???)
result set : FriendGroupName/nvarchar, AccountID/int, AccountName/nvarchar, CharacterID/bigint, CharacterName/nvarchar, JobCode/tinyint, CharacterLevel/tinyint, FriendMemo/nvarchar, RegisterDate/datetime2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103133 = ?? ???? ????.
history :
3 = ??? ??? ?? ???? ???? ???? ?? ?? ?? at 2010-06-16
**/

CREATE PROCEDURE [dbo].[P_GetListFriendForGSM]
	  @inbCharacterID bigint --// ??? ID
	, @inyListType tinyint --// 1=?? ???? ??? ??? ??? ??, 2=?? ???? ??? ??? ??? ??
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
	IF @inyListType = 1
		SELECT FG.FriendGroupName, VC.AccountID, VC.AccountName, VC.CharacterID, VC.CharacterName, VC.JobCode, VC.CharacterLevel, F.FriendMemo, F.RegisterDate
		FROM dbo.Friends F WITH (NOLOCK)
			INNER JOIN dbo.V_Characters VC WITH (NOLOCK) ON F.FriendCharacterID = VC.CharacterID
			LEFT OUTER JOIN dbo.FriendGroups FG WITH (NOLOCK) ON F.FriendGroupID = FG.FriendGroupID
		WHERE F.CharacterID = @inbCharacterID AND VC.DeleteFlag = CAST(0 AS bit)
		ORDER BY F.FriendGroupID, VC.CharacterName;

	ELSE IF @inyListType = 2
		SELECT FG.FriendGroupName, VC.AccountID, VC.AccountName, VC.CharacterID, VC.CharacterName, VC.JobCode, VC.CharacterLevel, F.FriendMemo, F.RegisterDate
		FROM dbo.Friends F WITH (NOLOCK)
			INNER JOIN dbo.V_Characters VC WITH (NOLOCK) ON F.CharacterID = VC.CharacterID
			LEFT OUTER JOIN dbo.FriendGroups FG WITH (NOLOCK) ON F.FriendGroupID = FG.FriendGroupID
		WHERE F.FriendCharacterID = @inbCharacterID AND VC.DeleteFlag = CAST(0 AS bit)
		ORDER BY VC.CharacterName, F.FriendGroupID;

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
