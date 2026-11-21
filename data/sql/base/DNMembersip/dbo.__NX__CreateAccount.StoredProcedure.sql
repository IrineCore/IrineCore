USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[__NX__CreateAccount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[__NX__CreateAccount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[__NX__CreateAccount] AS' 
END
GO
/**
0 ????
1 ??????
2 ??????????
3 ??????
**/

ALTER PROCEDURE [dbo].[__NX__CreateAccount]
	  @AccountName varchar(18)  -- ????6~18?
	, @NxLoginPwd varchar(18)-- ??,????6~18?,?????varchar,?????MD5????
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

--DECLARE @AccountName varchar(15);
DECLARE @NxLoginPwdMD5 varchar(32);

DECLARE @inyAccountLevelCode tinyint --// 0=?? ??, 10=??(??), 20=????(??), 30=???(??), 99=QA, 100=???
DECLARE @inyCharacterCreateLimit tinyint --// ?? ?? ??? ??? ?
DECLARE @inyCharacterMaxCount tinyint --// ?? ??? ? ?? ??? ?
DECLARE @dt2LastLoginDate datetime2(0) --// ?? ??? ??
DECLARE @vchLastLoginIP varchar(15) = NULL --// (optional) ?? ???? IP ??
DECLARE @intLastSessionID int = NULL --// (optional) ?? ???? ?? ID
DECLARE @vchJoinIP varchar(15) = NULL --// (optional) ?? Join ? IP ??
DECLARE @inyPublisherCode tinyint --// 0=????, 1=??, 2=??, 3=NHN Japan
DECLARE @inyGenderCode tinyint = NULL --// (optional) 1=??, 2=??
DECLARE @datBirthDate date = NULL --// (optional) ?? ?? (??? ??? ? ????.)
DECLARE @inyNationalityCode tinyint = NULL --// ?? ??. 95	Hong Kong, 207=Taiwan
DECLARE @inyChannelPartnerCode tinyint = NULL --// 1=MSN(Japan). ???? ?? ???? NULL?? ?????.
DECLARE @intAccountID int = 0

/**_# Rollback and return if inside an uncommittable transaction.*/


	SET @inyAccountLevelCode = 0;
	SET @inyPublisherCode = 4; --Indinisya:8 Singapore:6 4tw
	SET @inyCharacterCreateLimit = 7;
	SET @inyCharacterMaxCount = 4;
	
	IF (len(@AccountName)<6 or len(@AccountName)>18)
		RETURN 2;
	IF (len(@AccountName)<6 or len(@AccountName)>18)
		RETURN 2;
	/** Account already exist.*/
	IF EXISTS ( SELECT AccountName FROM dbo.Accounts WITH (NOLOCK) WHERE AccountName = @AccountName )
	BEGIN
		RETURN 1;
	END
	
	--SELECT substring(sys.fn_VarBinToHexStr(hashbytes('MD5','pass1014')),3,32)
	--SELECT substring(sys.fn_VarBinToHexStr(hashbytes('MD5',@NxLoginPwd)),3,32)
	-- ??MD5???,??0x??
	--SELECT @NxLoginPwd
	SELECT @NxLoginPwdMD5 = UPPER(substring(sys.fn_VarBinToHexStr(hashbytes('MD5', @NxLoginPwd)),3,32));

	INSERT dbo.Accounts (AccountName, AccountLevelCode, GameOption, 
	CharacterCreateLimit, LastCharacterCreateDate, CharacterMaxCount, 
	LastLoginDate, LastLogoutDate, LastLoginIP, LastSessionID, JoinIP, 
	RegisterDate, PublisherCode, GenderCode, BirthDate, Passphrase, 
	NationalityCode, ChannelPartnerCode,NxLoginPwd)
	VALUES (@AccountName, @inyAccountLevelCode, NULL, @inyCharacterCreateLimit, NULL, 
	@inyCharacterMaxCount, @dt2LastLoginDate, NULL, 
	NULL, @intLastSessionID, NULL, GETDATE(), 
	@inyPublisherCode, @inyGenderCode, @datBirthDate, 
	NULL, @inyNationalityCode, @inyChannelPartnerCode, 
	@NxLoginPwdMD5);
	
	-- SET @intAccountID = SCOPE_IDENTITY();
	-- UPDATE dbo.Accounts SET Passphrase = HASHBYTES('MD5', @vchPassphrase) WHERE AccountID = @intAccountID;
	-- SELECT HASHBYTES('MD5', @vchPassphrase)
	
	IF(@@ERROR=0)
		RETURN 0;
	ELSE
	    RETURN 3;
GO
