USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacterByAccount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListCharacterByAccount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListCharacterByAccount] AS' 
END
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-07
description : ??? ?? ??? ??? ?????.
result set : WorldID, CharacterID, CharacterName, CharacterNameDeleteFlag, CreateDate, DeleteFlag, DeleteDate, DeletePrivateIP, DeletePublicIP, CharacterRestraintFlag, AccountID, JumpFlag
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101117 = ???? ???? ????.
history :
5 = AccountID? ????? ?? at 2010-10-25
6 = JumpFlag ????? ?? at 2011-08-03
**/

ALTER PROCEDURE [dbo].[P_GetListCharacterByAccount]
	  @intAccountID int --// ??? ?? ID
	, @nvcAccountName nvarchar(50) --// ??? ?? ??
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
	/**_# [Characters], [DeletedCharacters] ???? SELECT ???.*/
	IF @intAccountID IS NOT NULL
		SELECT CH.WorldID, CH.CharacterID, COALESCE(CH.CharacterName, DCH.CharacterName) AS CharacterName
			, CASE WHEN CH.CharacterName IS NULL THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END AS CharacterNameDeleteFlag
			, CH.CreateDate, CH.DeleteFlag, DCH.DeleteDate, dbo.FN_IPNumber2IP(DCH.DeletePrivateIP) AS DeletePrivateIP
			, dbo.FN_IPNumber2IP(DCH.DeletePublicIP) AS DeletePublicIP
			, CASE
				WHEN EXISTS (
					SELECT *
					FROM dbo.Restraints WITH (NOLOCK)
					WHERE CharacterID = CH.CharacterID AND CAST(GETDATE() AS smalldatetime) BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit)
				) THEN CAST(1 AS bit)
				ELSE CAST(0 AS bit)
			END AS CharacterRestraintFlag, CH.AccountID, CASE WHEN JC.CharacterID IS NULL THEN CAST(0 AS bit) ELSE CAST(1 AS bit) END AS JumpFlag
		FROM dbo.Characters CH WITH (NOLOCK)
			LEFT OUTER JOIN dbo.DeletedCharacters DCH WITH (NOLOCK) ON CH.CharacterID = DCH.CharacterID
			LEFT OUTER JOIN dbo.JumpingCharacters JC WITH (NOLOCK) ON CH.CharacterID = JC.CharacterID
		WHERE CH.AccountID = @intAccountID
		ORDER BY CH.DeleteFlag, CH.CharacterID DESC;

	ELSE
		SELECT CH.WorldID, CH.CharacterID, COALESCE(CH.CharacterName, DCH.CharacterName) AS CharacterName
			, CASE WHEN CH.CharacterName IS NULL THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END AS CharacterNameDeleteFlag
			, CH.CreateDate, CH.DeleteFlag, DCH.DeleteDate, dbo.FN_IPNumber2IP(DCH.DeletePrivateIP) AS DeletePrivateIP
			, dbo.FN_IPNumber2IP(DCH.DeletePublicIP) AS DeletePublicIP
			, CASE
				WHEN EXISTS (
					SELECT *
					FROM dbo.Restraints WITH (NOLOCK)
					WHERE CharacterID = CH.CharacterID AND CAST(GETDATE() AS smalldatetime) BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit)
				) THEN CAST(1 AS bit)
				ELSE CAST(0 AS bit)
			END AS CharacterRestraintFlag, CH.AccountID, CASE WHEN JC.CharacterID IS NULL THEN CAST(0 AS bit) ELSE CAST(1 AS bit) END AS JumpFlag
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.Accounts ACC WITH (NOLOCK) ON CH.AccountID = ACC.AccountID
			LEFT OUTER JOIN dbo.DeletedCharacters DCH WITH (NOLOCK) ON CH.CharacterID = DCH.CharacterID
			LEFT OUTER JOIN dbo.JumpingCharacters JC WITH (NOLOCK) ON CH.CharacterID = JC.CharacterID
		WHERE ACC.AccountName = @nvcAccountName
		ORDER BY CH.DeleteFlag, CH.CharacterID DESC;

	/**_# ??? ???? ?? ??? 101117? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101117;
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
