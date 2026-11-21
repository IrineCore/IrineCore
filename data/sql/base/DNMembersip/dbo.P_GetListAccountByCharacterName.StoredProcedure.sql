USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccountByCharacterName]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListAccountByCharacterName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListAccountByCharacterName] AS' 
END
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-07
description : ?? ??? ???? ??? ??? ??? ???? ?????.
result set : AccountID, AccountName, AccountLevelCode, CharacterCreateLimit, CharacterMaxCount, LastLoginDate, LastLogoutDate, LastLoginIP, JoinIP, RegisterDate, GenderCode, BirthDate, AccountRestraintFlag, CharacterCount, WorldID, CharacterID, CharacterName, CharacterNameDeleteFlag, CreateDate, DeleteFlag, DeleteDate, DeletePrivateIP, DeletePublicIP, CharacterRestraintFlag
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
history :
8 = ??? ??? exact match ???? ?? at 2010-06-11
**/

ALTER PROCEDURE [dbo].[P_GetListAccountByCharacterName]
	  @nvcCharacterName nvarchar(30) = NULL --// ??? ??
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
	;WITH C1 AS (
		SELECT DISTINCT AccountID
		FROM dbo.Characters WITH (NOLOCK)
		WHERE DeleteFlag = CAST(0 AS bit) AND CharacterName = @nvcCharacterName
		UNION ALL
		SELECT DISTINCT CH.AccountID
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.DeletedCharacters DCH WITH (NOLOCK) ON CH.CharacterID = DCH.CharacterID
		WHERE DCH.CharacterName = @nvcCharacterName
	)
	SELECT ACC.AccountID, ACC.AccountName, ACC.AccountLevelCode, CASE WHEN ACC.LastCharacterCreateDate < CAST(GETDATE() AS date) THEN 'MAX' ELSE CAST(ACC.CharacterCreateLimit AS varchar(10)) END AS CharacterCreateLimit
		, ACC.CharacterMaxCount, ACC.LastLoginDate, ACC.LastLogoutDate, dbo.FN_IPNumber2IP(ACC.LastLoginIP) AS LastLoginIP, dbo.FN_IPNumber2IP(ACC.JoinIP) AS JoinIP, ACC.RegisterDate, ACC.GenderCode, ACC.BirthDate
		, CASE
			WHEN EXISTS (SELECT * FROM dbo.Restraints WITH (NOLOCK) WHERE AccountID = ACC.AccountID AND CAST(GETDATE() AS smalldatetime) BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit)) THEN CAST(1 AS bit)
			ELSE CAST(0 AS bit)
		END AS AccountRestraintFlag, CAST(CC.CharacterCount AS int) AS CharacterCount
		, CH.WorldID, CH.CharacterID, COALESCE(CH.CharacterName, DCH.CharacterName) AS CharacterName
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
		END AS CharacterRestraintFlag
	FROM C1
		INNER JOIN dbo.Accounts ACC WITH (NOLOCK) ON C1.AccountID = ACC.AccountID
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON C1.AccountID = CH.AccountID
		LEFT OUTER JOIN dbo.DeletedCharacters DCH WITH (NOLOCK) ON CH.CharacterID = DCH.CharacterID
		LEFT OUTER JOIN dbo.VIX_CharacterCount CC WITH (NOEXPAND, NOLOCK) ON ACC.AccountID = CC.AccountID
	ORDER BY ACC.AccountID DESC;

	/**_## ??? ???? ?? ???? ??? 101115? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
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
