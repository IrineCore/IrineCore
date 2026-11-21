USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddAccount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddAccount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddAccount] AS' 
END
GO
/**
version : 12
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-04
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101101 = ?? ??? ??? ?????.
101109 = ??? ??? IP???.
history :
9 = input param. @inyNationalityCode ?? at 2010-09-14
10 = input param. @inyChannelPartnerCode ?? at 2010-11-25
11 = (48559) IP ?? ?? ?? by ??? at 2011-12-23
12 = ?? ?? ?? ?? ??? ?? ?? ?? by ??? at 2013-01-07
**/

ALTER PROCEDURE [dbo].[P_AddAccount]
	  @nvcAccountName nvarchar(50) --// ??? ?? ??
	, @inyAccountLevelCode tinyint --// 0=?? ??, 10=??(??), 20=????(??), 30=???(??), 99=QA, 100=???
	, @inyCharacterCreateLimit tinyint --// ?? ?? ??? ??? ?
	, @inyCharacterMaxCount tinyint --// ?? ??? ? ?? ??? ?
	, @dt2LastLoginDate datetime2(0) --// ?? ??? ??
	, @vchLastLoginIP varchar(15) = NULL --// (optional) ?? ???? IP ??
	, @intLastSessionID int = NULL --// (optional) ?? ???? ?? ID
	, @vchJoinIP varchar(15) = NULL --// (optional) ?? Join ? IP ??
	, @inyPublisherCode tinyint --// 0=????, 1=??, 2=??, 3=NHN Japan
	, @inyGenderCode tinyint = NULL --// (optional) 1=??, 2=??
	, @datBirthDate date = NULL --// (optional) ?? ?? (??? ??? ? ????.)
	, @vchPassphrase varchar(12) = NULL --// (optional) ?? ???? ??? ?? ??
	, @inyNationalityCode tinyint = NULL --// ?? ??. 95	Hong Kong, 207=Taiwan
	, @inyChannelPartnerCode tinyint = NULL --// 1=MSN(Japan). ???? ?? ???? NULL?? ?????.
	, @intAccountID int = NULL OUTPUT --// ??? ?? ID
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
	/**_# ?? ??? ??? IP?? 101109? ?????.*/
	IF EXISTS (
		SELECT * 
		FROM dbo.BlockedIPs WITH (NOLOCK) 
		WHERE dbo.FN_IP2IPNumber(@vchJoinIP) BETWEEN StartIP AND EndIP 
			AND (StartDate IS NULL OR GETDATE() BETWEEN StartDate AND EndDate)
			AND IPBlockMethodCode = 2
	)
	BEGIN
		SET @intReturnValue = 101109;
		GOTO ErrorHandler;
	END

	/**_# [Accounts] ???? INSERT???.*/
	INSERT dbo.Accounts (AccountName, AccountLevelCode, GameOption, CharacterCreateLimit, LastCharacterCreateDate, CharacterMaxCount, LastLoginDate, LastLogoutDate, LastLoginIP, LastSessionID, JoinIP, RegisterDate, PublisherCode, GenderCode, BirthDate, Passphrase, NationalityCode, ChannelPartnerCode, NewbieRewardFlag)
	VALUES (@nvcAccountName, @inyAccountLevelCode, NULL, @inyCharacterCreateLimit, NULL, @inyCharacterMaxCount, @dt2LastLoginDate, NULL, dbo.FN_IP2IPNumber(@vchLastLoginIP), @intLastSessionID, dbo.FN_IP2IPNumber(@vchJoinIP), GETDATE(), @inyPublisherCode, @inyGenderCode, @datBirthDate, CASE @inyPublisherCode WHEN 0 THEN 0x0 ELSE NULL END, @inyNationalityCode, @inyChannelPartnerCode, CAST(0 AS bit));

	SET @intAccountID = SCOPE_IDENTITY();

	/**_# ????? ??? ??, ????? ???? ?????.*/
	IF @inyPublisherCode = 0 AND @vchPassphrase IS NOT NULL
		UPDATE dbo.Accounts
		SET Passphrase = dbo.FN_HashPassphrase2(@vchPassphrase, @intAccountID)
		WHERE AccountID = @intAccountID;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UN_Accounts_AccountName%'
		SET @intReturnValue = 101101;

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
