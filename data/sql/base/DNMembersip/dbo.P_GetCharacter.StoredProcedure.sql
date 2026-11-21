USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCharacter]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCharacter]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCharacter] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-07
description : ??? ??
result set : AccountID, AccountName, AccountLevelCode, CharacterCreateLimit, CharacterMaxCount, LastLoginDate, LastLogoutDate, LastLoginIP, JoinIP, RegisterDate, GenderCode, BirthDate, AccountRestraintFlag, CharacterCount, WorldID, CharacterID, CharacterName, CreateDate, CharacterRestraintFlag
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101117 = ???? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_GetCharacter]
	  @nvcCharacterName nvarchar(30) --// ??? ??
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
	SELECT ACC.AccountID, ACC.AccountName, ACC.AccountLevelCode, CASE WHEN ACC.LastCharacterCreateDate < CAST(GETDATE() AS date) THEN 'MAX' ELSE CAST(ACC.CharacterCreateLimit AS varchar(10)) END AS CharacterCreateLimit
		, ACC.CharacterMaxCount, ACC.LastLoginDate, ACC.LastLogoutDate, dbo.FN_IPNumber2IP(ACC.LastLoginIP) AS LastLoginIP, dbo.FN_IPNumber2IP(ACC.JoinIP) AS JoinIP, ACC.RegisterDate, ACC.GenderCode, ACC.BirthDate
		, CASE
			WHEN EXISTS (SELECT * FROM dbo.Restraints WITH (NOLOCK) WHERE AccountID = ACC.AccountID AND CAST(GETDATE() AS smalldatetime) BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit)) THEN CAST(1 AS bit)
			ELSE CAST(0 AS bit)
		END AS AccountRestraintFlag, CAST(CC.CharacterCount AS int) AS CharacterCount
		, CH.WorldID, CH.CharacterID, CH.CharacterName
		, CH.CreateDate
		, CASE
			WHEN EXISTS (
				SELECT *
				FROM dbo.Restraints WITH (NOLOCK)
				WHERE CharacterID = CH.CharacterID AND CAST(GETDATE() AS smalldatetime) BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit)
			) THEN CAST(1 AS bit)
			ELSE CAST(0 AS bit)
		END AS CharacterRestraintFlag
	FROM dbo.Accounts ACC WITH (NOLOCK)
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON ACC.AccountID = CH.AccountID
		LEFT OUTER JOIN dbo.VIX_CharacterCount CC WITH (NOEXPAND, NOLOCK) ON ACC.AccountID = CC.AccountID AND CH.WorldID = CC.WorldID
	WHERE CH.CharacterName = @nvcCharacterName;

	/**_## ??? ???? ?? ???? ??? 101117? ?????.*/
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
